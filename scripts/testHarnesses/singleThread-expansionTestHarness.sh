#!/bin/bash

if [ "$1" = "help" ] || [ "$1" = "-help" ] || [ "$1" = "?" ]
then
  echo "./expansionTestHarness.sh <starting instance #> <# of instances to test> <Domain Type> <Domain Variables> <# of algorithm> <algorithm> <optional: additional algorithms> <Lookahead value> <optional: additional lookahead values>"
  echo "Available domain types are TreeWorld and SlidingPuzzle"
  echo "Algorithms are: bfs, astar, fhat, lsslrtastar, risk, riskdd"
  echo "Domain variables for TreeWorld: <branching factor> <tree depth>"
  echo "Domain variables for SlidingPuzzle: <puzzle dimensions> <puzzle type>"
  exit 1
fi

if (($# < 7))
then
  echo "./expansionTestHarness.sh <starting instance #> <# of instances to test> <Domain Type> <Domain Variables> <# of algorithm> <algorithm> <optional: additional algorithms> <Lookahead value> <optional: additional lookahead values>"
  echo "Available domain types are TreeWorld and SlidingPuzzle"
  echo "Algorithms are: bfs, astar, fhat, lsslrtastar, risk, riskdd"
  echo "Domain variables for TreeWorld: <branching factor> <tree depth>"
  echo "Domain variables for SlidingPuzzle: <puzzle dimensions> <puzzle type>"
  exit 1
fi

# Which instance to start testing on
firstInstance=$1

# The maximum number of instances to test on
maxInstances=$2
lastInstance=$(( $firstInstance + $maxInstances ))

# The domain to run on
domainType=$3

trap "exit" INT

if [ "$domainType" = "SlidingPuzzle" ] || [ "$domainType" = "pancake" ]
then
  dimensions=$4
  subdomain=$5
  algNum=$6


  for (( i=1;i<=$algNum;i++ ))
  do 
    algindex=$((6+$i))	  
	algname=${!algindex}
	algstop=$((6+$algNum+1))
	echo $algname
    for lookahead in "${@:$algstop}"
    do
      echo "lookahead $lookahead"
	  mkdir -p ../../results/pancake/expansionTests/NancyDD/${subdomain}/${algname}/${dimensions}
	  #mkdir -p ../../results/SlidingTilePuzzle/expansionTests/NancyDD/${subdomain}/${algname}/${dimensions}x${dimensions}
      instance=$firstInstance
      while ((instance < lastInstance))
      do
	    infile="../../../worlds/pancake/${instance}-${dimensions}.pan"
		#file="../../worlds/pancake/${instance}-${dimensions}x${dimensions}.st"
		file_name="../../results/pancake/expansionTests/NancyDD/\
${subdomain}/${algname}/${dimensions}/LA${lookahead}-${instance}"
		outfile="${file_name}.json"
		tempfile="${file_name}.temp"
        		  
	    if [ -f ${outfile} ] || [ -f ${tempfile} ]; then 
	      let instance++
	    else

		  echo "ph" > ${tempfile}
		  echo "runing LA${lookahead}-${instance}" 

	      timeout 1800 ./../../expansionTests ${domainType} ${lookahead} ${subdomain} ${algname} ${outfile} < ${infile}

		  if [ -f ${outfile} ]; then
		     rm ${tempfile}
	      fi

	      let instance++
	    fi
      done
    done
  done
else
  echo "Available domain types are pancake and SlidingPuzzle"
  echo "Domain variables for TreeWorld: <branching factor> <tree depth>"
  echo "Domain variables for SlidingPuzzle: <puzzle dimensions> <puzzle type>"
  exit 1
fi
