#include <iostream>
#include <string>
#include <fstream>
#include "RealTimeSearch.h"
#include "utility/DiscreteDistributionDD.h"
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
#include "domain/PancakePuzzle.h"
#include <memory>

using namespace std;

void getCpuStatistic(vector<double>& lookaheadCpuTime,
        vector<double>& percentiles,
        double& mean) {
    sort(lookaheadCpuTime.begin(), lookaheadCpuTime.end());

    mean = accumulate(lookaheadCpuTime.begin(), lookaheadCpuTime.end(), 0.0) /
            lookaheadCpuTime.size();

    for (int i = 1; i <= 100; i++) {
        int percentID =
                (int)(((double)i / 100) * (lookaheadCpuTime.size() - 1));
        percentiles.push_back(lookaheadCpuTime[percentID]);
    }
}

template<class Domain>
void startAlg(shared_ptr<Domain> domain_ptr,
        string expansionModule,
        string learningModule,
        string decisionModule,
        double lookahead,
        string algName,
        string& result,
        double k = 1,
        string beliefType = "normal") {
    shared_ptr<RealTimeSearch<Domain>> searchAlg =
            make_shared<RealTimeSearch<Domain>>(*domain_ptr,
                    expansionModule,
                    learningModule,
                    decisionModule,
                    lookahead,
                    k,
                    beliefType);

	if (algName == "RiskDD" || algName == "RiskDDSquish")
		DiscreteDistributionDD::readData<Domain>(domain_ptr);

    ResultContainer res = searchAlg->search(1000*200/lookahead);

    /*if (res.solutionFound && !domain.validatePath(res.path)) {*/
    // cout << "Invalid path detected from search: " << expansionModule
    //<< endl;
    // exit(1);
    /*}*/

    result += "\"" + algName + "\": " + to_string(res.solutionCost) +
            ", \"epsilonHGlobal\": " + to_string(res.epsilonHGlobal) +
            ", \"epsilonDGlobal\": " + to_string(res.epsilonDGlobal) +
            ", \"cpu-percentiles\": [";

    vector<double> cpuPercentiles;
    double cpuMean;

    getCpuStatistic(res.lookaheadCpuTime, cpuPercentiles, cpuMean);

    for (auto& t : cpuPercentiles) {
        result += to_string(t) + ", ";
    }

    result.pop_back();
    result.pop_back();
    result += "], \"cpu-mean\": " + to_string(cpuMean) + ", ";
}

int main(int argc, char** argv)
{
    if (argc != 6) {
        cout << "Wrong number of arguments: ./expansionTests.sh <Domain Type> <expansion limit> <sub domain type> <algorithm> <output file> < <domain file>"
             << endl;
        cout << "Available domains are TreeWorld, SlidingPuzzle, pancake" << endl;
        cout << "tree subdomains are na" << endl;
        cout << "pancake subdomains are regular, heavy" << endl;
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
    vector<string> riskPersistConfig{"risk", "learn", "k-best-persist", "PRisk", "normal"};
    vector<string> riskddConfig{
            "riskDD", "learnDD", "nancyDD", "RiskDD", "data"};
vector<string> riskddSquishConfig{
            "riskDDSquish", "learnDD", "nancyDD", "RiskDDSquish", "data"};
    unordered_map<string, vector<string>> algorithmsConfig({{"bfs", bfsConfig},
            {"astar", astarConfig},
            {"fhat", fhatConfig},
            {"lsslrtastar", lsslrtastarConfig},
            {"risk", riskConfig},
            {"prisk", riskPersistConfig},
            {"riskddSquish", riskddSquishConfig},
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

        std::shared_ptr<SlidingTilePuzzle> world;

        if (subDomain == "uniform") {
            world = std::make_shared<SlidingTilePuzzle>(cin);
        } else if (subDomain == "heavy") {
            world = std::make_shared<HeavyTilePuzzle>(cin);
        } else if (subDomain == "inverse") {
            world = std::make_shared<InverseTilePuzzle>(cin);
        }

        startAlg<SlidingTilePuzzle>(world,
                algorithmsConfig[argv[4]][0],
                algorithmsConfig[argv[4]][1],
                algorithmsConfig[argv[4]][2],
                lookaheadDepth,
                algorithmsConfig[argv[4]][3],
                result,
                1,
                algorithmsConfig[argv[4]][4]);

    } else if (domain == "TreeWorld") {

        std::shared_ptr<TreeWorld> world = std::make_shared<TreeWorld>(cin);

        startAlg<TreeWorld>(world,
                algorithmsConfig[argv[4]][0],
                algorithmsConfig[argv[4]][1],
                algorithmsConfig[argv[4]][2],
                lookaheadDepth,
                algorithmsConfig[argv[4]][3],
                result,
                1,
                algorithmsConfig[argv[4]][4]);

	} else if (domain == "pancake") {

		std::shared_ptr<PancakePuzzle> world = std::make_shared<PancakePuzzle>(cin);

		startAlg<PancakePuzzle>(world,
				algorithmsConfig[argv[4]][0],
				algorithmsConfig[argv[4]][1],
				algorithmsConfig[argv[4]][2],
				lookaheadDepth,
				algorithmsConfig[argv[4]][3],
				result,
				1,
				algorithmsConfig[argv[4]][4]);
    }else {
        cout << "Available domains are TreeWorld, SlidingPuzzle, pancake" << endl;
        exit(1);
    }

    result += "\"Lookahead\": " + to_string(lookaheadDepth) + " }";

    ofstream out(argv[5]);

    out << result;
    out.close();
}
