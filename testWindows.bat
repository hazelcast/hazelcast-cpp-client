rm -rf build
mkdir build
cd build
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug 
MSBuild.exe HazelcastClient.sln /property:Configuration=Debug /p:VisualStudioVersion=12.0
cd ..
cd java
start "cpp-java" mvn package exec:java -Dexec.mainClass="CppClientListener"
ping 1.1.1.1 -n 1 -w 3000 > nul
cd ..
build\hazelcast\test\Debug\clientTest_STATIC_64.exe.exe
taskkill /F /FI "WINDOWTITLE eq cpp-java"
