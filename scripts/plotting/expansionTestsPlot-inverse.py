import matplotlib.pyplot as plt
import pandas as pd
import json
import seaborn as sns
from os import listdir

def makeViolinPlot(width, height, xAxis, yAxis, dataframe, dodge, hue, orderList, hueOrderList, xLabel, yLabel, outputName):
    sns.set(rc={'figure.figsize': (width, height), 'font.size': 26, 'text.color': 'black'})
    ax = sns.pointplot(x=xAxis, y=yAxis, hue=hue, order=orderList, hue_order=hueOrderList, data=dataframe, join=False, dodge=dodge, palette=sns.color_palette(["red"]), markers="_", errwidth=3, ci=95)
    ax.tick_params(colors='black', labelsize=12)
    plt.setp(ax.lines, zorder=100)
    plt.setp(ax.collections, zorder=100, label="")
    ax.legend_.remove()
    
    sns.violinplot(x=xAxis, y=yAxis, hue=hue, order=orderList, hue_order=hueOrderList, data=dataframe, palette="Set2")    

    plt.ylabel(yLabel, color='black', fontsize=18)
    plt.xlabel(xLabel, color='black', fontsize=18)
    plt.savefig(outputName, bbox_inches="tight", pad_inches=0)
    
    plt.close()
    plt.clf()
    plt.cla()
    return

def makeDifferencePlot(width, height, xAxis, yAxis, dataframe, dodge, hue, orderList, hueOrderList, xLabel, yLabel, outputName, markerList):
    sns.set(rc={'figure.figsize': (width, height), 'font.size': 26, 'text.color': 'black'})
    ax = sns.pointplot(x=xAxis, y=yAxis, hue=hue, order=orderList, hue_order=hueOrderList, data=dataframe, ci=95, errwidth=3, join=False, dodge=dodge, palette="Set2", markers=markerList)
    ax.tick_params(colors='black', labelsize=12)
    plt.ylabel(yLabel, color='black', fontsize=18)
    plt.xlabel(xLabel, color='black', fontsize=18)
    plt.savefig(outputName, bbox_inches="tight", pad_inches=0)
    plt.close()
    plt.clf()
    plt.cla()
    return

markers=["o", "v", "s", "<", "p", "h", "^", "D", "X", ">", "o", "v", "s", "<", "p", "h", "^", "D", "X", ">"]

# Hard coded result directories
resultDirs = {"4x4"}
tileType = "inverse"
limits = [3, 10, 30, 100, 300, 1000]
algorithms = ["A*", "F-Hat", "BFS", "Risk", "RiskDD", "LSS-LRTA*"]
# algorithms = ["A*", "F-Hat", "BFS", "Risk", "LSS-LRTA*"]

instance = []
lookAheadVals = []
algorithm = []
solutionCost = []
baselineCost = []
differenceCost = []

print("reading in data...")

for dir in resultDirs:
    for file in listdir("../../results/SlidingTilePuzzle/expansionTests/Nancy/"+tileType+'/' + dir):
        with open("../../results/SlidingTilePuzzle/expansionTests/Nancy/" +tileType+'/' + dir + "/" + file) as json_data:
            resultData = json.load(json_data)
            for algo in algorithms:
                instance.append(str(dir))
                lookAheadVals.append(resultData["Lookahead"])
                algorithm.append(algo)
                solutionCost.append(resultData[algo.replace("A*", "A*")])
                differenceCost.append(resultData[algo.replace("A*", "A*")] - resultData["A*"])
                baselineCost.append(resultData["A*"])

df = pd.DataFrame({
    "instance":instance,
    "Node Expansion Limit":lookAheadVals,
    "Solution Cost":solutionCost,
    "Base Line Cost":baselineCost,
    "Algorithm":algorithm
})

dfDiff = pd.DataFrame({
    "instance":instance,
    "Node Expansion Limit":lookAheadVals,
    "Algorithm Cost - A* Cost":differenceCost,
    "Solution Cost":solutionCost,
    "Base Line Cost":baselineCost,
    "Algorithm":algorithm
})

algorithmsExpC = ["A*", "F-Hat", "BFS", "Risk", "RiskDD", "LSS-LRTA*"]
# algorithmsExpC = ["A*", "F-Hat", "BFS", "Risk", "LSS-LRTA*"]

print("building plots...")

for instance in resultDirs:
    sns.set(rc={'figure.figsize': (11, 8), 'font.size': 26, 'text.color': 'black'})
    instanceDataExp = df.loc[(df["instance"] == instance) & (df["Solution Cost"]>0) & (df["Base Line Cost"]>0)]
    instanceDataDiffExp = dfDiff.loc[(dfDiff["instance"] == instance) & (dfDiff["Solution Cost"]>0) & (dfDiff["Base Line Cost"]>0)]

    makeViolinPlot(13, 10, "Node Expansion Limit", "Solution Cost", instanceDataExp, 0.59, "Algorithm", limits, algorithmsExpC, "Node Expansion Limit", "Solution Cost", "../../plots/"+tileType+'/'+"Experiment2CViolin" + instance + ".pdf")

    makeDifferencePlot(13, 10, "Node Expansion Limit", "Algorithm Cost - A* Cost", instanceDataDiffExp, 0.35, "Algorithm", limits, algorithmsExpC, "Node Expansion Limit", "Algorithm Cost - A* Cost", "../../plots/"+tileType+'/'+"Experiment2CDifference" + instance + ".pdf", markers)
