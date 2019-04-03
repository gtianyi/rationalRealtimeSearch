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
#include "learningAlgorithms/Ignorance.h"
#include "learningAlgorithms/Dijkstra.h"
#include "domain/TreeWorld.h"
#include "domain/HeavyTilePuzzle.h"
#include "domain/InverseTilePuzzle.h"
#include <memory>

using namespace std;

template <class Domain>
shared_ptr<RealTimeSearch<Domain>> startAlg(Domain& domain, string expansionModule, string learningModule, string decisionModule,
        double lookahead, string algName, string& result, double k = 1, string beliefType = "normal") {
    shared_ptr<RealTimeSearch<Domain>> searchAlg =
            make_shared<RealTimeSearch<Domain>>(domain,
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

	return searchAlg;
}

int main(int argc, char** argv)
{
    if (argc != 5) {
        cout << "Wrong number of arguments: ./expansionTests.sh <Domain Type> <expansion limit> <sub domain type> <output file> < <domain file>"
             << endl;
        cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
        cout << "tree domains are na" << endl;
        cout << "Puzzle sub-domains are uniform, heavy, inverse, sroot" << endl;
        exit(1);
    }

    // Get the lookahead depth
    int lookaheadDepth = stoi(argv[2]);

    // Get the domain type
    string domain = argv[1];

    // Get sub-domain type
    string subDomain = argv[3];

	//this is used to get around the seg fault
	shared_ptr<RealTimeSearch<SlidingTilePuzzle>> alg1;
	shared_ptr<RealTimeSearch<SlidingTilePuzzle>> alg2;
	shared_ptr<RealTimeSearch<SlidingTilePuzzle>> alg3;
	shared_ptr<RealTimeSearch<SlidingTilePuzzle>> alg4;
	shared_ptr<RealTimeSearch<SlidingTilePuzzle>> alg5;
	shared_ptr<RealTimeSearch<SlidingTilePuzzle>> alg6;


    string result = "{ ";

    if (domain == "TreeWorld") {
        // Make a tree world
        TreeWorld world = TreeWorld(cin);

        startAlg(world, "bfs", "none", "k-best", lookaheadDepth, "BFS", result, 1, "normal");
        startAlg(world, "a-star", "none", "k-best", lookaheadDepth, "A*", result, 1, "normal");
        startAlg(world, "f-hat", "none", "k-best", lookaheadDepth, "F-Hat", result, 1, "normal");
        startAlg(world, "risk", "none", "k-best", lookaheadDepth, "Risk", result, 1, "normal");
        startAlg(world, "a-star", "none", "minimin", lookaheadDepth, "LSS-LRTA*", result);
    } else if (domain == "SlidingPuzzle") {
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

		//alg1 = startAlg(*world, "bfs", "learn", "k-best", lookaheadDepth, "BFS", result, 1, "normal");
		//alg2 =startAlg(*world, "a-star", "learn", "k-best", lookaheadDepth, "A*", result, 1, "normal");
		//alg3 =startAlg(*world, "f-hat", "learn", "k-best", lookaheadDepth, "F-Hat", result, 1, "normal");
		//alg4 =startAlg(*world, "risk", "learn", "k-best", lookaheadDepth, "Risk", result, 1, "normal");
		alg5 = startAlg(*world, "a-star", "learn", "k-best", lookaheadDepth, "RiskDD",result, 1, "data");
		//alg6 =startAlg(*world, "a-star", "learn", "minimin", lookaheadDepth, "LSS-LRTA*", result);

    } else {
        cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
        exit(1);
    }

    result += "\"Lookahead\": " + to_string(lookaheadDepth) + " }";

    ofstream out(argv[4]);

    out << result;
    out.close();
}
