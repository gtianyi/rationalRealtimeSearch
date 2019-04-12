import matplotlib.pyplot as plt
import pandas as pd
import json
import seaborn as sns
from os import listdir

def makeCoverageTable(dataframe):
   grp =  dataframe.groupby(['Node Expansion Limit','Algorithm'])['Solution Cost'].count()
   print(grp)
   # grp.to_csv("../../plots/inverse/coverage.csv")
    
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

df = pd.DataFrame({
    "instance":instance,
    "Node Expansion Limit":lookAheadVals,
    "Solution Cost":solutionCost,
    "Algorithm":algorithm
})

algorithmsExpC = ["A*", "F-Hat", "BFS", "Risk", "Riskdd", "LSS-LRTA*"]
# algorithmsExpC = ["A*", "F-Hat", "BFS", "Risk", "LSS-LRTA*"]

print("building tables...")

for instance in resultDirs:
    sns.set(rc={'figure.figsize': (11, 8), 'font.size': 26, 'text.color': 'black'})
    instanceDataExp = df.loc[(df["instance"] == instance) & (df["Solution Cost"]>0)]

    makeCoverageTable(instanceDataExp)


