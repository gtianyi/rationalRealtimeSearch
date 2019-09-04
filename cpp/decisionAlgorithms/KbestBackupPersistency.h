#pragma once
#include <functional>
#include <limits>
#include <iostream>
#include <memory>
#include <stack>
#include "../utility/PriorityQueue.h"
#include "KBestBackup.h"

using namespace std;

template <class Domain, class Node, class TopLevelAction>
class KBestBackupPersistency : public KBestBackup<Domain, Node, TopLevelAction> {
    typedef typename Domain::State State;
    typedef typename Domain::Cost Cost;
    typedef typename Domain::HashState Hash;

public:
    KBestBackupPersistency(Domain& domain, double k, double lookahead)
            : KBestBackup<Domain, Node, TopLevelAction>(domain, k, lookahead) {}

    shared_ptr<Node> backup(PriorityQueue<shared_ptr<Node>>& open,
            vector<shared_ptr<TopLevelAction>>& tlas,
            shared_ptr<Node> start,
            unordered_map<State, shared_ptr<Node>, Hash>& closed) {
        this->kBestDecision(tlas);

        // Take the TLA with the lowest expected minimum path cost
        shared_ptr<TopLevelAction> lowestExpectedPathTLA = tlas[0];
        for (auto tla : tlas) {
            if (tla->expectedMinimumPathCost <
                    lowestExpectedPathTLA->expectedMinimumPathCost)
                lowestExpectedPathTLA = tla;
        }

        shared_ptr<Node> goalPrime = lowestExpectedPathTLA->topLevelNode;

        auto tlaBest = lowestExpectedPathTLA->open_TLA.top();

        if (persistPath.empty() ||
                lowestExpectedPathTLA->expectedMinimumPathCost < persistFhat) {
            // if there is no persist path, go head memoize it
            // if we find a better fhat for root, go head memoize it
            memoizePersistPath(lowestExpectedPathTLA);
        } else if (lowestExpectedPathTLA->expectedMinimumPathCost ==
                        persistFhat &&
                tlaBest->getHHatValueFromDist() <=
                        persistTarget->getHHatValueFromDist()) {
            // if  fhat ties break ties on h-hat then prefer not persist
            memoizePersistPath(lowestExpectedPathTLA);
        } else{
            // if we find a worse fhat, but previous target is inside LSS, 
			// we then still want to memoize it because the learning then 
			// will update the previous target.
            auto it = closed.find(persistTarget->getState());
            if (it != closed.end() && !it->second->onOpen()) {
                memoizePersistPath(lowestExpectedPathTLA);
            } else {
                cout << "persist" << endl;
                cout << "previous fhat " << persistFhat << endl;
                cout << "new fhat "
                     << lowestExpectedPathTLA->expectedMinimumPathCost +
                                lowestExpectedPathTLA->topLevelNode->getGValue()
                     << endl;
			}
        }

        goalPrime = persistPath.top();
		persistPath.pop();
		persistFhat -= goalPrime->getGValue();

        return goalPrime;
    }

private:
    void memoizePersistPath(shared_ptr<TopLevelAction> tla) {
		//clear the persist path
        while (!persistPath.empty()) {
            persistPath.pop();
        }

        auto cur = tla->open_TLA.top();

        while (cur->getParent() != nullptr) {
            persistPath.push(cur);
            cur = cur->getParent();
        }

        persistTarget = tla->open_TLA.top();
        persistFhat = tla->expectedMinimumPathCost;
    }

protected:
    std::stack<shared_ptr<Node>> persistPath;
    shared_ptr<Node> persistTarget;
    Cost persistFhat;
};
