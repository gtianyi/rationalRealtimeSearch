CXX=g++
STD=c++11

all: clean debug release

clean:
	rm -rf realtimeSolver
	rm -rf realtimeSolver-debug

release:
	if [ ! -f realtimeSolver.lock ]; then \
		rm -rf realtimeSolver; \
		${CXX} -o3 -std=${STD} ./cpp/expansionTests.cpp -o realtimeSolver ;\
		chmod a+x ./realtimeSolver ;\
	else\
		echo "the release executable is running!!! Try make debug" ; \
	fi;

debug:
	rm -rf realtimeSolver-debug
	${CXX} -g -D DEBUG -std=${STD} ./cpp/expansionTests.cpp -o realtimeSolver-debug
	chmod a+x realtimeSolver-debug
