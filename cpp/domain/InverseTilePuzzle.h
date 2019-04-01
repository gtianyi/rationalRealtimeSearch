#pragma once
#include "SlidingTilePuzzle.h"
#include <math.h>
#include <algorithm>
#include <random>

class InverseTilePuzzle : public SlidingTilePuzzle {
    using pNode = DiscreteDistribution::ProbabilityNode;
    using HDistribuitonMap = unordered_map<int, vector<pNode>>;
    using HData = unordered_map<double, vector<double>>;

private:
    template <class T>
    vector<T> getSampleByPercent(double percent, vector<T> in) const {
        vector<T> out;

        int n = (int)((double)in.size() * percent);
        n = max(n, 1);

        //cout << "numberofSample " << n << endl;

        while (n > 0) {
            std::random_device rd;
            std::mt19937 eng(rd());
            std::uniform_int_distribution<> distr(0, in.size() - 1);

            int randPos = distr(eng);

            out.push_back(in[randPos]);
            in.erase(std::next(in.begin(), randPos));

            n--;
        }
        return out;
    }

    vector<double> getApproximateSortedHsList(double& h,
            HData& hvshsData) const {
        double floorh = floor(h);
        double ceilh = ceil(h);
        vector<double>& floorHs = hvshsData[floorh];
        vector<double>& ceilHs = hvshsData[ceilh];

        if (floorh == ceilh)
            return floorHs;

        vector<double> sampleFromFloor =
				getSampleByPercent<double>(h - floorh, floorHs);
 
        vector<double> sampleFromCeil =
                getSampleByPercent<double>(ceilh-h, ceilHs);

        vector<double> ret = sampleFromFloor;
        ret.insert(ret.end(), sampleFromCeil.begin(), sampleFromCeil.end());
        sort(ret.begin(), ret.end());

		return ret;
   }

    vector<pNode> getDistributionBySortedhsList(
            vector<double>& sortedhsList) const {
        vector<pNode> ret;

        double prevhs = -1.0;
        int hsCounter = 1;

        for (const auto& hs : sortedhsList) {
            if (hs == prevhs) {
                hsCounter++;
				continue;
            }

            DiscreteDistribution::ProbabilityNode pn(
                    hs, (double) hsCounter / (double) sortedhsList.size());
            ret.push_back(pn);

            prevhs = hs;
            hsCounter = 1;
        }

		return ret;
    }

    void approximateHtableByData(HData& hvshsData,
            HDistribuitonMap& hValueTable,
            double& maxH) const {
        int resolution = 10;
        double oneBucket = 1.0;
        double step = oneBucket / resolution;

        for (double h = 0; h <= maxH; h += step) {
            if (h < 3.0) {
                pNode pn(0, 1);
                hValueTable[h].push_back(pn);
                continue;
            }

            vector<double> sampledSortedHsList;

            if (h < 4.0) {
				sampledSortedHsList	 = hvshsData[4.0];
            }
			else{
                sampledSortedHsList = getApproximateSortedHsList(h, hvshsData);
            }

            vector<pNode> distribution =
                    getDistributionBySortedhsList(sampledSortedHsList);
            hValueTable.insert({(int)(h * 10), distribution});
        }
   }

public:
    using SlidingTilePuzzle::SlidingTilePuzzle;

    Cost getEdgeCost(State state) { return 1.0 / (double)state.getFace(); }

    Cost heuristic(const State& state) {
        // Check if the heuristic of this state has been updated
        if (correctedH.find(state) != correctedH.end()) {
            return correctedH[state];
        }

        Cost h = manhattanDistanceWithInverseFaceCost(state);

        updateHeuristic(state, h);

        return correctedH[state];
    }

    Cost manhattanDistanceWithInverseFaceCost(const State& state) const {
        Cost manhattanSum = 0;

        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                auto value = state.getBoard()[r][c];
                if (value == 0) {
                    continue;
                }

                manhattanSum += (1.0 / (double)value) *
                        (abs(value / size - r) + abs(value % size - c));
                // cout << "value " << value << " sum " << manhattanSum << endl;
            }
        }

        return manhattanSum;
    }

    void readDistributionData(ifstream& f,
            HDistribuitonMap& hValueTable) const {
        //cout << "reading inverse tile data\n";
        string line;

        double h, valueCount, hs, hsCount, maxH;

        HData hData;

        while (getline(f, line)) {
            stringstream ss(line);

            ss >> h;
			maxH = h;
            ss >> valueCount;

            if (hData.find(h)!=hData.end()) {
				cout << "error: duplicate h from data " << h << "\n";
            }

            vector<double> hsList;
            while (!ss.eof()) {
                ss >> hs;
                ss >> hsCount;

                while (hsCount > 0) {
                    hsCount--;
					hsList.push_back(hs);
                }
            }
            hData[h] = hsList;
        }

		f.close();

		approximateHtableByData(hData, hValueTable, maxH);

		//cout << "total h " << hValueTable.size() << "\n";
    }

    virtual string getSubDomainName() const { return "inverse"; }

    virtual double getHAdjustCoefficientForDataDriven() const { return 15.0; }
};
