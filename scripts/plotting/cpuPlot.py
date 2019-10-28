'''
plotting code for generate nancydd related plots

Author: Tianyi Gu
Date: 10/22/2019
'''

#!/usr/bin/env python

__author__ = 'tianyigu'

import json
import math
import sys
from datetime import datetime
from os import listdir

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


def printUsage():
    print "usage: python expansionTestsPlot.py <plot type> <tile type>"
    print "plot type: coverage, pairwise, solutioncost"
    print "tile type: uniform heavy inverse"


def makeCpuPlot(width, height, xAxis, yAxis, xerr, yerr, dataframe, dodge, hue,
                hueOrderList, xLabel, yLabel, outputName, markerList):
    sns.set(rc={
        'figure.figsize': (width, height),
        'font.size': 27,
        'text.color': 'black'
    })

    g = sns.FacetGrid(data=dataframe, hue=hue, height=height)
    g.map(plt.errorbar, xAxis, yAxis, xerr, yerr, fmt='o',
          elinewidth=3).set(xscale="log")
    g.add_legend()

    # ax.tick_params(colors='black', labelsize=12)
    plt.ylabel(yLabel, color='black')
    # plt.ylabel(yLabel, color='black', fontsize=18)
    # plt.xlabel(xLabel, color='black', fontsize=18)
    plt.xlabel(xLabel, color='black')
    # plt.savefig(outputName, bbox_inches="tight", pad_inches=0)
    plt.savefig(outputName)
    plt.close()
    plt.clf()
    plt.cla()
    return


def makeCoverageTable(dataframe, tileType):
    grp = dataframe.groupby(['Node Expansion Limit',
                             'Algorithm'])['Solution Cost'].count()
    print(grp)
    grp.to_csv("../../plots/" + tileType + "/coverage-" + tileType + ".csv")


def getCpuStatistics(rawdf):

    ret_sol_cost = rawdf.groupby([
        "Node Expansion Limit", "Algorithm"
    ])['Solution Cost'].agg(['mean', 'count', 'std'
                             ]).add_suffix('_solution_cost').reset_index()

    ret_cpu95 = rawdf.groupby(["Node Expansion Limit",
                               "Algorithm"])['cpu95'].agg(
                                   ['mean', 'count',
                                    'std']).add_suffix('_cpu95').reset_index()

    ci95_solution_cost = []
    ci95_cpu95 = []

    for i in ret_sol_cost.index:
        c = ret_sol_cost.loc[i]["count_solution_cost"]
        s = ret_sol_cost.loc[i]["std_solution_cost"]

        ci95_solution_cost.append(1.96 * s / math.sqrt(c))

        c = ret_cpu95.loc[i]["count_cpu95"]
        s = ret_cpu95.loc[i]["std_cpu95"]

        ci95_cpu95.append(1.96 * s / math.sqrt(c))

    ret_sol_cost['ci95_solution_cost'] = ci95_solution_cost
    ret_sol_cost['ci95_cpu95'] = ci95_cpu95
    ret_sol_cost["mean_cpu95"] = ret_cpu95["mean_cpu95"].values

    return ret_sol_cost


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
    limits = [10, 30, 100, 300, 1000]
    algorithms_data = {
        # "risk-cpu-dtb": "Risk",
        # "prisk-cpu-dtb": "PRisk",
        # "riskddSquish-cpu-dtb": "RiskDDSquish",
        # "riskdd-cpu-dtb": "RiskDD",
        "lsslrtastar-cpu-dtb": "LSS-LRTA*"
    }

    algorithms = {
        # "risk-cpu-dtb": "Nancy",
        # "prisk-cpu-dtb": "Nancy (pers.)",
        # "riskddSquish-cpu-dtb": "Nancy (DD)",
        # "riskdd-cpu-dtb": "Nancy (DD PE)",
        "lsslrtastar-cpu-dtb": "LSS-LRTA*"
    }

    baseline = "LSS-LRTA*"

    instance = []
    lookAheadVals = []
    cpu95 = []
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

                resultData = json.load(json_data)
                if float(resultData[algorithms_data[alg]]) != -1.0:
                    instance.append(str(jsonFile))
                    lookAheadVals.append(resultData["Lookahead"])
                    cpu95.append(resultData["cpu-percentiles"][94])
                    algorithm.append(algorithms[alg])
                    solutionCost.append(resultData[algorithms_data[alg]])

    rawdf = pd.DataFrame({
        "instance": instance,
        "Node Expansion Limit": lookAheadVals,
        "Solution Cost": solutionCost,
        "Algorithm": algorithm,
        "cpu95": cpu95
    })

    cpudf = getCpuStatistics(rawdf)

    print cpudf

    print("building plots...")

    nowstr = datetime.now().strftime("%d%m%Y-%H%M")

    if sys.argv[1] == "coverage":
        makeCoverageTable(rawdf, tileType)

    elif sys.argv[1] == "cpu95":
        makeCpuPlot(
            13, 10, "mean_cpu95", "mean_solution_cost",
            "ci95_solution_cost", "ci95_cpu95", cpudf, 0.35, "Algorithm",
            algorithms.values(), "cpu95", "Solution Cost",
            "../../plots/" + tileType + '/' + tileType +
            "-tile-solution-cost-vs-cpu-" + nowstr + ".png", markers)
    else:
        printUsage()


if __name__ == '__main__':
    main()
