#include <iostream>
#include <string>
#include <fstream>
#include "RealTimeSearch.h"
#include "utility/DiscreteDistribution.h"
#include "decisionAlgorithms/KBestBackup.h"
#include "expansionAlgorithms/AStar.h"
#include "expansionAlgorithms/BreadthFirst.h"
#include "expansionAlgorithms/DepthFirst.h"
#include "expansionAlgorithms/Risk.h"
#include "expansionAlgorithms/RiskDD.h"
#include "learningAlgorithms/Ignorance.h"
#include "learningAlgorithms/Dijkstra.h"
#include "domain/TreeWorld.h"
#include "domain/HeavyTilePuzzle.h"
#include "domain/InverseTilePuzzle.h"
#include <memory>

using namespace std;

void startAlg(SlidingTilePuzzle& domain, string expansionModule, string learningModule, string decisionModule,
        double lookahead, string algName, string& result, double k = 1, string beliefType = "normal") {
    shared_ptr<RealTimeSearch<SlidingTilePuzzle>> searchAlg =
            make_shared<RealTimeSearch<SlidingTilePuzzle>>(domain,
                    expansionModule,
                    learningModule,
                    decisionModule,
                    lookahead,
                    k,
                    beliefType);

    ResultContainer res = searchAlg->search(1000*200/lookahead);

    if (res.solutionFound && !domain.validatePath(res.path)) {
        cout << "Invalid path detected from search: " << expansionModule
             << endl;
        exit(1);
    }

    result += "\""+algName+"\": " + to_string(res.solutionCost) + ", ";
}

int main(int argc, char** argv)
{
    if (argc != 6) {
        cout << "Wrong number of arguments: ./expansionTests.sh <Domain Type> <expansion limit> <sub domain type> <algorithm> <output file> < <domain file>"
             << endl;
        cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
        cout << "tree domains are na" << endl;
        cout << "Puzzle sub-domains are uniform, heavy, inverse, sroot" << endl;
        cout << "Available algorithm are bfs, astar, fhat, lsslrtastar, risk, riskdd" << endl;
        exit(1);
    }

    vector<string> bfsConfig{"bfs", "learn", "k-best", "BFS", "normal"};
    vector<string> astarConfig{"a-star", "learn", "k-best", "A*", "normal"};
    vector<string> fhatConfig{"f-hat", "learn", "k-best", "F-Hat", "normal"};
    vector<string> lsslrtastarConfig{
            "a-star", "learn", "minimin", "LSS-LRTA*", "normal"};
    vector<string> riskConfig{"risk", "learn", "k-best", "Risk", "normal"};
    vector<string> riskddConfig{
            "riskDD", "learnDD", "nancyDD", "RiskDD", "data"};
    unordered_map<string, vector<string>> algorithmsConfig({{"bfs", bfsConfig},
            {"astar", astarConfig},
            {"fhat", fhatConfig},
            {"lsslrtastar", lsslrtastarConfig},
            {"risk", riskConfig},
            {"riskdd", riskddConfig}});

    if (algorithmsConfig.find(argv[4]) == algorithmsConfig.end()) {
        cout << "Available algorithm are bfs, astar, fhat, lsslrtastar, "
                "risk, riskdd"
             << endl;
        exit(1);
    }

    // Get the lookahead depth
    int lookaheadDepth = stoi(argv[2]);

    // Get the domain type
    string domain = argv[1];

    // Get sub-domain type
    string subDomain = argv[3];

    string result = "{ ";

    if (domain == "SlidingPuzzle") {
        // Make a tile puzzle
        std::shared_ptr<SlidingTilePuzzle> world;

        if (subDomain == "uniform") {
            world = std::make_shared<SlidingTilePuzzle>(cin);
            DiscreteDistribution::readData<SlidingTilePuzzle>(*world);
        } else if (subDomain == "heavy") {
            world = std::make_shared<HeavyTilePuzzle>(cin);
			DiscreteDistribution::readData<SlidingTilePuzzle>(*world);
        } else if (subDomain == "inverse") {
            world = std::make_shared<InverseTilePuzzle>(cin);
			DiscreteDistribution::readData<SlidingTilePuzzle>(*world);
        }

        startAlg(*world,
                algorithmsConfig[argv[4]][0],
                algorithmsConfig[argv[4]][1],
                algorithmsConfig[argv[4]][2],
                lookaheadDepth,
                algorithmsConfig[argv[4]][3],
                result,
                1,
                algorithmsConfig[argv[4]][4]);

    } else {
        cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
        exit(1);
    }

    result += "\"Lookahead\": " + to_string(lookaheadDepth) + " }";

    ofstream out(argv[5]);

    out << result;
    out.close();
}
