#!/bin/bash

print_usage(){
  echo "./singleThread-realtimeSolver.sh"
  echo "[-f instance]                    default: 1"
  echo "[-n # of instances to test]      default: 100"
  echo "[-d domain]                      default: pancake"
  echo "[-s subdomain]                   default: regular"
  echo "[-z domain size]                 default: 16"
  echo "[-a algorithm ]"
  echo " support list,eg: -a a1 -a a2    default: risk, riskddSquish, lsslrtastar"
  echo "[-l lookahead, ]"
  echo " support list,eg: -l 10 -l 30    default: 3, 10, 30, 100, 300, 1000"
  echo "[-e algorithm extention          default: null]"
  echo "[-h help]"
  exit 1
}

if [ "$1" = "-h" ] || [ "$1" = "-help" ] || [ "$1" = "?" ]; then
  print_usage
fi

# Which instance to start testing on
first=1
# The number of instances to test on
n_of_i=100
domain="pancake"
subdomain="regular"
size="32"
algorithms=("risk" "riskddSquish" "lsslrtastar")
lookaheads=(3 10 30 100 300 1000)
maxProcs=1
extention=""

algCleared=false
lookaheadCleared=false
#parse arguments
for (( i=1; i <= "$#"; i++ )); do
    if [ ${!i} == "-f" ]; then
        if [ $((i+1)) -le "$#" ]; then
            var=$((i+1))
            first=${!var}
        fi
    fi

    if [ ${!i} == "-n" ]; then
        if [ $((i+1)) -le "$#" ]; then
            var=$((i+1))
            n_of_i=${!var}
        fi
    fi

    if [ ${!i} == "-d" ]; then
        if [ $((i+1)) -le "$#" ]; then
            var=$((i+1))
			domain=${!var}
        fi
    fi

    if [ ${!i} == "-s" ]; then
        if [ $((i+1)) -le "$#" ]; then
            var=$((i+1))
			subdomain=${!var}
        fi
    fi

    if [ ${!i} == "-z" ]; then
        if [ $((i+1)) -le "$#" ]; then
            var=$((i+1))
			size=${!var}
        fi
    fi

    if [ ${!i} == "-a" ]; then
        if [ $((i+1)) -le "$#" ]; then
			if ! $algCleared; then
				unset algorithms
				algCleared=true
			fi
            var=$((i+1))
			algorithms+=(${!var})
        fi
    fi

    if [ ${!i} == "-l" ]; then
        if [ $((i+1)) -le "$#" ]; then
			if ! $lookaheadCleared; then
				unset lookaheads
			    lookaheadCleared=true
			fi
            var=$((i+1))
			lookaheads+=(${!var})
        fi
    fi

    if [ ${!i} == "-e" ]; then
        if [ $((i+1)) -le "$#" ]; then
            var=$((i+1))
			extention=${!var}
        fi
    fi

    if [ ${!i} == "-h" ]; then
		print_usage
    fi

done

echo "first ${first}"
echo "n_of_i ${n_of_i}"
echo "domain ${domain}"
echo "subdomain ${subdomain}"
echo "size ${size}"
echo "lookaheads ${lookaheads[*]}"
echo "algorithms ${algorithms[*]}"
echo "thread ${maxProcs}"
echo "extention ${extention}"

algorithms_dir=(${algorithms[@]})

if [ ! -z "$extention" ]; then
	for i in "${!algorithms_dir[@]}"; do
		algorithms_dir[i]="${algorithms_dir[i]}-${extention}"
	done
fi

echo "algorithms_dir ${algorithms_dir[*]}"

exit 1


last=$(( $first + $n_of_i ))

###############################################

trap "exit" INT

for algid in "${!algorithms[@]}"; do 

	algname=algorithms[algid]
	algdir=algorithms_dir[algid]
	echo $algname
	echo $algdir

    for lookahead in "${lookaheads[@]}"; do
      echo "lookahead $lookahead"
	  mkdir -p ../../results/${domain}/expansionTests/NancyDD/${subdomain}/${algdir}/${size}
	  #mkdir -p ../../results/SlidingTilePuzzle/expansionTests/NancyDD/${subdomain}/${algname}-nop/${size}x${size}
      instance=$first
      while ((instance < last))
      do
	    infile="../../../worlds/${domain}/${instance}-${size}.pan"
		#file="../../worlds/pancake/${instance}-${size}x${size}.st"
		file_name="../../results/${domain}/expansionTests/NancyDD/\
${subdomain}/${algdir}/${size}/LA${lookahead}-${instance}"
		outfile="${file_name}.json"
		tempfile="${file_name}.temp"
        		  
	    if [ -f ${outfile} ] || [ -f ${tempfile} ]; then 
	      let instance++
	    else

		  echo "ph" > ${tempfile}
		  echo "runing LA${lookahead}-${instance}" 

	      timeout 1800 ./../../expansionTests ${domain} ${lookahead} ${subdomain} ${algname} ${outfile} < ${infile}

		  if [ -f ${outfile} -a -f ${tempfile}]; then
		     rm ${tempfile}
	      fi

	      let instance++
	    fi
      done
    done
done
