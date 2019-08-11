#pragma once
#include <functional>
#include <limits>
#include <iostream>
#include <memory>
#include "../utility/PriorityQueue.h"

using namespace std;

template <class Domain, class Node, class TopLevelAction>
class NancyDDBackup{
    typedef typename Domain::State State;
    typedef typename Domain::Cost Cost;

public:
    void csernaBackup(TopLevelAction& tla) {
        // We assume in k-best that only the k-best nodes matter.
        if (!tla.kBestNodes.empty()) {
            // Perform Cserna Backups on k-best nodes
            while (tla.kBestNodes.size() > 1) {
                // Take the first two and do a Cserna backup...
                tla.kBestNodes[0]->distribution =
                        tla.kBestNodes[0]->distribution *
                        tla.kBestNodes[1]->distribution;
                // Remove the other used in the backup
                tla.kBestNodes.erase(remove(tla.kBestNodes.begin(),
                                             tla.kBestNodes.end(),
                                             tla.kBestNodes[1]),
                        tla.kBestNodes.end());
            }

            // Get the expected value of the resulting Cserna Distribution
            tla.expectedMinimumPathCost =
                    tla.kBestNodes[0]->distribution.expectedCost();
            tla.belief = tla.kBestNodes[0]->distribution;
        } else {
            // If nothing was expanded under this TLA, use the expected value of
            // the TLA
            tla.expectedMinimumPathCost = numeric_limits<double>::infinity();
        }
    }

	//let's make it purely nancy, not a variation of k-best, which is a variation of csernaBackup
    static void backup2TLA(vector<TopLevelAction>& tlas) {
        // The K-Best decision assumes that the only nodes within the subtrees
        // of the TLAs are the k-best frontier nodes
        // on their opened lists. Find them.
        for (TopLevelAction& tla : tlas) {
            tla.kBestNodes.clear();

            // If this TLA has unique, probably optimal subtrees beneath it, it
            // is valid

            int i = 0;
            // Add to the best k nodes while i < k and non-selected nodes exist
            // on the frontier
            while (i < k && !tla.open.empty()) {
                shared_ptr<Node> best = tla.open.top();
                tla.open.pop();

                // Make this node's PDF a discrete distribution...
                if (beliefType == "normal") {
                    best->distribution = DiscreteDistribution(100,
                            best->getFValue(),
                            best->getFHatValue(),
                            best->getDValue(),
                            best->getFHatValue() - best->getFValue());
				//we should not create a new distribution here for dd
                } else if (beliefType == "data") {
                    bool retSucc, retSucc_ps;
                    best->distribution = DiscreteDistribution(
                            best->getGValue(),
                            best->getFValue()-best->getGValue(), retSucc);
                    best->distribution_ps = DiscreteDistribution(best->getGValue(),
                            best->getFValue() - best->getGValue(),
                            retSucc_ps, true);
                    if (!retSucc) {
                        best->distribution = DiscreteDistribution(100,
                                best->getFValue(),
                                best->getFHatValue(),
                                best->getDValue(),
                                best->getFHatValue() - best->getFValue());
						best->lackOfHValueData = true;
                    }
                } else {
                    best->distribution = DiscreteDistribution(100,
                            best->getGValue(),
                            best->getDValue(),
                            domain.getBranchingFactor());
                }

                tla.kBestNodes.push_back(best);
                i++;
            }

            // Now that k-best are selected, perform Cserna backup
            csernaBackup(tla);
        }
    }
};
