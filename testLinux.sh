set -e
rm -rf build
mkdir build
cd build
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
make
cd ..
cd java
mvn -q clean install
mvn exec:java -Dexec.mainClass="CppClientListener" &
sleep 20
cd ..
./build/hazelcast/test/clientTest_STATIC_64.exe
kill -9 $( jps | grep  Launcher | awk '{print $1;}' ) || true
