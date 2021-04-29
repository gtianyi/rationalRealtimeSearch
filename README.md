# ABOUT

This is a real-time solver framework. We implement algorithms such as LSSLRTA*, Nancy, Data-Driven Nancy.

# Paper
[1] Maximilian Fickert, Tianyi Gu, Leonhard Staut, Wheeler Ruml, Joerg Hoffmann, and Marek Petrik, Beliefs We Can Believe In: Replacing Assumptions with Data in Real-Time Search. Proceedings of the Thirty-fourth AAAI Conference on Artificial Intelligence (AAAI-20), 2020.

[[pdf]](http://cs.unh.edu/~tg1034/publication/DDNancy.pdf) [[slides]](http://cs.unh.edu/~tg1034/slides/DDNancy_slides.pdf) [[poster]](http://cs.unh.edu/~tg1034/slides/DDNancy_poster.pdf)

[2] Maximilian Fickert, Tianyi Gu, Leonhard Staut, Sai Lekyang, Wheeler Ruml, Joerg Hoffmann, and Marek Petrik, Real-time Planning as Data-driven Decision-making. Proceedings of the ICAPS Workshop on Bridging the Gap Between AI Planning and Reinforcement Learning (PRL-20), 2020.

[[pdf]](http://cs.unh.edu/~tg1034/publication/DDNancy-PRL.pdf) [[talk]](https://youtu.be/4f1ual5R4s0) [[slides]](http://cs.unh.edu/~tg1034/slides/prl_nancy_slides.pdf) [[poster]](http://cs.unh.edu/~tg1034/slides/prl_nancy_poster.pdf)

# How to build

make

# How to run
## Single Run (solve one problem instance)
```console
Usage:
  ./realtimeSolver [OPTION...] < [input file]

 -d, --domain arg          domain type: randomtree, tile, pancake, racetrack
                                (default: racetrack)
 -s, --subdomain arg       puzzle type: uniform, inverse, heavy, sqrt;
                           pancake type: regular, heavy;racetrack map :
                           barto-bigger, hanse-bigger-double, uniform (default:
                           barto-bigger)
 -a, --alg arg             realtime algorithm: bfs, astar, fhat,
                           lsslrtastar, risk, riskdd, riskddSquish (default: risk)
 -l, --lookahead arg       expansion limit (default: 100)
 -o, --performenceOut arg  performence Out file (default: out.txt)
 -v, --pathOut arg         path Out file
 -h, --help                Print usage
```

## Run batch of experiments 
```console
cd scipts/testHarnesses
```

```console
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
```

# Training Repo
Source code of offline training is avaiable [here](https://github.com/gtianyi/rationalRealtimeSearch-training)

# Problem Instances
All the instances can be found [here](https://github.com/gtianyi/searchDomainInstanceFiles)
