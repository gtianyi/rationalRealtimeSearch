#!/usr/bin/env python
'''
python2 script
plotting code for generate nancydd related plots

Author: Tianyi Gu
Date: 08/15/2019
Update: 10/15/2019
Update: 04/18/2020
Update: 04/27/2020
Update: 05/11/2020
'''

__author__ = 'TianyiGu'

import argparse
import json
import os
from collections import OrderedDict
# import sys
from datetime import datetime

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns


def configure(args):
    # limits = [3, 10, 30, 100, 300, 1000]
    # limits = [10, 30, 100, 300, 1000]
    limits = [30, 100, 300, 1000]
    # limits = [30]
    # limits = [100, 300, 1000]

    algorithms_data = {
        "ie": "ie",
        "ie-nancy": "ie",
        "ie-nancyAll": "ie",
        "ie-nancy-tlaopen": "ie",
        "ie--as": "ie",
        "risk": "risk",
        "lsslrtastar": "lsslrtastar",
        "riskddSquish": "riskddSquish"
    }

    algorithms_data_old = {
        # "astar": "A*",
        # "fhat": "F-Hat",
        # "bfs": "BFS",
        "risk": "Risk",
        # "risk-learnhhat": "Risk",
        # "risk-withassump": "Risk",
        # "risk-cpu-dtb": "Risk",
        # "risk-nobug": "Risk",
        # "riskdd-cpu-dtb": "RiskDD",
        # "riskdd-nopersist": "RiskDD",
        # "riskdd": "RiskDD",
        # "prisk": "PRisk",
        # "prisk-nop-learnhhat": "PRisk",
        # "prisk-withassump": "PRisk",
        # "prisk-withassump": "PRisk",
        # "prisk-withassump-learnhhat": "PRisk",
        # "prisk-cpu-dtb": "PRisk",
        # "prisk-nobug": "PRisk",
        # "riskddSquish": "RiskDDSquish",
        # "riskddSquish-nopersist": "RiskDDSquish",
        # "riskddSquish-nop-withassump": "RiskDDSquish",
        # "riskddSquish-newP-withassump": "RiskDDSquish",
        # "riskddSquish-newP": "RiskDDSquish",
        # "riskddSquish-1kt": "RiskDDSquish",
        "lsslrtastar": "LSS-LRTA*",
        # "lsslrtastar-cpu-dtb": "LSS-LRTA*",
        # "riskdd-lssTr": "RiskDD",
        # "riskddSquish-lssTr": "RiskDDSquish"
        # "riskddSquish-cpu-dtb-dumpallcpu": "RiskDDSquish"
        "riskddSquish": "RiskDDSquish"
        # "riskddSquish-newP": "RiskDDSquish",
        # "riskddSquish-nop-withassump": "RiskDDSquish"
    }

    if args.old_data:
        algorithms_data.update(algorithms_data_old)

    algorithms = OrderedDict({
        "ie": "IE",
        "ie-nancy": "IE-Nancy-TLA",
        "ie-nancyAll": "IE-Nancy-TLAAndOpen",
        "ie-nancy-tlaopen": "IE-Nancy-Open",
        "ie--as": "IE-AS",
        # "astar": "A*",
        # "fhat": "F-Hat",
        # "bfs": "BFS",
        "risk": "Nancy (pers.)",
        # "risk-learnhhat": "Nancy-hhat",
        # "risk-withassump": "Nancy-fix-assumption",
        # "risk-cpu-dtb": "Nancy",
        # "risk-nobug": "Nancy NOBUG",
        # "riskdd": "Nancy (DD PE)",
        # "riskdd-cpu-dtb": "Nancy (DD PE)",
        # "riskdd-nopersist": "Nancy (DD PE Nper)",
        # "prisk": "Nancy (pers.)",
        # "prisk-withassump-learnhhat": "Nancy (pers-fix-assumption-hhat.)",
        # "prisk-nop-learnhhat": "Nancy (pers-hhat.)",
        # "prisk-withassump": "Nancy (pers-fix-assumption.)",
        # "prisk-cpu-dtb": "Nancy (pers.)",
        # "prisk-nobug": "Nancy (pers. NOBUG)",
        # "riskddSquish": "Nancy (DD)",
        # "riskddSquish-nopersist": "Nancy (DD Nper)",
        # "riskddSquish-nop-withassump": "Nancy (Assumption-DD Nper)",
        # "riskddSquish-newP-withassump": "Nancy (Assumption-DD NewPer)",
        # "riskddSquish-newP": "Nancy (DD NewPer)",
        # "riskddSquish-1kt": "Nancy (DD 1kt)",
        "lsslrtastar": "LSS-LRTA*",
        # "lsslrtastar-cpu-dtb": "LSS-LRTA*",
        # "lsslrtastar-new": "LSS-LRTA* BUG",
        # "riskdd-lssTr": "Nancy (DD PE LSSTR)",
        # "riskddSquish-lssTr": "Nancy (DD LSSTR)"
        # "riskddSquish-cpu-dtb": "Nancy (DD)"
        "riskddSquish": "Nancy (DD)"
        # "riskddSquish-newP": "Nancy (DD)",
        # "riskddSquish-nop-withassump": "Nancy (Assumption-DD)"
    })

    #specify the order for camera ready of AAAI-20
    # algorithm_order = [
    # 'Nancy (DD)', 'Nancy (DD NewPer)', 'Nancy (Assumption-DD NewPer)',
    # 'LSS-LRTA*', 'Nancy (pers.)', 'Nancy', 'Nancy-hhat',
    # 'Nancy (pers-fix-assumption.)', 'Nancy (pers-fix-assumption-hhat.)',
    # 'Nancy (pers-hhat.)'
    # ]
    algorithm_order = [
        'IE', 'IE-AS', 'IE-Nancy-Open', 'IE-Nancy-TLA', 'IE-Nancy-TLAAndOpen',
        'Nancy (DD)', 'LSS-LRTA*', 'Nancy (pers.)'
    ]
    # algorithm_order = ['Nancy (DD)', 'LSS-LRTA*']

    baseline = "LSS-LRTA*"

    return limits, algorithms_data, algorithms, algorithm_order, baseline


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

    plt.setp(ax.get_legend().get_texts(), fontsize='18')  # for legend text
    plt.setp(ax.get_legend().get_title(), fontsize='18')  # for legend title

    plt.savefig(outputName, bbox_inches="tight", pad_inches=0)
    plt.close()
    plt.clf()
    plt.cla()
    return


def makeCoverageTable(dataframe, domainType, subdomainType, size):
    grp = dataframe.groupby(['Node Expansion Limit',
                             'Algorithm'])['Solution Cost'].count()
    print(grp)
    grp.to_csv("../../plots/" + domainType + "/coverage-" + subdomainType +
               "-" + size + ".csv")


def parseArugments():

    parser = argparse.ArgumentParser(description='realtimeExpPlot')

    parser.add_argument(
        '-d',
        action='store',
        dest='domain',
        help='domain: slidingTile(default), pancake, racetrack',
        default='slidingTile')

    parser.add_argument(
        '-s',
        action='store',
        dest='subdomain',
        help='subdomain: tile: uniform(default), heavy, inverse; \
        pancake: regular, heavy; \
        racetrack : barto-bigger, hanse-bigger-double, uniform',
        default='uniform')

    parser.add_argument('-z',
                        action='store',
                        dest='size',
                        help='domain size (default: 4)',
                        default='4')

    parser.add_argument(
        '-t',
        action='store',
        dest='plotType',
        help='plot type, pairwise(default), solutioncost, coverage',
        default='pairwise')

    parser.add_argument('-o',
                        action='store_true',
                        dest='old_data',
                        help='Set old data switch to true',
                        default=False)

    return parser


def readData(args, algorithms, algorithms_data, baseline):
    domainSize = args.size
    domainType = args.domain
    subdomainType = args.subdomain

    instance = []
    lookAheadVals = []
    algorithm = []
    solutionCost = []
    differenceCost = []

    print("reading in data...")

    inPath = "../../results/" + domainType + "/expansionTests/NancyDD/" + \
        subdomainType + '/alg'

    if domainType == "slidingTile" or domainType == "pancake":
        inPath = inPath + '/' + domainSize

    for alg in algorithms:
        inPath_alg = inPath.replace('alg', alg)
        for jsonFile in os.listdir(inPath_alg):
            if jsonFile[-5:] != ".json":
                continue
            with open(inPath_alg + "/" + jsonFile) as json_data:
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
    return df, rawdf


def plotting(args, parser, df, rawdf, baseline, limits, algorithm_order):
    print("building plots...")

    domainSize = args.size
    domainType = args.domain
    subdomainType = args.subdomain

    markers = [
        "o", "v", "s", "<", "p", "h", "^", "D", "X", ">", "o", "v", "s", "<",
        "p", "h", "^", "D", "X", ">"
    ]

    nowstr = datetime.now().strftime("%d%m%Y-%H%M")

    out_dir = "../../plots/" + domainType

    if not os.path.exists(out_dir):
        os.mkdir(out_dir)

    out_file = out_dir + '/' + domainType + "-" + subdomainType + "-" + domainSize + '-' + nowstr

    if args.plotType == "coverage":
        makeCoverageTable(rawdf, domainType, subdomainType, domainSize)

    elif args.plotType == "pairwise":
        makeDifferencePlot(13, 10, "Node Expansion Limit",
                           "Algorithm Cost - " + baseline + " Cost", df, 0.35,
                           "Algorithm", limits, algorithm_order,
                           "Node Expansion Limit",
                           "Algorithm Cost - " + baseline + " Cost",
                           out_file + "-pairwise.eps", markers)

    elif args.plotType == "solutioncost":
        makeDifferencePlot(13, 10, "Node Expansion Limit", "Solution Cost", df,
                           0.35, "Algorithm", limits, algorithm_order,
                           "Node Expansion Limit", "Solution Cost",
                           out_file + "-solutioncost.png", markers)
    else:
        parser.print_help()


def main():
    parser = parseArugments()
    args = parser.parse_args()
    print(args)

    limits, algorithms_data, algorithms, algorithm_order, baseline = configure(
        args)

    df, rawdf = readData(args, algorithms, algorithms_data, baseline)

    plotting(args, parser, df, rawdf, baseline, limits, algorithm_order)


if __name__ == '__main__':
    main()
