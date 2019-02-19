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
#include <memory>

using namespace std;

int main(int argc, char** argv)
{
	if (argc != 5)
	{
		cout << "Wrong number of arguments: ./expansionTests.sh <Domain Type> <expansion limit> <sub domain type> <output file> < <domain file>" << endl;
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

	ResultContainer bfsRes;
	ResultContainer astarRes;
	ResultContainer fhatRes;
	ResultContainer riskRes;
	ResultContainer riskddRes;
    ResultContainer confidenceRes;
    ResultContainer lsslrtaRes;

    DiscreteDistribution::readData(subDomain);

    if (domain == "TreeWorld") {
        // Make a tree world
        TreeWorld world = TreeWorld(cin);

        RealTimeSearch<TreeWorld> bfs(
                world, "bfs", "none", "k-best", lookaheadDepth, 1, "normal");
        RealTimeSearch<TreeWorld> astar(
                world, "a-star", "none", "k-best", lookaheadDepth, 1, "normal");
        RealTimeSearch<TreeWorld> fhat(
                world, "f-hat", "none", "k-best", lookaheadDepth, 1, "normal");
        RealTimeSearch<TreeWorld> risk(
                world, "risk", "none", "k-best", lookaheadDepth, 1, "normal");
        RealTimeSearch<TreeWorld> confidence(world, "confidence", "learn", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<TreeWorld> lsslrta(world, "a-star", "none", "minimin", lookaheadDepth);

		astarRes = astar.search();
        if (!world.validatePath(astarRes.path))
        {
            cout << "Invalid path detected from f search!" << endl;
            exit(1);
        }

		riskRes = risk.search();
        if (!world.validatePath(riskRes.path))
        {
            cout << "Invalid path detected from risk search!" << endl;
            exit(1);
        }
        
        //confidenceRes = confidence.search();
		fhatRes = fhat.search();
        if (!world.validatePath(fhatRes.path))
        {
            cout << "Invalid path detected from f-hat search!" << endl;
            exit(1);
        }

		bfsRes = bfs.search();
        if (!world.validatePath(bfsRes.path))
        {
            cout << "Invalid path detected from BFS search!" << endl;
            exit(1);
        }
        
        lsslrtaRes = lsslrta.search();
        if (!world.validatePath(lsslrtaRes.path))
        {
            cout << "Invalid path detected from LSS-LRTA* search!" << endl;
            exit(1);
        }
	}
	else if (domain == "SlidingPuzzle")
	{
		// Make a tile puzzle
			std::shared_ptr<SlidingTilePuzzle> world;

			if(subDomain =="uniform"){
				world = std::make_shared<SlidingTilePuzzle>(cin);
			} else if (subDomain == "heavy"){
				world = std::make_shared<HeavyTilePuzzle>(cin);
			}
			

		RealTimeSearch<SlidingTilePuzzle> bfs(*world, "bfs", "learn", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<SlidingTilePuzzle> astar(*world, "a-star", "learn", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<SlidingTilePuzzle> fhat(*world, "f-hat", "learn", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<SlidingTilePuzzle> risk(*world, "risk", "learn", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<SlidingTilePuzzle> riskdd(*world, "risk", "learn", "k-best", lookaheadDepth, 1, "data");
        //RealTimeSearch<SlidingTilePuzzle> confidence(world, "confidence", "learn", "k-best", lookaheadDepth, 1, "normal");
		RealTimeSearch<SlidingTilePuzzle> lsslrta(*world, "a-star", "learn", "minimin", lookaheadDepth);

		astarRes = astar.search();
        if (!world->validatePath(astarRes.path))
        {
            cout << "Invalid path detected from f search!" << endl;
            exit(1);
        }

		riskRes = risk.search();
        if (!world->validatePath(riskRes.path))
        {
            cout << "Invalid path detected from risk search!" << endl;
            exit(1);
        }
		riskddRes = riskdd.search();
		if (!world->validatePath(riskddRes.path))
		{
			cout << "Invalid path detected from riskdd search!" << endl;
			exit(1);
		}
		
        //confidenceRes = confidence.search();
		fhatRes = fhat.search();
        if (!world->validatePath(fhatRes.path))
        {
            cout << "Invalid path detected from f-hat search!" << endl;
            exit(1);
        }

		bfsRes = bfs.search();
        if (!world->validatePath(bfsRes.path))
        {
            cout << "Invalid path detected from BFS search!" << endl;
            exit(1);
        }
        
        lsslrtaRes = lsslrta.search();
        if (!world->validatePath(lsslrtaRes.path))
        {
            cout << "Invalid path detected from LSS-LRTA* search!" << endl;
            exit(1);
        }
	}
	else
	{
		cout << "Available domains are TreeWorld and SlidingPuzzle" << endl;
		exit(1);
	}

	string result = "{ \"BFS\": " + to_string(bfsRes.solutionCost) + 
        ", \"A*\": " + to_string(astarRes.solutionCost) + 
        ", \"F-Hat\": " + to_string(fhatRes.solutionCost) +
		", \"Risk\": " + to_string(riskRes.solutionCost) + 
		", \"Riskdd\": " + to_string(riskddRes.solutionCost) + 
        ", \"Confidence\": " + to_string(confidenceRes.solutionCost) +
        ", \"LSS-LRTA*\": " + to_string(lsslrtaRes.solutionCost) +
		", \"Lookahead\": " + to_string(lookaheadDepth) + " }";

    ofstream out(argv[4]);

    out << result;
    out.close();

}
