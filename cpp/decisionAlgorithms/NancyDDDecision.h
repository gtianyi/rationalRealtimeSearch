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
        // we have to do one more back up, because we havn't back up the frontier
		// after the last expansion.
        NancyDDBackup<Node, TopLevelAction>::backup2TLA(tlas);

        // Take the TLA with the lowest expected minimum path cost
        TopLevelAction lowestExpectedPathTLA = tlas[0];
        for (const TopLevelAction& tla : tlas) {
            cout << "expectedMinimumPathCost of TLA "
                 << tla.expectedMinimumPathCost << endl;

            /*if (tla.expectedMinimumPathCost ==*/
                    //lowestExpectedPathTLA.expectedMinimumPathCost) {
                //if (tla.getF_TLA() < lowestExpectedPathTLA.getF_TLA())
                    //lowestExpectedPathTLA = tla;
            //} else if (tla.expectedMinimumPathCost <
                    //lowestExpectedPathTLA.expectedMinimumPathCost)
                /*lowestExpectedPathTLA = tla;*/

            if (tla.expectedMinimumPathCost <
                    lowestExpectedPathTLA.expectedMinimumPathCost)
                lowestExpectedPathTLA = tla;

            /*cout << "tla size: " << tla.open_TLA.size() << endl;*/
            //cout << "tla open top: " << tla.open_TLA.top() << endl;
            //cout << "tla open top h: " << tla.open_TLA.top()->getHValue()
                 //<< endl;
            //cout << "tla open top original h: "
                 //<< domain.manhattanDistance(tla.open_TLA.top()->getState()) << endl;
            //cout << "tla open top dist exp: "
                 //<< tla.open_TLA.top()->getHstartDistribution().expectedCost()
                 //<< endl;
            /*cout << "tla exp cost: " << tla.expectedMinimumPathCost << endl;*/
        }

        shared_ptr<Node> goalPrime = lowestExpectedPathTLA.topLevelNode;

        return goalPrime;
    }

protected:
    Domain& domain;
    double lookahead;
};
