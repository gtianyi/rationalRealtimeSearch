#pragma once
#include "SlidingTilePuzzle.h"

class HeavyTilePuzzle : public SlidingTilePuzzle {
public:
    using SlidingTilePuzzle::SlidingTilePuzzle;

    Cost getEdgeCost(State state) { return state.getFace(); }

	Cost heuristic(const State& state) {
        // Check if the heuristic of this state has been updated
        if (correctedH.find(state) != correctedH.end()) {
            return correctedH[state];
        }

        Cost h = manhattanDistanceWithFaceCost(state);

        updateHeuristic(state, h);

        return correctedH[state];
    }

    Cost manhattanDistanceWithFaceCost(const State& state) const {
        Cost manhattanSum = 0;

        for (int r = 0; r < size; r++) {
            for (int c = 0; c < size; c++) {
                auto value = state.getBoard()[r][c];
                if (value == 0) {
                    continue;
                }

                manhattanSum +=
                        value * (abs(value / size - r) + abs(value % size - c));
				//cout << "value " << value << " sum " << manhattanSum << endl;
            }
        }

        return manhattanSum;
    }

    virtual string getSubDomainName() const { return "heavy"; }

    Cost distance(const State& state) {
        // Check if the distance of this state has been updated
        if (correctedD.find(state) != correctedD.end()) {
            return correctedD[state];
        }

        Cost d = manhattanDistanceWithFaceCost(state);

        updateDistance(state, d);

        return correctedD[state];
    }

    virtual DiscreteDistributionDD hstart_distribution(const State& state) {
        // Check if the heuristic h-hat of this state has been updated
        if (correctedDistribution.find(state) != correctedDistribution.end()) {
            return correctedDistribution[state];
        }

        Cost h = manhattanDistanceWithFaceCost(state);

        correctedDistribution[state] = DiscreteDistributionDD(h);
        correctedPostSearchDistribution[state] = DiscreteDistributionDD(h,true);

        updateHeuristic(state, h);

        return correctedDistribution[state];
    }

    virtual DiscreteDistributionDD hstart_distribution_ps(const State& state) {
        // Check if the heuristic h-hat of this state has been updated
        if (correctedDistribution.find(state) != correctedDistribution.end()) {
            return correctedDistribution[state];
        }

        Cost h = manhattanDistanceWithFaceCost(state);

        correctedDistribution[state] = DiscreteDistributionDD(h);
        correctedPostSearchDistribution[state] = DiscreteDistributionDD(h,true);

        return correctedPostSearchDistribution[state];
    }
};
