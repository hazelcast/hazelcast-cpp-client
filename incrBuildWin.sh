mkdir build;
cd build;
cmake ..;
msbuild /property:Configuration=Debug /p:WarningLevel=0 .\hz_cppClient.sln
../hazelcast/test/java/clientTest;