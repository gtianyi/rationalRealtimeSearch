CXX=g++
STD=c++11

all: clean debug release thrt thrt-debug

clean:
	rm -rf realtimeSolver
	rm -rf realtimeSolver-debug
	rm -rf thrt
	rm -rf thrt-debug

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

thrt: ./cpp/trialBased/trialbasedTestRT.cpp ./cpp/trialBased/TrialBasedRealTimeSearch.h
	$(CXX) -O3 -std=${STD} ./cpp/trialBased/trialbasedTestRT.cpp -o thrt

thrt-debug: ./cpp/trialBased/trialbasedTestRT.cpp ./cpp/trialBased/TrialBasedRealTimeSearch.h
	$(CXX) -g -D DEBUG -std=${STD} ./cpp/trialBased/trialbasedTestRT.cpp -o thrt-debug
