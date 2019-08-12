#pragma once
#include <vector>
#include <set>
#include <functional>
#include <unordered_map>
#include <memory>
#include "utility/DiscreteDistribution.h"
#include "utility/PriorityQueue.h"
#include "utility/ResultContainer.h"
#include "decisionAlgorithms/DecisionAlgorithm.h"
#include "decisionAlgorithms/KBestBackup.h"
#include "decisionAlgorithms/NancyDDDecision.h"
#include "decisionAlgorithms/ScalarBackup.h"
#include "expansionAlgorithms/ExpansionAlgorithm.h"
#include "expansionAlgorithms/AStar.h"
#include "expansionAlgorithms/BreadthFirst.h"
#include "expansionAlgorithms/DepthFirst.h"
#include "expansionAlgorithms/Risk.h"
#include "expansionAlgorithms/RiskDD.h"
#include "expansionAlgorithms/Confidence.h"
#include "learningAlgorithms/LearningAlgorithm.h"
#include "learningAlgorithms/Dijkstra.h"
#include "learningAlgorithms/Dijkstra_distribution.h"
#include "learningAlgorithms/Ignorance.h"

#include <time.h>

using namespace std;

template <class Domain>
class RealTimeSearch {
public:
    typedef typename Domain::State State;
    typedef typename Domain::Cost Cost;
    typedef typename Domain::HashState Hash;

    struct Node {
        Cost g;
        Cost h;
        Cost d;
        Cost derr;
        Cost epsH;
        Cost epsD;
        shared_ptr<Node> parent;
        State stateRep;
        int owningTLA;
        bool open;
        int delayCntr;
        DiscreteDistribution distribution;
        DiscreteDistribution hStartDistribution;
        DiscreteDistribution hStartDistribution_ps;
        bool lackOfHValueData;

    public:
        Cost getGValue() const { return g; }
        Cost getHValue() const { return h; }
        Cost getDValue() const { return d; }
        Cost getDErrValue() const { return derr; }
        Cost getFValue() const { return g + h; }
        Cost getEpsilonH() const { return epsH; }
        Cost getEpsilonD() const { return epsD; }
        Cost getFHatValue() const { return g + getHHatValue(); }
        Cost getFHatValueFromDist() const { return g + getHHatValueFromDist(); }
        Cost getDHatValue() const { return (derr / (1.0 - epsD)); }
        Cost getHHatValue() const { return h + getDHatValue() * epsH; }
        Cost getHHatValueFromDist() const { return hStartDistribution.expectedCost(); }
        State getState() const { return stateRep; }
        shared_ptr<Node> getParent() const { return parent; }
        int getOwningTLA() const { return owningTLA; }

        DiscreteDistribution getHstartDistribution() const {
            return hStartDistribution;
        }

        DiscreteDistribution getHstartDistribution_ps() const {
            return hStartDistribution_ps;
        }

        void setHValue(Cost val) { h = val; }
        void setGValue(Cost val) { g = val; }
        void setDValue(Cost val) { d = val; }
        void setDErrValue(Cost val) { derr = val; }
        void setEpsilonH(Cost val) { epsH = val; }
        void setEpsilonD(Cost val) { epsD = val; }
        void setState(State s) { stateRep = s; }
        void setOwningTLA(int tla) { owningTLA = tla; }
        void setParent(shared_ptr<Node> p) { parent = p; }

        void setHStartDistribution(DiscreteDistribution& dist) {
            hStartDistribution = dist;
        }

        void setHStartDistribution_ps(DiscreteDistribution& dist) {
            hStartDistribution_ps = dist;
        }

        bool onOpen() { return open; }
        void close() { open = false; }
        void reOpen() { open = true; }

        void markStart() { stateRep.markStart(); }

        void incDelayCntr() { delayCntr++; }
        int getDelayCntr() { return delayCntr; }

        Node(Cost g,
                Cost h,
                Cost d,
                Cost derr,
                Cost epsH,
                Cost epsD,
                State state,
                shared_ptr<Node> parent,
                int tla)
                : g(g),
                  h(h),
                  d(d),
                  derr(derr),
                  epsH(epsH),
                  epsD(epsD),
                  stateRep(state),
                  parent(parent),
                  owningTLA(tla) {
            open = true;
            delayCntr = 0;
        }

        Node(Cost g,
                DiscreteDistribution& hstartdist,
                DiscreteDistribution& hstartdist_ps,
                State state,
                shared_ptr<Node> parent,
                int tla)
                : g(g),
                  hStartDistribution(hstartdist),
                  hStartDistribution_ps(hstartdist_ps),
                  stateRep(state),
                  parent(parent),
                  owningTLA(tla) {
            open = true;
            delayCntr = 0;
            lackOfHValueData = false;
        }

        friend std::ostream& operator<<(std::ostream& stream,
                const Node& node) {
            stream << node.getState() << endl;
            stream << "f: " << node.getFValue() << endl;
            stream << "g: " << node.getGValue() << endl;
            stream << "h: " << node.getHValue() << endl;
            stream << "derr: " << node.getDErrValue() << endl;
            stream << "d: " << node.getDValue() << endl;
            stream << "epsilon-h: " << node.getEpsilonH() << endl;
            stream << "epsilon-d: " << node.getEpsilonD() << endl;
            stream << "f-hat: " << node.getFHatValue() << endl;
            stream << "d-hat: " << node.getDHatValue() << endl;
            stream << "h-hat: " << node.getHHatValue() << endl;
            stream << "action generated by: " << node.getState().getLabel()
                   << endl;
            stream << "-----------------------------------------------" << endl;
            stream << endl;
            return stream;
        }

        static bool compareNodesF(const shared_ptr<Node> n1,
                const shared_ptr<Node> n2) {
            // Tie break on g-value
            if (n1->getFValue() == n2->getFValue()) {
                return n1->getGValue() > n2->getGValue();
            }
            return n1->getFValue() < n2->getFValue();
        }

        static bool compareNodesFHat(const shared_ptr<Node> n1,
                const shared_ptr<Node> n2) {
            // Tie break on g-value
            if (n1->getFHatValue() == n2->getFHatValue()) {
                if (n1->getFValue() == n2->getFValue())
                {
				    return n1->getGValue() > n2->getGValue();
                }
                return n1->getFValue() < n2->getFValue();
			}
			return n1->getFHatValue() < n2->getFHatValue();
        }

        static bool compareNodesFHatFromDist(const shared_ptr<Node> n1,
                const shared_ptr<Node> n2) {
            // Tie break on g-value
            if (n1->getFHatValueFromDist() == n2->getFHatValueFromDist()) {
                if (n1->getFValue() == n2->getFValue())
                {
				    return n1->getGValue() > n2->getGValue();
                }
                return n1->getFValue() < n2->getFValue();
			}
			return n1->getFHatValue() < n2->getFHatValue();
        }

        static bool compareNodesH(const shared_ptr<Node> n1,
                const shared_ptr<Node> n2) {
            if (n1->getHValue() == n2->getHValue()) {
                return n1->getGValue() > n2->getGValue();
            }
            return n1->getHValue() < n2->getHValue();
        }
    };

    struct TopLevelAction {
    public:
        PriorityQueue<shared_ptr<Node>> open_TLA;
        Cost expectedMinimumPathCost;
        shared_ptr<Node> topLevelNode;
        vector<shared_ptr<Node>> kBestNodes;
        DiscreteDistribution belief;
        DiscreteDistribution belief_ps;

        TopLevelAction() { open_TLA.swapComparator(Node::compareNodesFHat); }

        TopLevelAction(const TopLevelAction& tla) {
            open_TLA = tla.open_TLA;
            expectedMinimumPathCost = tla.expectedMinimumPathCost;
            topLevelNode = tla.topLevelNode;
            kBestNodes = tla.kBestNodes;
            belief = tla.belief;
        }

        TopLevelAction& operator=(const TopLevelAction& rhs) {
            if (&rhs == this)
                return *this;
            open_TLA = rhs.open_TLA;
            expectedMinimumPathCost = rhs.expectedMinimumPathCost;
            topLevelNode = rhs.topLevelNode;
            kBestNodes = rhs.kBestNodes;
            belief = rhs.belief;
            return *this;
        }
    };

	struct TopLevelActionDD : public TopLevelAction {
		TopLevelActionDD() {
			this->open_TLA.swapComparator(Node::compareNodesFHatFromDist);
		}
	};

    RealTimeSearch(Domain& domain,
            string expansionModule,
            string learningModule,
            string decisionModule,
            double lookahead,
            double k = 1,
            string beliefType = "normal")
            : domain(domain),
              expansionPolicy(expansionModule),
              learningPolicy(learningModule),
              decisionPolicy(decisionModule),
              lookahead(lookahead),
              beliefType(beliefType) {
        if (expansionModule == "a-star") {
            expansionAlgo = make_shared<AStar<Domain, Node, TopLevelAction>>(
                    domain, lookahead, "f");
        } else if (expansionModule == "f-hat") {
            expansionAlgo = make_shared<AStar<Domain, Node, TopLevelAction>>(
                    domain, lookahead, "fhat");
        } else if (expansionModule == "dfs") {
            expansionAlgo =
                    make_shared<DepthFirst<Domain, Node, TopLevelAction>>(
                            domain, lookahead);
        } else if (expansionModule == "bfs") {
            expansionAlgo =
                    make_shared<BreadthFirst<Domain, Node, TopLevelAction>>(
                            domain, lookahead);
        } else if (expansionModule == "risk") {
            expansionAlgo = make_shared<Risk<Domain, Node, TopLevelAction>>(
                    domain, lookahead, 1);
        }  else if (expansionModule == "riskDD") {
            expansionAlgo = make_shared<RiskDD<Domain, Node, TopLevelActionDD>>(
                    domain, lookahead, 1);
        } else if (expansionModule == "confidence") {
            expansionAlgo =
                    make_shared<Confidence<Domain, Node, TopLevelAction>>(
                            domain, lookahead, 1);
        } else {
            expansionAlgo = make_shared<AStar<Domain, Node, TopLevelAction> >(domain, lookahead, "f");
            cout << "not specified expansion Mudule type, use Astart" << endl;
        }

        if (learningModule == "none") {
            learningAlgo =
                    make_shared<Ignorance<Domain, Node, TopLevelAction>>();
        } else if (learningModule == "learn") {
            learningAlgo =
                    make_shared<Dijkstra<Domain, Node, TopLevelAction>>(domain);
        } else if (learningModule == "learnDD") {
            learningAlgo = make_shared<
                    DijkstraDistribution<Domain, Node, TopLevelActionDD>>(
                    domain);
        }  else {
            learningAlgo = make_shared<Dijkstra<Domain, Node, TopLevelAction> >(domain);
        }

        if (decisionModule == "minimin") {
            decisionAlgo =
                    make_shared<ScalarBackup<Domain, Node, TopLevelAction>>(
                            "minimin");
        } else if (decisionModule == "bellman") {
            decisionAlgo =
                    make_shared<ScalarBackup<Domain, Node, TopLevelAction>>(
                            "bellman");
        } else if (decisionModule == "k-best") {
            decisionAlgo =
                    make_shared<KBestBackup<Domain, Node, TopLevelAction>>(
                            domain, k, lookahead);
        } else if (decisionModule == "nancyDD") {
            decisionAlgo =
                    make_shared<NancyDDDecision<Domain, Node, TopLevelActionDD>>(
                            domain, lookahead);
        }  else {
            decisionAlgo =
                    make_shared<ScalarBackup<Domain, Node, TopLevelAction>>(
                            "minimin");
        }
    }

    ~RealTimeSearch() { clean(); }

    ResultContainer search(int iterationlimit) {
        domain.initialize(expansionPolicy, lookahead);

        ResultContainer res;

        shared_ptr<Node> start;

        if (beliefType == "normal") {
            start = make_shared<Node>(0,
                    domain.heuristic(domain.getStartState()),
                    domain.distance(domain.getStartState()),
                    domain.distanceErr(domain.getStartState()),
                    domain.epsilonHGlobal(),
                    domain.epsilonDGlobal(),
                    domain.getStartState(),
                    nullptr,
                    -1);

        } else if (beliefType == "data") {
            start = make_shared<Node>(0,
                    domain.hstart_distribution(domain.getStartState()),
                    domain.hstart_distribution_ps(domain.getStartState()),
                    domain.getStartState(),
                    nullptr,
                    -1);
        }

        // Get the start node
        int count = 0;
        //
        // clock_t startTime = clock();

        while (count <= iterationlimit) {
            // mark this node as the start of the current search (to
            // prevent state pruning based on label)
            start->markStart();

            count++;

            // if (beliefType == "data") {
            // cout << "rl loop " << count << "h "
            //<< start->getFValue() - start->getGValue() << endl;
            //}

            // Check if a goal has been reached
            if (domain.isGoal(start->getState())) {
                // Calculate path cost and return solution
                calculateCost(start, res);

                // cout<<"count "<<count<<"\n";

                return res;
            }

            restartLists(start);

            // Exploration Phase
            domain.updateEpsilons();

            if (beliefType == "normal") {
                generateTopLevelActions(start, res);
                expansionAlgo->expand(
                        open, closed, tlas, duplicateDetection, res);
            } else if (beliefType == "data") {
                generateTopLevelActionsDD(start, res);
                expansionAlgo->expand(
                        open, closed, tlas, duplicateDetectionDD, res);
            } else {
                cout << "Realtime search main loop line 370: wrong "
                        "belief "
                        "type!!!"
                     << endl;
            }

            // Check if this is a dead end
            if (open.empty()) {
                break;
            }

            // Learning Phase
            learningAlgo->learn(open, closed);

            // Decision-making Phase
            start = decisionAlgo->backup(open, tlas, start);

            cout << "g " << start->getGValue() << " h " << start->getHValue()
                 << endl;

            // Add this step to the path taken so far
            res.path.push(start->getState().getLabel());
		}

		//cout<<"iteration: " << count<<endl;

		if (count >= iterationlimit)
            noSolutionFound(res);

		return res;
    }

private:
    static bool duplicateDetection(shared_ptr<Node> node,
            unordered_map<State, shared_ptr<Node>, Hash>& closed,
            PriorityQueue<shared_ptr<Node>>& open,
            vector<TopLevelAction>& tlaList) {
        // Check if this state exists
        typename unordered_map<State, shared_ptr<Node>, Hash>::iterator it =
                closed.find(node->getState());

        if (it != closed.end()) {
            // This state has been generated before, check if its node is on
            // OPEN
            if (it->second->onOpen()) {
                // This node is on OPEN, keep the better g-value
                if (node->getGValue() < it->second->getGValue()) {
                    tlaList[it->second->getOwningTLA()].open.remove(it->second);
                    it->second->setGValue(node->getGValue());
                    it->second->setParent(node->getParent());
                    it->second->setHValue(node->getHValue());
                    it->second->setDValue(node->getDValue());
                    it->second->setDErrValue(node->getDErrValue());
                    it->second->setEpsilonH(node->getEpsilonH());
                    it->second->setEpsilonD(node->getEpsilonD());
                    it->second->setState(node->getState());
                    it->second->setOwningTLA(node->getOwningTLA());
                    tlaList[node->getOwningTLA()].open.push(it->second);
                }
            } else {
                // This node is on CLOSED, compare the f-values. If this new
                // f-value is better, reset g, h, and d.
                // Then reopen the node.
                if (node->getFValue() < it->second->getFValue()) {
                    it->second->setGValue(node->getGValue());
                    it->second->setParent(node->getParent());
                    it->second->setHValue(node->getHValue());
                    it->second->setDValue(node->getDValue());
                    it->second->setDErrValue(node->getDErrValue());
                    it->second->setEpsilonH(node->getEpsilonH());
                    it->second->setEpsilonD(node->getEpsilonD());
                    it->second->setState(node->getState());
                    it->second->setOwningTLA(node->getOwningTLA());
                    tlaList[node->getOwningTLA()].open.push(it->second);
                    it->second->reOpen();
                    open.push(it->second);
                }
            }

            return true;
        }

        return false;
    }

    static bool duplicateDetectionDD(shared_ptr<Node> node,
            unordered_map<State, shared_ptr<Node>, Hash>& closed,
            PriorityQueue<shared_ptr<Node>>& open,
            vector<TopLevelAction>& tlaList) {
        // Check if this state exists
        typename unordered_map<State, shared_ptr<Node>, Hash>::iterator it =
                closed.find(node->getState());

        if (it != closed.end()) {
            // This state has been generated before, check if its node is on
            // OPEN
            if (it->second->onOpen()) {
                // This node is on OPEN, keep the better g-value
                if (node->getGValue() < it->second->getGValue()) {
                    tlaList[it->second->getOwningTLA()].open.remove(it->second);
                    it->second->setGValue(node->getGValue());
                    it->second->setParent(node->getParent());
                    it->second->setHStartDistribution(node->getHstartDistribution());
                    it->second->setHStartDistribution_ps(node->getHstartDistribution_ps());
                    it->second->setState(node->getState());
                    it->second->setOwningTLA(node->getOwningTLA());
                    tlaList[node->getOwningTLA()].open.push(it->second);
                }
            } else {
                // This node is on CLOSED, compare the f-values. If this new
                // f-value is better, reset g, h, and d.
                // Then reopen the node.
				//
				// here for dd we compare f-hat, because in the learning, 
				// if its distribution is updated, then the hvalue should not
				// be ues anywhere
                if (node->getFHatValueFromDist() < it->second->getFHatValueFromDist()) {
                    it->second->setGValue(node->getGValue());
                    it->second->setParent(node->getParent());
                    it->second->setHStartDistribution(
                            node->getHstartDistribution());
                    it->second->setHStartDistribution_ps(node->getHstartDistribution_ps());
                    it->second->setState(node->getState());
                    it->second->setOwningTLA(node->getOwningTLA());
                    tlaList[node->getOwningTLA()].open.push(it->second);
                    it->second->reOpen();
                    open.push(it->second);
                }
            }

            return true;
        }

        return false;
    }

    // we need a new generate funciton for nancydd and simplified node
    // constructor
    void generateTopLevelActions(shared_ptr<Node> start, ResultContainer& res) {
        // The first node to be expanded in any problem is the start node
        // Doing so yields the top level actions
        start->close();
        closed[start->getState()] = start;
        res.nodesExpanded++;

        vector<State> children = domain.successors(start->getState());
        res.nodesGenerated += children.size();

        State bestChild;
        Cost bestF = numeric_limits<double>::infinity();

        for (State child : children) {
            shared_ptr<Node> childNode = make_shared<Node>(
                    start->getGValue() + domain.getEdgeCost(child),
                    domain.heuristic(child),
                    domain.distance(child),
                    domain.distanceErr(child),
                    domain.epsilonHGlobal(),
                    domain.epsilonDGlobal(),
                    child,
                    start,
                    tlas.size());

            if (childNode->getFValue() < bestF) {
                bestF = childNode->getFValue();
                bestChild = child;
            }

            // No top level action will ever be a duplicate, so no need to
            // check.
            // Make a new top level action and push this node onto its open
            TopLevelAction tla;
            tla.topLevelNode = childNode;

            childNode->distribution = DiscreteDistribution(100,
                    childNode->getFValue(),
                    childNode->getFHatValue(),
                    childNode->getDValue(),
                    childNode->getFHatValue() - childNode->getFValue());

            tla.expectedMinimumPathCost =
                    childNode->distribution.expectedCost();

            // Push this node onto open and closed
            closed[child] = childNode;
            open.push(childNode);
            tla.open.push(childNode);

            // Add this top level action to the list
            tlas.push_back(tla);
		}

		// Learn one-step error
		if (!children.empty())
		{
			Cost epsD = (1 + domain.distance(bestChild)) - start->getDValue();
			Cost epsH = (domain.getEdgeCost(bestChild) + domain.heuristic(bestChild)) - start->getHValue();

			domain.pushEpsilonHGlobal(epsH);
			domain.pushEpsilonDGlobal(epsD);
		}
    }

    void generateTopLevelActionsDD(shared_ptr<Node> start,
            ResultContainer& res) {
        // The first node to be expanded in any problem is the start node
        // Doing so yields the top level actions
        start->close();
        closed[start->getState()] = start;
        res.nodesExpanded++;

        vector<State> children = domain.successors(start->getState());
        res.nodesGenerated += children.size();

        State bestChild;
        Cost bestF = numeric_limits<double>::infinity();

        for (State child : children) {
            shared_ptr<Node> childNode = make_shared<Node>(
                    start->getGValue() + domain.getEdgeCost(child),
                    domain.hstart_distribution(child),
                    domain.hstart_distribution_ps(child),
                    child,
                    start,
                    tlas.size());

            if (childNode->getFValue() < bestF) {
                bestF = childNode->getFValue();
                bestChild = child;
            }

            // No top level action will ever be a duplicate, so no need to
            // check.
            // Make a new top level action and push this node onto its open
            TopLevelAction tla;
            tla.topLevelNode = childNode;

            childNode->distribution = domain.hstart_distribution(child);

            childNode->distribution_ps = domain.hstart_distribution_ps(child);

            tla.expectedMinimumPathCost =
                    childNode->distribution.expectedCost() +
                    childNode->getGValue();

            // Push this node onto open and closed
            closed[child] = childNode;
            open.push(childNode);
            tla.open.push(childNode);

            // Add this top level action to the list
            tlas.push_back(tla);
        }
        }

        void restartLists(shared_ptr<Node> start)
	{
		// clear the TLA list
		tlas.clear();

		// Empty OPEN and CLOSED
		open.clear();

		// delete all of the nodes from the last expansion phase
		closed.clear();
	}

	void clean()
	{
		// clear the TLA list
		tlas.clear();

		// Empty OPEN and CLOSED
		open.clear();

		// delete all of the nodes from the last expansion phase
		closed.clear();
	}

	void calculateCost(shared_ptr<Node> solution, ResultContainer& res)
	{
		res.solutionFound = true;
		res.solutionCost = solution->getFValue();
	}

    void noSolutionFound(ResultContainer& res) {
            res.solutionFound = false;
            res.solutionCost = -1;
	}

protected:
	Domain& domain;
	shared_ptr<ExpansionAlgorithm<Domain, Node, TopLevelAction> > expansionAlgo;
	shared_ptr<LearningAlgorithm<Domain, Node, TopLevelAction> > learningAlgo;
	shared_ptr<DecisionAlgorithm<Domain, Node, TopLevelAction> > decisionAlgo;
	PriorityQueue<shared_ptr<Node> > open;
	unordered_map<State, shared_ptr<Node>, Hash> closed;
	vector<TopLevelAction> tlas;

	double lookahead;
	string beliefType;
	string expansionPolicy;
	string learningPolicy;
	string decisionPolicy;
};
