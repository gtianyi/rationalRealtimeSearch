#pragma once
#include <unordered_map>
#include <functional>
#include <limits>
#include <memory>
#include "ExpansionAlgorithm.h"
#include "../utility/PriorityQueue.h"
#include"../utility/ResultContainer.h"

using namespace std;

template <class Domain, class Node, class TopLevelAction>
class Risk : public ExpansionAlgorithm<Domain, Node, TopLevelAction>
{
	typedef typename Domain::State State;
	typedef typename Domain::Cost Cost;
	typedef typename Domain::HashState Hash;

public:
	Risk(Domain& domain, double lookahead, int expansionAllocation)
		: domain(domain), lookahead(lookahead), expansionsPerIteration(expansionAllocation)
	{}

	void incrementLookahead()
	{
		lookahead++;
	}

	void expand(PriorityQueue<shared_ptr<Node> >& open, unordered_map<State, shared_ptr<Node>, Hash>& closed, vector<shared_ptr<TopLevelAction>>& tlas,
		std::function<bool(shared_ptr<Node>, unordered_map<State, shared_ptr<Node>, Hash>&, PriorityQueue<shared_ptr<Node> >&, vector<shared_ptr<TopLevelAction>>&)> duplicateDetection,
		ResultContainer& res)
	{
		// This starts at 1, because we had to expand start to get the top level actions
		int expansions = 1;

		while (expansions < lookahead && !open.empty())
		{
			// At the beginning of each expansion we are going to update our beliefs at every TLA.
			// Why? Because this could be the first expansion in the phase.
			// If it isn't? Wouldn't we only need to update the TLA of the node we expanded last iteration?
			// In a tree, yes. But in a graph, this could have placed nodes onto the open lists of other TLAs.
			// Therefore, the beliefs of all TLAs should be updated before every expansion.
            kBestDecision(tlas);

			// Simulate expansion of best node under each TLA
			int chosenTLAIndex = simulateExpansion(tlas);

			// Expand under the TLA which holds the lowest risk
			shared_ptr<Node> chosenNode = tlas[chosenTLAIndex]->open_TLA.top();

			// Add this node to the expansion delay window
			domain.pushDelayWindow(chosenNode->getDelayCntr());

			// Check if current node is goal. If it is, then the expansion phase is over, time to move.
			if (domain.isGoal(chosenNode->getState()))
			{
				return;
			}

			// Remove the chosen node from open
			tlas[chosenTLAIndex]->open_TLA.pop();
			open.remove(chosenNode);
			chosenNode->close();

			// Book keeping for expansion count
			res.nodesExpanded++;
			expansions++;

			// Increment the delay counts of every other node on open...
			for (shared_ptr<Node> n : open)
			{
				n->incDelayCntr();
			}

			// Generate the successors of the chosen node
			vector<State> children = domain.successors(chosenNode->getState());

			// Book keeping for number of nodes generated
			res.nodesGenerated += children.size();

			State bestChild;
			Cost bestF = numeric_limits<double>::infinity();

			// Iterate over the successor states
			for (State child : children)
			{
				// Create a node for this state
				shared_ptr<Node> childNode = make_shared<Node>(chosenNode->getGValue() + domain.getEdgeCost(child),
					domain.heuristic(child), domain.distance(child), domain.distanceErr(child), 
					domain.epsilonHGlobal(), domain.epsilonDGlobal(), child, chosenNode, chosenNode->getOwningTLA());

				bool dup = duplicateDetection(childNode, closed, open, tlas);

				if (!dup && childNode->getFValue() < bestF)
				{
					bestF = childNode->getFValue();
					bestChild = child;
				}

				// Duplicate detection performed
				if (!dup)
				{
					// If this state hasn't yet been reached, add this node open 
					open.push(childNode);
					closed[child] = childNode;

					// Add to open of generating TLA
					tlas[chosenTLAIndex]->open_TLA.push(childNode);
				}
			}

			// Learn the one-step error
			if (bestF != numeric_limits<double>::infinity())
			{
				Cost epsD = (1 + domain.distance(bestChild)) - chosenNode->getDValue();
				Cost epsH = (domain.getEdgeCost(bestChild) + domain.heuristic(bestChild)) - chosenNode->getHValue();

				domain.pushEpsilonHGlobal(epsH);
				domain.pushEpsilonDGlobal(epsD);
			}
		}
	}

private:
	int simulateExpansion(vector<shared_ptr<TopLevelAction>>& tlas)
	{
		int minimalRiskTLA = 0;
		double minimalRisk = numeric_limits<double>::infinity();

		// Start by identifying alpha: the TLA with lowest expected cost
		int alphaTLA = 0;
		double alphaExpectedCost = tlas[0]->belief.expectedCost();
		for (int i = 1; i < tlas.size(); i++)
		{
			if (tlas[i]->belief.expectedCost() < alphaExpectedCost)
			{
				alphaExpectedCost = tlas[i]->belief.expectedCost();
				alphaTLA = i;
			}
		}

		// Iterate over the top level actions
		for (int i = 0; i < tlas.size(); i++)
		{
			// If this TLA has no unique subtree, skip its risk calc, it is pruned
			if (tlas[i]->open_TLA.empty())
				continue;

			// Simulate how expanding this TLA's best node would affect its belief
			// Belief of TLA is squished as a result of search. Mean stays the same, but variance is decreased by a factor based on expansion delay.
			double ds = expansionsPerIteration / domain.averageDelayWindow();
			double dy = tlas[i]->open_TLA.top()->getDValue();
			double squishFactor = min(1.0, (ds / dy));

			// Now squish the simulated belief by factor
		    vector<TopLevelAction> squishedTopLevelActions;
            for (auto a : tlas) {
                squishedTopLevelActions.push_back(*a);
            }

            squishedTopLevelActions[i].squishBelief(squishFactor);

			// Calculate the risk associated with expanding that node (by using the simulated belief as alpha in risk analysis)
			double riskCalculation = riskAnalysis(alphaTLA, squishedTopLevelActions);

			// If this is the first TLA risk has been calculated for, it by default minimizes risk...
            // If two actions minimize risk by same value, tie break on f-hat -> f -> g in this order.
            if (riskCalculation == minimalRisk)
            {
                if (tlas[i]->topLevelNode->getFHatValue() == tlas[minimalRiskTLA]->topLevelNode->getFHatValue())
                {
                    if (tlas[i]->topLevelNode->getFValue() == tlas[minimalRiskTLA]->topLevelNode->getFValue())
                    {
                        if (tlas[i]->topLevelNode->getGValue() ==
                                tlas[minimalRiskTLA]
                                        ->topLevelNode->getGValue()) {
                            if (tlas[i]->topLevelNode->getState().key() >
                                    tlas[minimalRiskTLA]
                                            ->topLevelNode->getState()
                                            .key()) {
                                minimalRiskTLA = i;
                            }
                        } else if (tlas[i]->topLevelNode->getGValue() >
                                tlas[minimalRiskTLA]
                                        ->topLevelNode->getGValue()) {
                            minimalRiskTLA = i;
                        }
                    }
                    else if (tlas[i]->topLevelNode->getFValue() < tlas[minimalRiskTLA]->topLevelNode->getFValue())
                    {
                        minimalRiskTLA = i;
                    }
                }
                else if (tlas[i]->topLevelNode->getFHatValue() < tlas[minimalRiskTLA]->topLevelNode->getFHatValue())
                {
                    minimalRiskTLA = i;
                }
            }
			else if (riskCalculation < minimalRisk)
			{
				// Otherwise the TLA with the lower risk replaces the current lowest
				minimalRisk = riskCalculation;
				minimalRiskTLA = i;
			}
		}

		return minimalRiskTLA;
	}

	double riskAnalysis(int alphaIndex, vector<TopLevelAction>& squishedTopLevelActions)
	{
		double risk = 0;

		// Perform numerical integration to calculate risk associated with taking alpha as the expansion
		for (auto& alpha : squishedTopLevelActions[alphaIndex].belief)
		{
			for (int tla = 0; tla < squishedTopLevelActions.size(); tla++)
			{
				// Don't integrate over alpha as a beta action
				if (tla == alphaIndex)
					continue;

				// Integrate over values in beta belief
				for (auto& beta : squishedTopLevelActions[tla].belief)
				{
					// Only use beta costs less than alpha cost in risk analysis
					if (beta.cost < alpha.cost)
					{
						// Calculate the risk
						double value = alpha.probability * beta.probability * (alpha.cost - beta.cost);
						risk += value;
					}
					else
						break;
				}
			}
		}

		return risk;
        }

        void kBestDecision(vector<shared_ptr<TopLevelAction>>& tlas) {
            // The K-Best decision assumes that the only nodes within the
            // subtrees of the TLAs are the k-best frontier nodes
            // on their opened lists. Find them.
            for (auto tla : tlas) {
                //tla->kBestNodes.clear();

                shared_ptr<Node> best = tla->open_TLA.top();
                //tla->open_TLA.pop();

                // Make this node's PDF a discrete distribution...
               /* best->distribution = DiscreteDistribution(100,*/
                        //best->getFValue(),
                        //best->getFHatValue(),
                        //best->getDValue(),
                        //best->getFHatValue() - best->getFValue());

                //tla->kBestNodes.push_back(best);

                // Now put the nodes back in the top level open list
                /*for (shared_ptr<Node> n : tla->kBestNodes) {*/
                    //tla->open_TLA.push(n);
                /*}*/

                // Now that k-best are selected, perform Cserna backup
                // csernaBackup(tla);
                tla->expectedMinimumPathCost = best->getFHatValue();
                tla->setBelief(best);
            }
	}

protected:
	Domain & domain;
	double lookahead;
	string sortingFunction;
	int k = 1;
	int expansionsPerIteration;
};
