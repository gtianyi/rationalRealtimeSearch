#!/bin/bash

if [ "$1" = "help" ] || [ "$1" = "-help" ] || [ "$1" = "?" ]
then
  echo "./multiThread-expansionTestHarness.sh <# of processes> <starting instance #> <# of instances to test> <Domain Type> <Domain Variables> <# of algorithm> <algorithm> <optional: additional algorithms> <Lookahead value> <optional: additional lookahead values>"
  echo "Available domain types are TreeWorld and SlidingPuzzle"
  echo "Algorithms are: bfs, astar, fhat, lsslrtastar, risk, riskdd"
  echo "Domain variables for TreeWorld: <branching factor> <tree depth>"
  echo "Domain variables for SlidingPuzzle: <puzzle dimensions> <puzzle type>"
  exit 1
fi

if (($# < 8))
then
  echo "./multiThread-expansionTestHarness.sh <# of processes> <starting instance #> <# of instances to test> <Domain Type> <Domain Variables> <# of algorithm> <algorithm> <optional: additional algorithms> <Lookahead value> <optional: additional lookahead values>"
  echo "Available domain types are TreeWorld and SlidingPuzzle"
  echo "Algorithms are: bfs, astar, fhat, lsslrtastar, risk, riskdd"
  echo "Domain variables for TreeWorld: <branching factor> <tree depth>"
  echo "Domain variables for SlidingPuzzle: <puzzle dimensions> <puzzle type>"
  exit 1
fi

# Max number of background processes to start, should probably not be more than the number of cores on the machine
maxProcs=$1

numProcs=0

trap "exit" INT

while ((numProcs < ${maxProcs}))
do
	./singleThread-expansionTestHarness.sh ${@:2} &
	sleep 1
    let numProcs++
done
