#rm ../generateTrees
#rm ../lastIncremental
#rm ../backupTestsDFS
#rm ../backupTestsAS
rm -rf ../realtimeSolver
rm -rf ../realtimeSolver-debug

#g++ -std=c++11 ../cpp/generateTrees.cpp -o ../generateTrees
#g++ -std=c++11 ../cpp/lastIncrementalTests.cpp -o ../lastIncremental
#g++ -std=c++11 ../cpp/backupTestsDFS.cpp -o ../backupTestsDFS
#g++ -std=c++11 ../cpp/backupTestsAS.cpp -o ../backupTestsAS
g++ -o3 -std=c++11 ../cpp/expansionTests.cpp -o ../realtimeSolver
g++ -g -std=c++11 ../cpp/expansionTests.cpp -o ../realtimeSolver-debug

#chmod a+x ../generateTrees
#chmod a+x ../lastIncremental
#chmod a+x ../backupTestsDFS
#chmod a+x ../backupTestsAS
chmod a+x ../realtimeSolver
chmod a+x ../realtimeSolver-debug
