#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <limits>
#include <math.h>
#include "../utility/ResultContainer_sai.h"
#include "../utility/DiscreteDistribution.h"
using namespace std;

template <class Domain>
class THTS_RT{
    typedef typename Domain::State State;
    typedef typename Domain::Cost Cost;
    typedef typename Domain::HashState Hash;

public:
    struct Node {
		Node* parent;
        State state; // State
        unordered_set<Node*> successors; // Children
        double value; 
        int visits; 
        bool initialized;
        double edgeCost;
        double g;
        double h;
        double d;
        double derr;
        double epsH;
        double epsD;
        double frontierH;
        double frontierG;


    public:
        Node(State state, int visit, Node* parent, double g, double h, double d, double derr, double epsH, double epsD, double edgeCost)
                        : state(state), visits(visit), parent(parent), g(g), h(h), d(d), derr(derr), epsH(epsH), epsD(epsD), edgeCost(edgeCost){
            initialized = false;
            value = 0;
            frontierH = 0;
            frontierG = 0;

        }

        State getState() const {return state;}
        double getEdgeCost() const {return edgeCost;}
        double getGValue() const {return g;}
        double getDValue() const {return d;}
        double getHValue() const {return h;}
        double getFValue() const {return getGValue() + h;}
        double getFHatValue() const {return getGValue() + getHHatValue();}
        double getDHatValue() const {return (derr / (1.0 - epsD));}
        double getHHatValue() const {return h + getDHatValue() * epsH;}
    };

    struct maxG {
        bool operator()(const Node* n1, const Node* n2){
            if (n1->getGValue() == n2->getGValue()){
                return rand() % 2;
            }
            return n1->getGValue() < n2->getGValue();
        }
    };

    struct minH {
        bool operator()(const Node* n1, const Node* n2){
			if (n1->h == n2->h){
                n1->getEdgeCost() > n2->getEdgeCost();
			}
			return n1->h > n2->h;
        }
    };

    struct minFrontierH {
        bool operator()(const Node* n1, const Node* n2){
			if (n1->frontierH == n2->frontierH){
                n1->getEdgeCost() > n2->getEdgeCost();
			}
			return n1->frontierH > n2->frontierH;
        }
    };

    struct minFrontierF {
        bool operator()(const Node* n1, const Node* n2){
			if (n1->frontierH + n1->frontierG == n2->frontierH + n2->frontierG){
                n1->frontierH > n1->frontierH;
			}
			return n1->frontierH + n1->frontierG > n2->frontierH + n2->frontierG;
        }
    };

    typedef priority_queue<Node*, vector<Node*>, maxG> PQueueMaxG;
    typedef priority_queue<Node*, vector<Node*>, minH> PQueueMinH; 
    typedef priority_queue<Node*, vector<Node*>, minH> PQueueMinFrontierH;
    typedef priority_queue<Node*, vector<Node*>, minH> PQueueMinFrontierF;
    
    THTS_RT(Domain& domain, string algorithm, int lookahead, bool greedyOneStep = false, bool hLearning = false) 
                    : domain(domain), algorithm(algorithm), lookahead(lookahead), greedyOneStep(greedyOneStep), hLearning(hLearning) {
        srand(1);
        if (algorithm == "AS"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
        } else if (algorithm == "ASnancy"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
            backup_type = "nancy";
        } else if (algorithm == "ASie"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
            backup_type = "ie";
        } else if (algorithm == "ASiep"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
            backup_type = "iep";
        } else if (algorithm == "WAS"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
            w = 5;
        } else if (algorithm == "WASnancy"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
            backup_type = "nancy";
            w = 5; 
        } else if (algorithm == "WASie"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
            backup_type = "ie";
            w = 5; 
        } else if (algorithm == "WASiep"){
            trial_expansion = "bfs";
            trial_backup = "bfs";
            backup_type = "iep";
            w = 5; 
        } else if (algorithm == "UCT" || algorithm == "GUCT"){
            trial_expansion = "uct";
            trial_backup = "uct";
        } else if (algorithm == "UCTS" || algorithm == "GUCTS"){
            trial_expansion = "uct";
            trial_backup = "bfs";
        } else if (algorithm == "UCTnancy" || algorithm == "GUCTnancy"){
            trial_expansion = "uct";
            trial_backup = "uct";
            backup_type = "nancy";
        } else if (algorithm == "UCTSnancy" || algorithm == "GUCTSnancy"){
            trial_expansion = "uct";
            trial_backup = "bfs";
            backup_type = "nancy";
        } else if (algorithm == "UCTie" || algorithm == "GUCTie"){
            trial_expansion = "uct";
            trial_backup = "uct";
            backup_type = "ie";
        } else if (algorithm == "UCTSie" || algorithm == "GUCTSie"){
            trial_expansion = "uct";
            trial_backup = "bfs";
            backup_type = "ie";
        } else if (algorithm == "UCTiep" || algorithm == "GUCTiep"){
            trial_expansion = "uct";
            trial_backup = "uct";
            backup_type = "iep";
        } else if (algorithm == "UCTSiep" || algorithm == "GUCTSiep"){
            trial_expansion = "uct";
            trial_backup = "bfs";
            backup_type = "iep";
        } else {
            cout << "Invalid algorithm: " << algorithm << endl;
            exit(1);
        }  

        if (algorithm[0] == 'G'){
            k = 0;
        }
    }

    double getIE(Node* n, double z){
        // lower = -z, higher = +z
        double mean =  n->getHHatValue();
        if (n->h == mean){
            return n->h;
        }
        double error = mean - n->h;
        double stdDev = error / 2.0;
        double var = pow(stdDev, 2);
        // 1.96 is the Z value from the Z table to get the 2.5 confidence
        return max(n->h, mean + (z * var));
    }

    void resetNode(Node* n){
        n->parent = NULL;
        n->successors.clear();
        n->initialized = false;
    }

    void updateValue(Node* n){
        if (backup_type == "nancy"){
            n->value = DiscreteDistribution(100, n->getHValue(), n->getHHatValue(), 
                            n->getDValue(), n->getHHatValue() - n->getHValue()).expectedCost();
        } else if (backup_type == "ie"){
            n->value = getIE(n, -1.96);
         } else if (backup_type == "iep"){
            n->value = getIE(n, 1.96);
        } else {
            n->value = w * domain.heuristic(n->getState());
        }
    }

    void updatePrevNode(Node* n){
        // Change current state h to the sencond best, RTA* style
        // RTA* style, update state that is leaving to the second best f of it neighbors
        // Represents the estimated h cost of solving the problem by returning to this state
        // whereas LRTA* style uses min f, which will converge eventually

        priority_queue<double, vector<double>, greater<double>> minheap;
        for (Node* child : n->parent->successors){
            minheap.push(n->parent->g + child->edgeCost + child->h);
        } 
        if (minheap.size() > 1){
            minheap.pop();
        }
        if (domain.heuristic(n->parent->state) < minheap.top()){
            domain.updateHeuristic(n->parent->state, minheap.top());
        }
    }

    ResultContainer getPlan(){
        // Algorithm search begins here
        ResultContainer res;
        State root_state = domain.getStartState();
        res.solutionCost = 0;
        domain.initialize(algorithm, lookahead);

        Node* root = new Node(root_state, 1, NULL, 0, domain.heuristic(root_state), domain.distance(root_state), 
                            domain.distanceErr(root_state), domain.epsilonHGlobal(), domain.epsilonDGlobal(), 0);
        updateValue(root);
        root->frontierH = root->h;

        while(true){
            if (domain.isGoal(root->state)){
                res.solutionFound = true;
                break;
            }

            unordered_map<State, Node*, Hash> TREE;
            TREE[root->state] = root;
            domain.updateEpsilons();

            // Expansion phase
            for(int exp = 0; exp < lookahead; ++exp){
                if (performTrial(TREE, res, root)){
                    // Goal is reached
                    break;
                }
            }

            root = oneStepAction(root, TREE);
            updatePrevNode(root);
            resetNode(root);

            // Add this step to the path taken so far
            if (record_plan){
                res.path.push(root->state.getLabel());
            }

            // Clean up tree
            for (auto it : TREE){
                if (it.second != root){
                    delete(it.second);
                }
            }
        }

        res.solutionCost = root->getGValue();
        delete(root);
        return res;
    }

    bool performTrial(unordered_map<State, Node*, Hash>& TREE, ResultContainer& res, Node* root){
        PQueueMaxG pqMaxG;
        Node* n = root;
        while (n->initialized){           
            State tmp_state = n->state;
            n = trialAction(n, TREE);
            if (tmp_state == n->state){
                cout << "Deadend" << endl;
                exit(1);
            }
        }

        if (domain.isGoal(n->getState())){
            res.solutionFound = true;
            return true; 
        }

        expandNode(n, pqMaxG, TREE, res);
        pqMaxG.push(n);

        while (!pqMaxG.empty()){
            n = pqMaxG.top();
            pqMaxG.pop();
            backupNode(n, TREE);

            if (n != root){
                pqMaxG.push(n->parent);
                if(TREE.find(n->state) == TREE.end()){
                    delete(n);
                }
            }
        }

        return false;
    }

    void backupNode(Node* n, unordered_map<State, Node*, Hash>& TREE){
        if (n->successors.size() == 0){
            n->parent->successors.erase(n);
            TREE.erase(n->state);
            return;
        }
        priority_queue<double, vector<double>, greater<double>> pqMinHEdge;
        priority_queue<double, vector<double>, greater<double>> pqminFrontierH;        

        int visits = 0;
        if (trial_backup == "bfs" ){

            priority_queue<double, vector<double>, greater<double>> pqminValue;
            for (Node* child : n->successors){
                visits += child->visits;
                pqMinHEdge.push(domain.heuristic(child->state) + child->getEdgeCost());
                pqminFrontierH.push(child->frontierH);

                pqminValue.push(child->value + k * child->getEdgeCost());
            }

            n->value = pqminValue.top();
        } else if (trial_backup == "uct"){
            double value = 0;
            for (Node* child : n->successors){
                visits += child->visits;
                pqMinHEdge.push(domain.heuristic(child->state) + child->getEdgeCost());
                pqminFrontierH.push(child->frontierH);

                value += child->visits * (child->value + k * child->edgeCost);
            }

            n->value = value/visits;
        } else {
            cout << "Invalid backup algorithm: " << trial_backup << endl;
            exit(1);
        }

        n->visits = visits;
        n->frontierH = pqminFrontierH.top();
        if (hLearning && domain.heuristic(n->state) > pqMinHEdge.top()){
            domain.updateHeuristic(n->state, pqMinHEdge.top());
            n->h = pqMinHEdge.top();
        }
    }

    void updateSuccessors(Node* n){
        n->g = n->parent->g + n->edgeCost;
        priority_queue<double, vector<double>, greater<double>> pqminFrontierG;        

        for (Node* child : n->successors){
            updateSuccessors(child);
            pqminFrontierG.push(child->frontierG);
        }

        if (n->successors.size() == 0){
            n->frontierG = n->getGValue();
        } else {
            n->frontierG = pqminFrontierG.top();
        }
    }

    void expandNode(Node* n, PQueueMaxG& backUpQueue, unordered_map<State, Node*, Hash>& TREE, ResultContainer& res){
        res.nodesExpanded++;
        vector <State> children = domain.successors(n->state);
        res.nodesGenerated += children.size();

        State bestChild;
		double bestF = numeric_limits<double>::infinity();

        for (State child : children){
            auto it = TREE.find(child);
                
            if (it == TREE.end()){
                double edgeCost = domain.getEdgeCost(child);
                Node* childNode = new Node(child, 1, n, edgeCost + n->getGValue(), domain.heuristic(child), domain.distance(child), 
                            domain.distanceErr(child), domain.epsilonHGlobal(), domain.epsilonDGlobal(), edgeCost);
                updateValue(childNode);
                childNode->frontierH = childNode->getHValue();
                childNode->frontierG = childNode->getGValue();
                TREE[child] = childNode;
                n->successors.insert(childNode);
            } else {
                Node* s = (TREE[child]);
                if (n->getGValue() + s->getEdgeCost() < s->getGValue()) {
                    backUpQueue.push(s->parent);
                    s->parent->successors.erase(s);
                    s->parent = n;
                    s->state = child; // update label
                    s->edgeCost = domain.getEdgeCost(child);
                    n->successors.insert(s);
                    updateSuccessors(s);
                }
            }

            if (!TREE[child]->initialized){
                if (TREE[child]->getFValue() < bestF){
                    bestF = TREE[child]->getFValue();
                    bestChild = child;
                }
            }
        }

        // Learn one-step error
        if (bestF != numeric_limits<double>::infinity()){
            double epsD = (1 + domain.distance(bestChild)) - n->d;
            double epsH = (domain.getEdgeCost(bestChild) + domain.heuristic(bestChild)) - n->h;
            domain.pushEpsilonHGlobal(epsH);
            domain.pushEpsilonDGlobal(epsD);
        }
        n->initialized = true;
    }
    
    Node* oneStepAction(Node* n, unordered_map<State, Node*, Hash>& TREE){
        if (greedyOneStep){
            PQueueMinH pqMinFrontierH;
            for (Node* child : n->successors){
                pqMinFrontierH.push(child);
            }
            return pqMinFrontierH.top();
        } else {
            return selectBestFirst(n);
        }
    }

    Node* selectBestFirst(Node* n){
        priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<pair<double, Node*>> > pqMin; 
        for (Node* child : n->successors){
            pqMin.push(make_pair(child->value + k * child->edgeCost, child));
        }

        // Break ties on h
        PQueueMinH pqMinH;
        double best = pqMin.top().first;
        while(!pqMin.empty() && pqMin.top().first == best){
            pqMinH.push(pqMin.top().second);
            pqMin.pop();
        }
        return pqMinH.top();
    }

    Node* selectUCT(Node* n){
        priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<pair<double, Node*>> > pqMin; 
        double min = numeric_limits<double>::infinity();
        double max = 0;
        vector <double> childrenFBar;

        // Finiding the max and min for normalization
        for (Node* child : n->successors){
            double child_value = child->value + k * child->edgeCost;
            if (child_value < min){
                min = child_value;
            }
            if (child_value > max){
                max = child_value;
            }
            childrenFBar.push_back(child_value);
        }

        int i  = 0;
        double denom = max - min;
        for (Node* child : n->successors){
            double child_value;
            double fb = 0;

            int self_visits = n->visits;
            int child_visits = child->visits;
            double csq = (C * sqrt(log(self_visits)/child_visits));

            if (denom != 0){
                fb = (childrenFBar[i] - min)/denom;
            }

            child_value = fb - csq;
            pqMin.push(make_pair(child_value, child));
            ++i;
        }
        
        // Break ties on h
        PQueueMinH pqMinH;
        double best = pqMin.top().first;
        while(!pqMin.empty() && pqMin.top().first == best){
            pqMinH.push(pqMin.top().second);
            pqMin.pop();
        }
        return pqMinH.top();
    }
    
    
    Node* trialAction(Node* n, unordered_map<State, Node*, Hash>& TREE){
        if (n->successors.size() == 1){
            n = *(n->successors.begin());
            return n;
        }
        if (trial_expansion == "bfs"){
            return selectBestFirst(n);
        } else if (trial_expansion == "uct"){
            return selectUCT(n);
        } else {
            cout << "Invalid decision algorithm" << endl;
            exit(1);
        }  
    }

    void setRecordPlan(bool b){
        record_plan = b;
    }

protected:
    Domain& domain;
    double w = 1;
    double k = 1;
    int lookahead;
    double C = 1.414; // exploration parameter C
    bool greedyOneStep;
    bool hLearning;
    string algorithm;
    string trial_expansion;
    string trial_backup;
    string backup_type;
    bool record_plan = false;
};
