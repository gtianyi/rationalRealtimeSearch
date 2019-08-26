rm -rf ../expansionTests-debug
rm -rf ../expansionTests-debug-quick

g++ -g -std=c++11 ../cpp/expansionTests.cpp -o ../expansionTests-debug
g++ -o3 -std=c++11 ../cpp/expansionTests.cpp -o ../expansionTests-debug-quick

chmod a+x ../expansionTests-debug
chmod a+x ../expansionTests-debug-quick
