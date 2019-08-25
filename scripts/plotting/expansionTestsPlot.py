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
import numpy as np


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


def makeCoverageTable(dataframe, tileType):
    grp = dataframe.groupby(['Node Expansion Limit',
                             'Algorithm'])['Solution Cost'].count()
    print(grp)
    grp.to_csv("../../plots/" + tileType + "/coverage-" + tileType + ".csv")


def main():

    markers = [
        "o", "v", "s", "<", "p", "h", "^", "D", "X", ">", "o", "v", "s", "<",
        "p", "h", "^", "D", "X", ">"
    ]

    # Hard coded result directories
    tileDimension = "4x4"
    tileType = "uniform"
    # tileType = "inverse"
    limits = [3, 10, 30, 100, 300, 1000]
    # limits = [3, 10, 30, 100, 300]
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

                if float(resultData[algorithms[alg]]) != -1.0:
                    instance.append(str(jsonFile))
                    lookAheadVals.append(resultData["Lookahead"])
                    algorithm.append(algorithms[alg])
                    solutionCost.append(resultData[algorithms[alg]])

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
        if relateastar.empty:
            differenceCost.append(np.nan)
        else:
            diffCost = row['Solution Cost'] - relateastar['Solution Cost']
            diffCost = diffCost.values[0]
            differenceCost.append(diffCost)

    df["Algorithm Cost - A* Cost"] = differenceCost

    # print df

    print("building plots...")
    # makeCoverageTable(df, tileType)
    makeDifferencePlot(13, 10, "Node Expansion Limit",
                       "Algorithm Cost - A* Cost", df, 0.35, "Algorithm",
                       limits, algorithms.values(), "Node Expansion Limit",
                       "Algorithm Cost - A* Cost",
                       "../../plots/" + tileType + '/' + "CostDD" + ".pdf",
                       markers)


# makeDifferencePlot(13, 10, "Node Expansion Limit",
# "Solution Cost", df, 0.35, "Algorithm", limits,
# algorithms.values(), "Node Expansion Limit",
# "Solution Cost",
# "../../plots/" + tileType + '/' + "CostDD" + ".pdf",
# markers)

if __name__ == '__main__':
    main()
