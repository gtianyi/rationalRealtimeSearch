'''
plotting code for generate nancydd related plots

Author: Tianyi Gu
Date: 08/15/2019
'''

#!/usr/bin/env python

__author__ = 'tianyigu'

import json
from os import listdir

import sys
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


def printUsage():
    print "usage: python expansionTestsPlot.py <plot type> <tile type>"
    print "plot type: coverage, pairwise, solutioncost"
    print "tile type: uniform heavy inverse"


def makeDifferencePlot(width, height, xAxis, yAxis, dataframe, dodge, hue,
                       orderList, hueOrderList, xLabel, yLabel, outputName,
                       markerList):
    sns.set(rc={
        'figure.figsize': (width, height),
        'font.size': 27,
        'text.color': 'black'
    })
    ax = sns.pointplot(x=xAxis,
                       y=yAxis,
                       hue=hue,
                       order=orderList,
                       hue_order=hueOrderList,
                       data=dataframe,
                       ci=95,
                       errwidth=3,
                       join=False,
                       dodge=dodge,
                       palette="Set2",
                       markers=markerList)
    ax.tick_params(colors='black', labelsize=12)
    plt.ylabel(yLabel, color='black', fontsize=18)
    plt.xlabel(xLabel, color='black', fontsize=18)
    plt.savefig(outputName, bbox_inches="tight", pad_inches=0)
    plt.close()
    plt.clf()
    plt.cla()
    return


def makeCoverageTable(dataframe, tileType):
    grp = dataframe.groupby(['Node Expansion Limit',
                             'Algorithm'])['Solution Cost'].count()
    print(grp)
    grp.to_csv("../../plots/" + tileType + "/coverage-" + tileType + ".csv")


def main():
    if len(sys.argv) != 3:
        printUsage()
        return

    markers = [
        "o", "v", "s", "<", "p", "h", "^", "D", "X", ">", "o", "v", "s", "<",
        "p", "h", "^", "D", "X", ">"
    ]

    # Hard coded result directories
    tileDimension = "4x4"
    tileType = sys.argv[2]
    # limits = [3, 10, 30, 100, 300, 1000]
    # limits = [10, 30, 100, 300, 1000]
    limits = [30, 100, 300, 1000]
    # limits = [100, 300, 1000]
    algorithms_data = {
        # "astar": "A*",
        # "fhat": "F-Hat",
        # "bfs": "BFS",
        "risk": "Risk",
        "riskdd": "RiskDD",
        "prisk": "PRisk",
        "riskddSquish": "RiskDDSquish",
        "lsslrtastar": "LSS-LRTA*"
    }

    algorithms = {
        # "astar": "A*",
        # "fhat": "F-Hat",
        # "bfs": "BFS",
        "risk": "Nancy",
        "riskdd": "Nancy (DD PE)",
        "prisk": "Nancy (pers.)",
        "riskddSquish": "Nancy (DD)",
        "lsslrtastar": "LSS-LRTA*"
    }

    baseline = "LSS-LRTA*"

    instance = []
    lookAheadVals = []
    algorithm = []
    solutionCost = []
    differenceCost = []

    print("reading in data...")

    for alg in algorithms:
        for jsonFile in listdir(
                "../../results/SlidingTilePuzzle/expansionTests/NancyDD/" +
                tileType + '/' + alg + '/' + tileDimension):
            if jsonFile[-5:] != ".json":
                continue
            with open(
                    "../../results/SlidingTilePuzzle/expansionTests/NancyDD/" +
                    tileType + '/' + alg + '/' + tileDimension + "/" +
                    jsonFile) as json_data:

                # print "../../results/SlidingTilePuzzle/expansionTests/NancyDD/", tileType, '/', alg + '/', tileDimension, "/", jsonFile

                resultData = json.load(json_data)
                if float(resultData[algorithms_data[alg]]) != -1.0:
                    instance.append(str(jsonFile))
                    lookAheadVals.append(resultData["Lookahead"])
                    algorithm.append(algorithms[alg])
                    solutionCost.append(resultData[algorithms_data[alg]])

    rawdf = pd.DataFrame({
        "instance": instance,
        "Node Expansion Limit": lookAheadVals,
        "Solution Cost": solutionCost,
        "Algorithm": algorithm
    })

    df = pd.DataFrame()
    df["instance"] = np.nan
    df["Node Expansion Limit"] = np.nan
    df["Solution Cost"] = np.nan
    df["Algorithm"] = np.nan

    for instance in rawdf["instance"].unique():
        dfins = rawdf[rawdf["instance"] == instance]
        if len(dfins) == len(algorithms):
            df = df.append(dfins)

    for rowdata in df.iterrows():
        row = rowdata[1]
        relateastar = df[(df["instance"] == row['instance'])
                         & (df["Algorithm"] == baseline)]
        if relateastar.empty:
            differenceCost.append(np.nan)
        else:
            diffCost = row['Solution Cost'] - relateastar['Solution Cost']
            diffCost = diffCost.values[0]
            differenceCost.append(diffCost)

    df["Algorithm Cost - " + baseline + " Cost"] = differenceCost

    # print df

    print("building plots...")

    if sys.argv[1] == "coverage":
        makeCoverageTable(rawdf, tileType)

    elif sys.argv[1] == "pairwise":
        makeDifferencePlot(
            13, 10, "Node Expansion Limit",
            "Algorithm Cost - " + baseline + " Cost", df, 0.35, "Algorithm",
            limits, algorithms.values(), "Node Expansion Limit",
            "Algorithm Cost - " + baseline + " Cost",
            "../../plots/" + tileType + '/' + tileType+"-tile-pairwise" + ".eps",
            markers)

    elif sys.argv[1] == "solutioncost":
        makeDifferencePlot(
            13, 10, "Node Expansion Limit", "Solution Cost", df, 0.35,
            "Algorithm", limits, algorithms.values(), "Node Expansion Limit",
            "Solution Cost",
            "../../plots/" + tileType + '/' + tileType+"-tile-solution-cost" + ".eps",
            markers)
    else:
        printUsage()


if __name__ == '__main__':
    main()
