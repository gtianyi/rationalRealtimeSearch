# ABOUT

This is a real-time solver framework. We implement algorithms such as LSSLRTA*, Nancy, Data-Driven Nancy.

# TO BUILD

make

# TO RUN
## Single Run (solve one problem instance)
Usage:
  ./realtimeSolver [OPTION...] < [input file]

 -d, --domain arg          domain type: randomtree, tile, pancake, racetrack\
                                (default: racetrack)\
 -s, --subdomain arg       puzzle type: uniform, inverse, heavy, sqrt;\
                           pancake type: regular, heavy;racetrack map :\
                           barto-bigger, hanse-bigger-double, uniform (default:
                           barto-bigger)\
 -a, --alg arg             realtime algorithm: bfs, astar, fhat,\
                           lsslrtastar, risk, riskdd, riskddSquish (default: risk)
 -l, --lookahead arg       expansion limit (default: 100)
 -o, --performenceOut arg  performence Out file (default: out.txt)
 -v, --pathOut arg         path Out file
 -h, --help                Print usage

## Run batch of experiments 
cd sciptt/testHarnesses

./multiThread-realtimeSolver.sh 
[-f instance]                    default: 1
[-n # of instances to test]      default: 100
[-d domain]                      default: pancake
[-s subdomain]                   default: regular
[-z domain size]                 default: 16
[-a algorithm ]
 support list,eg: -a a1 -a a2    default: risk, riskddSquish, lsslrtastar
[-l lookahead, ]
 support list,eg: -l 10 -l 30    default: 3, 10, 30, 100, 300, 1000
[-e algorithm extention          default: null]
[-t thread number]               default: 1
[-h help]
