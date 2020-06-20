#include <iostream>
#include <string>
#include <fstream>
#include "TrialBasedRealTimeSearch.h"
#include "../domain/TreeWorld.h"
#include "../domain/SlidingTilePuzzle.h"
#include "../domain/SlidingTilePuzzle_sai.h"
#include "../domain/HeavyTilePuzzle_sai.h"
#include "../domain/InverseTilePuzzle_sai.h"
#include "../domain/PancakePuzzle.h"
#include "../domain/RaceTrack_thts.h"
#include "../domain/TreeWorld.h"
#include "../domain/SqrtTilePuzzle_sai.h"
#include "../utility/cxxopts/include/cxxopts.hpp"

using namespace std;


int main(int argc, char** argv) {

    cxxopts::Options options("./thrtts",
            "This is a trial based realtime search program");

    options.add_options()

        ("d,domain", "domain type: treeWorld, slidingTile, pancake, racetrack", 
		 cxxopts::value<std::string>()->default_value("slidingTile"))

		("s,subdomain", "puzzle type: uniform, inverse, heavy, sqrt; "
		                "pancake type: regular, heavy;"
                        "racetrack map : barto-big, barto-bigger, hanse-bigger-double, uniform-small", 
		 cxxopts::value<std::string>()->default_value("uniform"))

        ("a,alg", "thts algorithm: WAS, GUCTS", 
		 cxxopts::value<std::string>()->default_value("WAS"))

        ("l,lookahead", "expansion limit", 
		 cxxopts::value<int>()->default_value("100"))

		("o,performenceOut", "performence Out file", 
		 cxxopts::value<std::string>()->default_value("out.txt"))

		("v,pathOut", "path Out file", cxxopts::value<std::string>())

        ("h,help", "Print usage")
    ;

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
	}

    auto domain = args["domain"].as<std::string>();
    auto subDomain = args["subdomain"].as<std::string>();
    auto lookahead = args["lookahead"].as<int>();
    auto algorithmInput = args["alg"].as<std::string>();
    auto outPerfromence = args["performenceOut"].as<string>();

    string prune_type = "erase";
    bool greedyOneStep = false;
    bool hLearning = false;
    bool check = true; // Check validate solution

    string algorithm = "";
    bool front = true;


    for(int i = 0; i < algorithmInput.length(); ++i){
        if (algorithmInput[i] == '-'){
            front = !front;
        } else {
            if (front){
                algorithm += algorithmInput[i];
            } else {
                if (algorithmInput[i] == 'L'){
                    hLearning = true;
                } else if (algorithmInput[i] == 'H'){
                    greedyOneStep = true;
                } 
            }
        }
    }

    ResultContainer res;
    if (domain == "pancake"){
        ResultContainer result;
        PancakePuzzle world = PancakePuzzle(cin);
        if (subDomain == "heavy"){
			world.setVariant(1);
		}
        THTS_RT <PancakePuzzle> thts(world, algorithm, lookahead, greedyOneStep, hLearning);

        thts.setRecordPlan(check);

        res = thts.getPlan();
        if (check && !world.validatePath(res.path)) {
            cout << "INVALID PATH!" << endl;
            exit(1);
        }
    } else if (domain == "treeWorld"){
        TreeWorld world = TreeWorld(cin);
        THTS_RT <TreeWorld> thts(world, algorithm, lookahead, greedyOneStep, hLearning);

        thts.setRecordPlan(check);

        res = thts.getPlan();
    }  else if (domain == "racetrack"){
        string mapFile = "/home/aifs1/gu/phd/research/workingPaper/"
                         "realtime-nancy/worlds/racetrack/map/" +
                subDomain + ".track";

        ifstream map(mapFile);

        if (!map.good()) {
            cout << "map file not exist: " << mapFile << endl;
            exit(1);
            }

        RaceTrack world = RaceTrack(map, cin);

        THTS_RT <RaceTrack> thts(world, algorithm, lookahead, greedyOneStep, hLearning);

        thts.setRecordPlan(check);

        res = thts.getPlan();
    } else if (domain == "slidingTile") {
        if (subDomain == "uniform") {
            SlidingTilePuzzle world = SlidingTilePuzzle(cin);
            THTS_RT<SlidingTilePuzzle> thts(
                    world, algorithm, lookahead, greedyOneStep, hLearning);

            thts.setRecordPlan(check);

            res = thts.getPlan();
            if (check && !world.validatePath(res.path)) {
                cout << "INVALID PATH!" << endl;
                exit(1);
            }

        } else if (subDomain == "heavy") {
            HeavyTilePuzzle world = HeavyTilePuzzle(cin);
            THTS_RT<HeavyTilePuzzle> thts(
                    world, algorithm, lookahead, greedyOneStep, hLearning);

            thts.setRecordPlan(check);

            res = thts.getPlan();
            if (check && !world.validatePath(res.path)) {
                cout << "INVALID PATH!" << endl;
                exit(1);
            }
        } else if (subDomain == "inverse") {
            InverseTilePuzzle world = InverseTilePuzzle(cin);
            THTS_RT<InverseTilePuzzle> thts(
                    world, algorithm, lookahead, greedyOneStep, hLearning);

            thts.setRecordPlan(check);

            res = thts.getPlan();
            if (check && !world.validatePath(res.path)) {
                cout << "INVALID PATH!" << endl;
                exit(1);
            }
        } else if (subDomain == "sqrt") {
            SqrtTilePuzzle world = SqrtTilePuzzle(cin);
            THTS_RT<SqrtTilePuzzle> thts(
                    world, algorithm, lookahead, greedyOneStep, hLearning);

            thts.setRecordPlan(check);

            res = thts.getPlan();
            if (check && !world.validatePath(res.path)) {
                cout << "INVALID PATH!" << endl;
                exit(1);
            }
        }
    }

    //string result = algorithmInput + "," + domain + "," + to_string(res.solutionCost) + "," + to_string(res.nodesGenerated) + "," + to_string(res.nodesExpanded) + "," + to_string(lookahead);
    string result="{\"thts-"+algorithmInput+"\": "+ to_string(res.solutionCost)+", \"Lookahead\": " + to_string(lookahead) + " }";

    ofstream out(outPerfromence);
    
    out << result;
    out.close();

	// dumpout solution path
    if (args.count("pathOut")) {
        ofstream out(args["pathOut"].as<std::string>());
        while (!res.path.empty()) {
            out << res.path.front();
            res.path.pop();
        }
        out.close();
    }
}
