#pragma once
#include <vector>
#include <set>
#include <functional>
#include <unordered_map>
#include <memory>
#include "utility/DiscreteDistribution.h"
#include "utility/DiscreteDistributionDD.h"
#include "utility/PriorityQueue.h"
#include "utility/ResultContainer.h"
#include "decisionAlgorithms/DecisionAlgorithm.h"
#include "decisionAlgorithms/KBestBackup_faster.h"
//#include "decisionAlgorithms/KBestBackup.h"
//#include "decisionAlgorithms/NancyBackup.h"
//#include "decisionAlgorithms/KbestBackupPersistency.h"
#include "decisionAlgorithms/NancyDDDecision.h"
#include "decisionAlgorithms/ScalarBackup.h"
#include "expansionAlgorithms/ExpansionAlgorithm.h"
#include "expansionAlgorithms/AStar.h"
#include "expansionAlgorithms/BreadthFirst.h"
#include "expansionAlgorithms/DepthFirst.h"
#include "expansionAlgorithms/Risk.h"
#include "expansionAlgorithms/RiskDD.h"
#include "expansionAlgorithms/RiskIE.h"
#include "expansionAlgorithms/RiskDD_squish.h"
#include "learningAlgorithms/LearningAlgorithm.h"
#include "learningAlgorithms/Dijkstra.h"
#include "learningAlgorithms/Dijkstra_distribution.h"
#include "learningAlgorithms/Ignorance.h"

#include <time.h>
#include <cassert>

#include "utility/debug.h"

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
        //shared_ptr<DiscreteDistribution> distribution;
        DiscreteDistribution distribution;
        DiscreteDistributionDD hStartDistribution;
        DiscreteDistributionDD hStartDistribution_ps;
		bool twoDistribtuionCleared;

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
		//Cost getDHatValue() const { return (derr / (1.0 - 0.18500444)); }
		Cost getHHatValue() const { return h + getDHatValue() * epsH; }
		//Cost getHHatValue() const { return h + getDHatValue() * 0.18500444; }
        Cost getHHatValueFromDist() const {
            if (twoDistribtuionCleared)
                return numeric_limits<double>::infinity();
            return hStartDistribution.expectedCost();
        }
        State getState() const { return stateRep; }
        shared_ptr<Node> getParent() const { return parent; }
        int getOwningTLA() const { return owningTLA; }

        DiscreteDistributionDD getHstartDistribution() const {
            return hStartDistribution;
        }

        DiscreteDistributionDD getHstartDistribution_ps() const {
            return hStartDistribution_ps;
        }

        /*shared_ptr<DiscreteDistribution> getAssumptionDistribution() const{*/
            //return distribution;
        /*}*/

        void setHValue(Cost val) { h = val; }
        void setGValue(Cost val) { g = val; }
        void setDValue(Cost val) { d = val; }
        void setDErrValue(Cost val) { derr = val; }
        void setEpsilonH(Cost val) { epsH = val; }
        void setEpsilonD(Cost val) { epsD = val; }
        void setState(State s) { stateRep = s; }
        void setOwningTLA(int tla) { owningTLA = tla; }
        void setParent(shared_ptr<Node> p) { parent = p; }

        void setHStartDistribution(const DiscreteDistributionDD& dist) {
            hStartDistribution = dist;
        }

        void setHStartDistribution_ps(const DiscreteDistributionDD& dist) {
            hStartDistribution_ps = dist;
        }

        bool onOpen() { return open; }
        void close() { open = false; }
        void reOpen() { open = true; }

        void markStart() { stateRep.markStart(); }

        void incDelayCntr() { delayCntr++; }
        int getDelayCntr() { return delayCntr; }

        void markClearTwoDistribution() { twoDistribtuionCleared = true; }
        void markUnClearTwoDistribution() { twoDistribtuionCleared = false; }

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
                const DiscreteDistributionDD& hstartdist,
                const DiscreteDistributionDD& hstartdist_ps,
                Cost h,
                State state,
                shared_ptr<Node> parent,
                int tla)
                : g(g),
                  hStartDistribution(hstartdist),
                  hStartDistribution_ps(hstartdist_ps),
				  h(h),
                  stateRep(state),
                  parent(parent),
                  owningTLA(tla) {
            open = true;
            delayCntr = 0;
            twoDistribtuionCleared = false;
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
                    if (n1->getGValue() == n2->getGValue()) {
                        return n1->getState().key() > n2->getState().key();
                    }
				    return n1->getGValue() > n2->getGValue();
                }
                return n1->getFValue() < n2->getFValue();
			}
			return n1->getFHatValue() < n2->getFHatValue();
        }

        static bool compareNodesFHatFromDist(const shared_ptr<Node> n1,
                const shared_ptr<Node> n2) {
            // Tie break on f-value then g-value
            if (n1->getFHatValueFromDist() == n2->getFHatValueFromDist()) {
                if (n1->getFValue() == n2->getFValue()) {
                    if (n1->getGValue() == n2->getGValue()) {
                        return n1->getState().key() > n2->getState().key();
                    }
                    return n1->getGValue() > n2->getGValue();
                }
                return n1->getFValue() < n2->getFValue();
			}
			return n1->getFHatValueFromDist() < n2->getFHatValueFromDist();
        }

        static bool compareNodesH(const shared_ptr<Node> n1,
                const shared_ptr<Node> n2) {
            if (n1->getHValue() == n2->getHValue()) {
                return n1->getGValue() > n2->getGValue();
            }
            return n1->getHValue() < n2->getHValue();
        }

        static bool compareNodesHHat(const shared_ptr<Node> n1,
                const shared_ptr<Node> n2) {
            if (n1->getHHatValue() == n2->getHHatValue()) {
                return n1->getGValue() > n2->getGValue();
            }
            return n1->getHHatValue() < n2->getHHatValue();
        }

        static double getLowerConfidence(const shared_ptr<Node> n) {
            double f = n->getFValue();
            double mean = n->getFHatValue();
            if (f == mean) {
                return f;
            }
            double error = mean - f;
            double stdDev = error / 2.0;
            double var = pow(stdDev, 2);
			// 1.96 is the Z value from the Z table to get the 2.5 confidence
			return max(f, mean - (1.96 * var));
        }

        static bool compareNodesLC(const shared_ptr<Node> n1, const shared_ptr<Node> n2) {
            // Lower confidence interval
            if (getLowerConfidence(n1) == getLowerConfidence(n2)) {
                return n1->getGValue() > n2->getGValue();
            }
            return getLowerConfidence(n1) < getLowerConfidence(n2);
        }
    };

    struct TopLevelAction {
    public:
        PriorityQueue<shared_ptr<Node>> open_TLA;
        Cost expectedMinimumPathCost;
        shared_ptr<Node> topLevelNode;
        //vector<shared_ptr<Node>> kBestNodes;
        Cost h_TLA;
        DiscreteDistribution belief;


        TopLevelAction() { open_TLA.swapComparator(Node::compareNodesFHat); }
        //TopLevelAction() { open_TLA.swapComparator(Node::compareNodesLC); }

        TopLevelAction(const TopLevelAction& tla) { copy(tla); }

        TopLevelAction& operator=(const TopLevelAction& rhs) {
            if (&rhs == this)
                return *this;
            copy(rhs);
            return *this;
        }

        Cost getF_TLA() const { return this->topLevelNode->getGValue() + h_TLA; }

        //shared_ptr<DiscreteDistribution> getBelief() { return belief; };
        //DiscreteDistribution getBelief() { return belief; };
        //double getBeliefExpectedCost() { return belief.expectedCost(); };

        //void squishBelief(double factor) { belief->squish(factor); };
        void squishBelief(double factor) { belief.squish(factor); };

        //void setBelief(shared_ptr<DiscreteDistribution> _belief) {
        void setBelief(shared_ptr<Node> best) {
            belief = DiscreteDistribution(100,
                    best->getFValue(),
                    best->getFHatValue(),
                    best->getDValue(),
                    best->getFHatValue() - best->getFValue());
        };

        virtual void setBeliefDD(const DiscreteDistributionDD& _belief) {
            DEBUG_MSG( "call set beliefDD from base TLA not TLADD: RealTimeSearch.h:247" );
            exit(1);
        }

		virtual void setBeliefDD_ps(const DiscreteDistributionDD& _belief) {
            DEBUG_MSG( "call set beliefDD_ps from base TLA not TLADD: RealTimeSearch.h:252" );
            exit(1);
        }

		virtual DiscreteDistributionDD getBeliefDD() const {
            DEBUG_MSG( "call get beliefDD from base TLA not TLADD: RealTimeSearch.h:247" );
            exit(1);
        }

		virtual DiscreteDistributionDD getBeliefDD_ps() const {
            DEBUG_MSG( "call get beliefDD_ps from base TLA not TLADD: RealTimeSearch.h:247" );
            exit(1);
        }

    private:
        //shared_ptr<DiscreteDistribution> belief;

        void copy(const TopLevelAction& tla) {
            open_TLA = tla.open_TLA;
            expectedMinimumPathCost = tla.expectedMinimumPathCost;
            topLevelNode = tla.topLevelNode;
            belief = tla.belief;
            //kBestNodes = tla.kBestNodes;
            h_TLA = tla.h_TLA;
        }
    };

    struct TopLevelActionDD : public TopLevelAction {
        TopLevelActionDD() {
            this->open_TLA.swapComparator(Node::compareNodesFHatFromDist);
        }

        DiscreteDistributionDD beliefDD;
        DiscreteDistributionDD beliefDD_ps;

        TopLevelActionDD(const TopLevelActionDD& tla) : TopLevelAction(tla) {
            copy(tla);
        }

        TopLevelActionDD& operator=(const TopLevelActionDD& rhs) {
            if (&rhs == this)
                return *this;
            TopLevelAction::operator=(rhs);
            copy(rhs);
            return *this;
        }

        void copy(const TopLevelActionDD& tla) {
            beliefDD = tla.beliefDD;
            beliefDD_ps = tla.beliefDD_ps;
        }

        DiscreteDistributionDD getBeliefDD() const { return beliefDD; };
        DiscreteDistributionDD getBeliefDD_ps() const { return beliefDD_ps; };

        void setBeliefDD(const DiscreteDistributionDD& _belief) {
            beliefDD = _belief;
        };

		void setBeliefDD_ps(const DiscreteDistributionDD& _belief) {
            beliefDD_ps = _belief;
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
        } else if (expansionModule == "riskDD") {
            expansionAlgo = make_shared<RiskDD<Domain, Node, TopLevelAction>>(
                    domain, lookahead, 1);
        }  else if (expansionModule == "riskDDSquish") {
            expansionAlgo = make_shared<RiskDDSquish<Domain, Node, TopLevelAction>>(
                    domain, lookahead, 1);
        }   else if (expansionModule == "ie") {
            //expansionAlgo = make_shared<RiskIE<Domain, Node, TopLevelAction>>(
                    //domain, lookahead);
            expansionAlgo = make_shared<AStar<Domain, Node, TopLevelAction>>(
                    domain, lookahead, "lowerconfidence");
        } else {
            expansionAlgo = make_shared<AStar<Domain, Node, TopLevelAction>>(
                    domain, lookahead, "f");
            DEBUG_MSG( "not specified expansion module type, use Astart" );
        }

        if (learningModule == "none") {
            learningAlgo =
                    make_shared<Ignorance<Domain, Node, TopLevelAction>>();
        } else if (learningModule == "learn") {
            learningAlgo =
                    make_shared<Dijkstra<Domain, Node, TopLevelAction>>(domain);
        } else if (learningModule == "learnDD") {
            learningAlgo = make_shared<
                    DijkstraDistribution<Domain, Node, TopLevelAction>>(
                    domain);
        } else {
            learningAlgo =
                    make_shared<Dijkstra<Domain, Node, TopLevelAction>>(domain);
        }

        if (decisionModule == "minimin") {
            decisionAlgo =
                    make_shared<ScalarBackup<Domain, Node, TopLevelAction>>(
                            "minimin");
        } else if (decisionModule == "bellman") {
            decisionAlgo =
                    make_shared<ScalarBackup<Domain, Node, TopLevelAction>>(
                            "bellman");
        } else if (decisionModule == "nancy") {
            decisionAlgo =
                    //make_shared<NancyBackup<Domain, Node, TopLevelAction>>(
                            //domain, lookahead);
                    make_shared<KBestBackup<Domain, Node, TopLevelAction>>(
                            domain, 1, lookahead);
        } else if (decisionModule == "nancy-persist") {
            decisionAlgo =
                    //make_shared<NancyBackup<Domain, Node, TopLevelAction>>(
                            //domain, lookahead);
                    make_shared<KBestBackup<Domain, Node, TopLevelAction>>(
                            domain, 1, lookahead);
        } else if (decisionModule == "nancyDD") {
            decisionAlgo = make_shared<
                    NancyDDDecision<Domain, Node, TopLevelAction>>(
                    domain, lookahead);
        } else {
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
                    domain.heuristic(domain.getStartState()),
                    domain.getStartState(),
                    nullptr,
                    -1);
        }

        int count = 0;

        // while (count <= iterationlimit) {
        while (1) {
            clock_t startTime = clock();

            // mark this node as the start of the current search (to
            // prevent state pruning based on label)
            start->markStart();

            //DEBUG_MSG("start: " << start->getState());

            count++;

            // if (beliefType == "data") {
            // DEBUG_MSG( "rl loop " << count << "h "
            //<< start->getFValue() - start->getGValue() );
            //}

            // Check if a goal has been reached
            if (domain.isGoal(start->getState())) {
                res.solutionFound = true;
				res.epsilonHGlobal = domain.epsilonHGlobal();
				res.epsilonDGlobal = domain.epsilonDGlobal();

                return res;
            }

            restartLists(start);

            // Exploration Phase
            domain.updateEpsilons();

            if (beliefType == "normal") {
                generateTopLevelActions(start, res);
                //DEBUG_MSG("after gen tlas");
                expansionAlgo->expand(
                        open, closed, tlas, duplicateDetection, res);
                //DEBUG_MSG("after lookahead");
            } else if (beliefType == "data") {
                generateTopLevelActionsDD(start, res);
                expansionAlgo->expand(
                        open, closed, tlas, duplicateDetectionDD, res);
            } else {
                DEBUG_MSG("Realtime search main loop line 370: wrong "
                          "belief "
                          "type!!!");
                exit(1);
            }

            // Check if this is a dead end
            if (open.empty()) {
                break;
            }

            // Decision-making Phase
            start = decisionAlgo->backup(open, tlas, start, closed);

            //DEBUG_MSG( "after decision");
            /*DEBUG_MSG( "h " << start->getHValue() << " hat "*/
                 //<< start->getHHatValueFromDist() );

            DEBUG_MSG("iteration: " << count );

            // LearninH Phase
            learningAlgo->learn(open, closed);


            // Add this step to the path taken so far
            //res.path.push(start->getState().getLabel());
			res.path.push(start->getState().toString());
            res.solutionCost += start->getGValue();

            res.lookaheadCpuTime.push_back(
                    double(clock() - startTime) / CLOCKS_PER_SEC);
        }

        // cout<<"iteration: " << count<<endl;

        // if (count >= iterationlimit)
        // noSolutionFound(res);

        return res;
    }

private:
    static bool duplicateDetection(shared_ptr<Node> node,
            unordered_map<State, shared_ptr<Node>, Hash>& closed,
            PriorityQueue<shared_ptr<Node>>& open,
            vector<shared_ptr<TopLevelAction>>& tlaList) {
        // Check if this state exists
        typename unordered_map<State, shared_ptr<Node>, Hash>::iterator it =
                closed.find(node->getState());

        if (it != closed.end()) {
            // This state has been generated before, check if its node is on
            // OPEN
            if (it->second->onOpen()) {
                // This node is on OPEN, keep the better g-value
                if (node->getGValue() < it->second->getGValue()) {
                    tlaList[it->second->getOwningTLA()]->open_TLA.remove(it->second);
                    it->second->setGValue(node->getGValue());
                    it->second->setParent(node->getParent());
                    it->second->setHValue(node->getHValue());
                    it->second->setDValue(node->getDValue());
                    it->second->setDErrValue(node->getDErrValue());
                    it->second->setEpsilonH(node->getEpsilonH());
                    it->second->setEpsilonD(node->getEpsilonD());
                    it->second->setState(node->getState());
                    it->second->setOwningTLA(node->getOwningTLA());
                    tlaList[node->getOwningTLA()]->open_TLA.push(it->second);
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
                    tlaList[node->getOwningTLA()]->open_TLA.push(it->second);
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
            vector<shared_ptr<TopLevelAction>>& tlaList) {
        // Check if this state exists
        typename unordered_map<State, shared_ptr<Node>, Hash>::iterator it =
                closed.find(node->getState());

        if (it != closed.end()) {
            // This state has been generated before, check if its node is on
            // OPEN
            if (it->second->onOpen()) {
                // This node is on OPEN, keep the better g-value
				//
				// we only update the g value, and the parent,
				// since everything else should be identical with the same state
				// then change the owing tla
                if (node->getGValue() < it->second->getGValue()) {
                    tlaList[it->second->getOwningTLA()]->open_TLA.remove(it->second);
                    it->second->setGValue(node->getGValue());
                    it->second->setParent(node->getParent());
                    it->second->setOwningTLA(node->getOwningTLA());
                    tlaList[node->getOwningTLA()]->open_TLA.push(it->second);
                }
            } else {
                // This node is on CLOSED, compare the f-values. If this new
                // f-value is better, reset g, h, and d.
                // Then reopen the node.
				//
				// here for dd we compare g, because for the same state, 
				// their h value are the same.
                if (node->getGValue() < it->second->getGValue()) {
                    it->second->setGValue(node->getGValue());
                    it->second->setParent(node->getParent());
                    it->second->setOwningTLA(node->getOwningTLA());
                    tlaList[node->getOwningTLA()]->open_TLA.push(it->second);
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

        if (children.size() == 0) {
            cerr<<"dead end, no TLA kids!\n";
            exit(1);
        }

        //DEBUG_MSG( "cur " << start->getState().toString()
             //<< " kids: " << children.size() << "\n";

        State bestChild;
        Cost bestF = numeric_limits<double>::infinity();

        for (State child : children) {
            //DEBUG_MSG("TLA kid: " << child);
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

            //DEBUG_MSG("TLA kid F: " << childNode->getFValue());
            //DEBUG_MSG("TLA kid G: " << childNode->getGValue());
            //DEBUG_MSG("TLA kid H: " << childNode->getHValue());

            if (childNode->getFValue() < bestF) {
                bestF = childNode->getFValue();
                bestChild = child;
                //DEBUG_MSG("TLA kids copied to best, child" << child);
                //DEBUG_MSG("TLA kids copied to best, best" << bestChild);
            }

            // No top level action will ever be a duplicate, so no need to
            // check.
            // Make a new top level action and push this node onto its open
            shared_ptr<TopLevelAction> tla = make_shared<TopLevelAction>();
            tla->topLevelNode = childNode;

         /*   childNode->distribution = make_shared<DiscreteDistribution>(100,*/
                    //childNode->getFValue(),
                    //childNode->getFHatValue(),
                    //childNode->getDValue(),
                    //childNode->getFHatValue() - childNode->getFValue());

            tla->expectedMinimumPathCost = childNode->getFHatValue();

            // Push this node onto open and closed
            closed[child] = childNode;
            open.push(childNode);
            tla->open_TLA.push(childNode);

            // Add this top level action to the list
            tlas.push_back(tla);
        }

        //DEBUG_MSG("best TLA kids " << bestChild);

        // Learn one-step error
        if (!children.empty()) {
            Cost epsD = (1 + domain.distance(bestChild)) - start->getDValue();
            Cost epsH = (domain.getEdgeCost(bestChild) +
                                domain.heuristic(bestChild)) -
                    start->getHValue();

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

        if (children.size() == 0) {
            cerr<<"dead end, no TLA kid!\n";
            exit(1);
        }

        for (State child : children) {
            shared_ptr<Node> childNode = make_shared<Node>(
                    start->getGValue() + domain.getEdgeCost(child),
                    domain.hstart_distribution(child),
                    domain.hstart_distribution_ps(child),
					domain.heuristic(child),
                    child,
                    start,
                    tlas.size());

            // No top level action will ever be a duplicate, so no need to
            // check.
            // Make a new top level action and push this node onto its open
            shared_ptr<TopLevelAction> tla = make_shared<TopLevelActionDD>();

            tla->topLevelNode = childNode;

            childNode->hStartDistribution = domain.hstart_distribution(child);

            childNode->hStartDistribution_ps = domain.hstart_distribution_ps(child);

            tla->expectedMinimumPathCost =
                    childNode->hStartDistribution.expectedCost() +
                    childNode->getGValue();

			tla->h_TLA  = childNode->getHValue();

            // Push this node onto open and closed
            closed[child] = childNode;
            open.push(childNode);
            tla->open_TLA.push(childNode);

            // Add this top level action to the list
            tlas.push_back(tla);
        }
    }

    void restartLists(shared_ptr<Node> start) {
        // clear the TLA list
        tlas.clear();

        // Empty OPEN and CLOSED
        open.clear();

        // delete all of the nodes from the last expansion phase
        closed.clear();

		//reset start g as 0
		start->setGValue(0);

		start->setParent(nullptr);
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
	vector<shared_ptr<TopLevelAction>> tlas;

	double lookahead;
	string beliefType;
	string expansionPolicy;
	string learningPolicy;
	string decisionPolicy;
};
