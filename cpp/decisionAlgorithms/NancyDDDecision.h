#pragma once
#include <functional>
#include <limits>
#include <iostream>
#include <memory>
#include <stack>
#include "../utility/PriorityQueue.h"
#include "DecisionAlgorithm.h"
#include "NancyDDBackup.h"

using namespace std;

template <class Domain, class Node, class TopLevelAction>
class NancyDDDecision : public DecisionAlgorithm<Domain, Node, TopLevelAction> {
    typedef typename Domain::State State;
    typedef typename Domain::Cost Cost;
    typedef typename Domain::HashState Hash;

public:
    NancyDDDecision(Domain& domain, double lookahead)
            : domain(domain),
              lookahead(lookahead) {}

    shared_ptr<Node> backup(PriorityQueue<shared_ptr<Node>>& open,
            vector<TopLevelAction>& tlas,
            shared_ptr<Node> start,
            unordered_map<State, shared_ptr<Node>, Hash>& closed) {
        // we have to do one more back up, because we havn't back up the frontier
		// after the last expansion.
        NancyDDBackup<Node, TopLevelAction>::backup2TLA(tlas);

        // Take the TLA with the lowest expected minimum path cost
        TopLevelAction lowestExpectedPathTLA = tlas[0];
        for (const TopLevelAction& tla : tlas) {
            if (tla.expectedMinimumPathCost <
                    lowestExpectedPathTLA.expectedMinimumPathCost)
                lowestExpectedPathTLA = tla;
        }

        shared_ptr<Node> goalPrime;

        if (persistPath.empty() ||
                lowestExpectedPathTLA.expectedMinimumPathCost +
                                lowestExpectedPathTLA.topLevelNode
                                        ->getGValue() <=
                        persistFhat) {
            // if there is no persist path, go head memoize it
            // if we find a better fhat for root, go head memoize it
            memoizePersistPath(lowestExpectedPathTLA);
            cout << "persist" << endl;
        } else {
            // if we find a worse fhat, but previous target is inside LSS, 
			// we then still want to memoize it because the learning then 
			// will update the previous target.
            auto it = closed.find(persistTarget->getState());
            if (it != closed.end() && !it->second->onOpen()) {
                memoizePersistPath(lowestExpectedPathTLA);
                cout << "persist" << endl;
            }
        }

        goalPrime = persistPath.top();
		persistPath.pop();
		persistFhat -= goalPrime->getGValue();

        return goalPrime;
    }

private:
    void memoizePersistPath(TopLevelAction& tla) {
		//clear the persist path
        while (!persistPath.empty()) {
            persistPath.pop();
        }

        auto cur = tla.open_TLA.top();

        while (cur->getParent() != nullptr) {
            persistPath.push(cur);
            cur = cur->getParent();
        }

        persistTarget = tla.open_TLA.top();
        persistFhat =
                tla.expectedMinimumPathCost + tla.topLevelNode->getGValue();
    }

protected:
    Domain& domain;
    std::stack<shared_ptr<Node>> persistPath;
    shared_ptr<Node> persistTarget;
    Cost persistFhat;
    double lookahead;
};
