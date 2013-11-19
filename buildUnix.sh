rm -rf build;
rm ./hazelcast/test/java/clientTest;
mkdir build;
cd build;
cmake ..;
make;
../hazelcast/test/java/clientTest;
