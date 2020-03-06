'''
code for generate average global epsilons

Author: Tianyi Gu
Date: 03/04/2020
'''

#!/usr/bin/env python

__author__ = 'tianyigu'

import json
import math
import sys
# from collections import OrderedDict
# from datetime import datetime
from os import listdir

# import matplotlib.pyplot as plt
# import numpy as np
# import pandas as pd
# import seaborn as sns

M_PI = 3.14159265358979323846
BIN_NUM = 100


def printUsage():
    print "usage: python expansionTestsPlot.py"


def prbabilityDensityFunction(x, mu, var):
    return ((1 / math.sqrt(2 * M_PI * var))) * math.exp(-(pow(x - mu, 2) /
                                                          (2 * var)))


def main():
    if len(sys.argv) != 1:
        printUsage()
        return

    print("reading in data...")

    epsilonH = []
    epsilonD = []

    for jsonFile in listdir(
            "../../results/SlidingTilePuzzle/expansionTests/NancyDD/uniform/risk-epsilons/4x4"
    ):
        if jsonFile[-5:] != ".json":
            continue
        with open(
                "../../results/SlidingTilePuzzle/expansionTests/NancyDD/uniform/risk-epsilons/4x4/"
                + jsonFile) as json_data:

            resultData = json.load(json_data)
            epsilonH.append(resultData["epsilonHGlobal"])
            epsilonD.append(resultData["epsilonDGlobal"])

    print("construct assumption distribution...")

    # nowstr = datetime.now().strftime("%d%m%Y-%H%M")

    epsD = sum(epsilonD) / len(epsilonD)
    epsH = sum(epsilonH) / len(epsilonH)

    print("epsD:", epsD)
    print("epsH:", epsH)

    #create json of made up data distribution
    #create made up bin for each distribution

    data = {
        "0": {
            "number of bins": 1,
            "bins": [{
                "h*": 1,
                "prob": "1.000000"
            }]
        }
    }
    for h in xrange(1, 100):

        #compute h-hat
        hhat = h + h / (1 - epsD) * epsH
        error = h / (1 - epsD) * epsH
        stdDev = error / 2.0
        var = math.pow(stdDev, 2)

        #create a discrete distribution from a gaussian
        lower = h
        upper = hhat + 3 * stdDev

        step = (upper - lower) / BIN_NUM

        currentBin = lower
        probSum = 0.0

        hist = {"number of bins": 100}
        bins = []
        for b in xrange(BIN_NUM):
            prob = prbabilityDensityFunction(currentBin, hhat, var)
            bins.append({"h*": currentBin, "prob": prob})
            probSum += prob
            currentBin += step

        #normalize the distribution probabilites
        for b in bins:
            b["prob"] = '%.6f' % (b["prob"] / probSum)

        hist["bins"] = bins

        data[str(h)] = hist

    with open('data001.json', 'w') as outfile:
        json.dump(data, outfile)


if __name__ == '__main__':
    main()
