#!/bin/bash
print_usage(){
  echo "./multiThread-realtimeSolver.sh"
  echo "[-f instance]                    default: 1"
  echo "[-n # of instances to test]      default: 100"
  echo "[-d domain]                      default: pancake"
  echo "[-s subdomain]                   default: regular"
  echo "[-z domain size]                 default: 16"
  echo "[-a algorithm ]"
  echo " support list,eg: -a a1 -a a2    default: risk, riskddSquish, lsslrtastar, ie, thts-WAS, thts-GUCTS"
  echo "[-l lookahead, ]"
  echo " support list,eg: -l 10 -l 30    default: 3, 10, 30, 100, 300, 1000"
  echo "[-e algorithm extention          default: null]"
  echo "[-t thread number]               default: 1"
  echo "[-h help]"
  exit 1
}

if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "?" ]; then
  print_usage
fi

maxProcs=1

# only parse the -t argument here
# all arguments would be pass to singleThread
for (( i=1; i <= "$#"; i++ )); do

    if [ ${!i} == "-t" ]; then
        if [ $((i+1)) -le "$#" ]; then
            var=$((i+1))
			maxProcs=${!var}
        fi
    fi


    if [ ${!i} == "-h" ]; then
		print_usage
    fi

done

# Max number of background processes to start, 
# should probably not be more than the number of threads - 1 on the machine
numProcs=0

trap "exit" INT

while ((numProcs < ${maxProcs}))
do
	./singleThread-realtimeSolver.sh ${@} &
	sleep 1
    let numProcs++
done
