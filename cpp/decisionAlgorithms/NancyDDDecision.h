#pragma once
#include <functional>
#include <limits>
#include <iostream>
#include <memory>
#include "../utility/PriorityQueue.h"
#include "DecisionAlgorithm.h"
#include "NancyDDBackup.h"

using namespace std;

template <class Domain, class Node, class TopLevelAction>
class NancyDDDecision : public DecisionAlgorithm<Domain, Node, TopLevelAction> {
    typedef typename Domain::State State;
    typedef typename Domain::Cost Cost;

public:
    NancyDDDecision(Domain& domain, double lookahead)
            : domain(domain),
              lookahead(lookahead) {}

    shared_ptr<Node> backup(PriorityQueue<shared_ptr<Node>>& open,
            vector<TopLevelAction>& tlas,
            shared_ptr<Node> start) {
		// we might can avoid do this because we acturally do this in every expansion
        NancyDDBackup<Node, TopLevelAction>::backup2TLA(tlas);

        // Take the TLA with the lowest expected minimum path cost
        TopLevelAction lowestExpectedPathTLA = tlas[0];
        for (TopLevelAction tla : tlas) {
            if (tla.expectedMinimumPathCost <
                    lowestExpectedPathTLA.expectedMinimumPathCost)
                lowestExpectedPathTLA = tla;
        }

        shared_ptr<Node> goalPrime = lowestExpectedPathTLA.topLevelNode;

        return goalPrime;
    }

protected:
    Domain& domain;
    double lookahead;
};
