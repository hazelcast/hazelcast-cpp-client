set -e
rm -rf build
mkdir build
cd build
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug
make
cd ..
cd java
mvn clean install
mvn exec:java -Dexec.mainClass="CppClientListener" &
cd ..
./build/hazelcast/test/clientTest_STATIC_64.exe
#lsof | grep IPv | grep 6543 | awk '{print $2}' | xargs -l -i kill -9 {}
