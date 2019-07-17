#!/usr/bin/env python3
import sys
import os
import subprocess

graphsTime = ["USA-road-t.BAY.gr", "USA-road-t.W.gr", "USA-road-t.CTR.gr", "USA-road-t.USA.gr"]
graphsDistance = ["USA-road-d.BAY.gr", "USA-road-d.W.gr", "USA-road-d.CTR.gr", "USA-road-d.USA.gr"]

graphDir = sys.argv[1]
dir_path = os.path.dirname(os.path.realpath(__file__))
commitId = subprocess.check_output(["git", "describe", "--always"]).strip().decode("ascii")
resultsDir = os.path.join(str(dir_path), "../results", str(commitId))
os.makedirs(resultsDir, exist_ok=True)

def runGraph(graphPath, turnCosts, time, dijkstra, EHForwardStalling=False, EHBackwardStalling=False, CHNoStallOnDemand=False, minimalSearchSpace=False, DFSPreOrder=True):
    graphName = os.path.basename(graphPath)
    resultsFile = os.path.join(resultsDir, graphName)
    if turnCosts :
        resultsFile += "TurnCosts"
    if dijkstra:
        resultsFile += "Dijkstra"

    numQueries = 100000
    if dijkstra:
        numQueries = 1000

    uTurnCost = 0
    if time:
        if graphName.startswith("USA"):
            uTurnCost = 3000
        elif graphName.startswith("scc"):
            uTurnCost = 1000
        else:
            print("Error! don't know cost for uturn for " + graphName)
    command = os.path.join(dir_path, "../build/app/benchmark") + " " + graphPath + " -q " + str(numQueries)

    if graphName.startswith("scc"):
        command += " --useCH"

    if turnCosts:
        command += " -t -u " + str(uTurnCost)

    if dijkstra:
        command += " -d"

    if DFSPreOrder:
        command += " --DFSPreOrder"
        resultsFile += "DFSPreOrder"

    if EHForwardStalling :
        command += " --EHForwardStalling"
        resultsFile += "EHForwardStalling"

    if EHBackwardStalling :
        command += " --EHBackwardStalling"
        resultsFile += "EHBackwardStalling"

    if CHNoStallOnDemand:
        command += " --CHNoStallOnDemand"
        resultsFile += "CHNoStallOnDemand"

    if minimalSearchSpace:
        command += " --minimalSearchSpace"
        resultsFile += "minimalSearchSpace"


    command += " > " + resultsFile

    print("execute command \n " + command)
    output = subprocess.check_output(command, shell=True)
    print(output)

def buildGraph(graphPath, turnCosts, time):
    graphName = os.path.basename(graphPath)
    resultsFile = os.path.join(resultsDir, graphName)
    if turnCosts :
        resultsFile += "TurnCosts"
    resultsFile += "Build"

    numQueries = 1

    uTurnCost = 0
    if time:
        if graphName.startswith("USA"):
            uTurnCost = 3000
        elif graphName.startswith("scc"):
            uTurnCost = 1000
        else:
            print("Error! don't know cost for uturn for " + graphName)

    command = os.path.join(dir_path, "../build/app/benchmark") + " " + graphPath + " -q " + str(numQueries) + " --rebuild"

    if graphName.startswith("scc"):
        command += " --useCH"

    if turnCosts:
        command += " -t -u " + str(uTurnCost)

    command += " > " + resultsFile

    print("execute command \n " + command)
    output = subprocess.check_output(command, shell=True)
    print(output)

def runGraphAll(graphName, _time, withDijkstra=False, stalling=False):
    graphPath = os.path.join(graphDir, graphName)

    buildGraph(graphPath, turnCosts=True, time=_time)
    buildGraph(graphPath, turnCosts=False, time=_time)

    runGraph(graphPath, turnCosts=False, time=_time, dijkstra=False)
    runGraph(graphPath, turnCosts=True, time=_time, dijkstra=False)

    if withDijkstra:
        runGraph(graphPath, turnCosts=True, time=_time, dijkstra=True)

    if stalling:
        runGraph(graphPath, turnCosts=True, time=_time, dijkstra=False, EHForwardStalling=False, EHBackwardStalling=True, CHNoStallOnDemand=False, minimalSearchSpace=True)
        runGraph(graphPath, turnCosts=True, time=_time, dijkstra=False, EHForwardStalling=False, EHBackwardStalling=False, CHNoStallOnDemand=True, minimalSearchSpace=False)
        runGraph(graphPath, turnCosts=True, time=_time, dijkstra=False, EHForwardStalling=True, EHBackwardStalling=False, CHNoStallOnDemand=False, minimalSearchSpace=False)
        runGraph(graphPath, turnCosts=True, time=_time, dijkstra=False, EHForwardStalling=False, EHBackwardStalling=True, CHNoStallOnDemand=False, minimalSearchSpace=False)



for g in graphsTime:
    runGraphAll(g, True)

for g in graphsDistance:
    runGraphAll(g, False)

runGraphAll("scc-eur2time.gr", True, True, True)
runGraphAll("scc-eur2dist.gr", False, True, True)
