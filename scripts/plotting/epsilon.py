'''
code for generate average global epsilons

Author: Tianyi Gu
Date: 03/04/2020
'''

#!/usr/bin/env python

__author__ = 'tianyigu'

import json
import sys
# from collections import OrderedDict
# from datetime import datetime
from os import listdir

# import matplotlib.pyplot as plt
# import numpy as np
# import pandas as pd
# import seaborn as sns


def printUsage():
    print "usage: python expansionTestsPlot.py"


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

    epsD = sum(epsilonD)/len(epsilonD)
    epsH = sum(epsilonH)/len(epsilonH)

    print ("epsD:", epsD)
    print ("epsH:", epsH)


if __name__ == '__main__':
    main()
