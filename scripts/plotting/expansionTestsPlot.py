'''
plotting code for generate nancydd related plots

Author: Tianyi Gu
Date: 08/15/2019
'''

#!/usr/bin/env python

__author__ = 'tianyigu'

import json
from os import listdir

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns


def makeDifferencePlot(width, height, xAxis, yAxis, dataframe, dodge, hue,
                       orderList, hueOrderList, xLabel, yLabel, outputName,
                       markerList):
    sns.set(rc={
        'figure.figsize': (width, height),
        'font.size': 26,
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


def main():

    markers = [
        "o", "v", "s", "<", "p", "h", "^", "D", "X", ">", "o", "v", "s", "<",
        "p", "h", "^", "D", "X", ">"
    ]

    # Hard coded result directories
    tileDimension = "4x4"
    tileType = "uniform"
    limits = [3, 10, 30, 100, 300, 1000]
    algorithms = {
        "astar": "A*",
        "fhat": "F-Hat",
        "bfs": "BFS",
        "risk": "Risk",
        "riskdd": "RiskDD",
        "lsslrtastar": "LSS-LRTA*"
    }

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
            with open(
                    "../../results/SlidingTilePuzzle/expansionTests/NancyDD/" +
                    tileType + '/' + alg + '/' + tileDimension + "/" +
                    jsonFile) as json_data:
                resultData = json.load(json_data)

                instance.append(str(jsonFile))
                lookAheadVals.append(resultData["Lookahead"])
                algorithm.append(algorithms[alg])
                solutionCost.append(resultData[algorithms[alg]])
                # differenceCost.append(resultData[algo.replace("A*", "A*")] - resultData["A*"])

    df = pd.DataFrame({
        "instance": instance,
        "Node Expansion Limit": lookAheadVals,
        "Solution Cost": solutionCost,
        "Algorithm": algorithm
    })

    for rowdata in df.iterrows():
        row = rowdata[1]
        relateastar = df[(df["instance"] == row['instance'])
                         & (df["Algorithm"] == 'A*')]
        diffCost = row['Solution Cost'] - relateastar['Solution Cost']
        diffCost = diffCost.values[0]
        differenceCost.append(diffCost)

    df["Algorithm Cost - A* Cost"] = differenceCost

    print("building plots...")

    makeDifferencePlot(
    13, 10, "Node Expansion Limit", "Algorithm Cost - A* Cost", df,
    0.35, "Algorithm", limits, algorithms.values(), "Node Expansion Limit",
    "Algorithm Cost - A* Cost", "../../plots/" + tileType + '/' +
    "CostDD" + ".pdf", markers)


if __name__ == '__main__':
    main()
