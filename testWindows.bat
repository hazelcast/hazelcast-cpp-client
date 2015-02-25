rm -rf build
mkdir build
cd build
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
MSBuild.exe HazelcastClient.sln /property:Configuration=Release /p:VisualStudioVersion=12.0 || exit /b 1 
cd ..
cd java
start "cpp-java" mvn package exec:java -Dexec.mainClass="CppClientListener"
ping 1.1.1.1 -n 1 -w 20000 > nul
cd ..
build\hazelcast\test\Release\clientTest_STATIC_64.exe.exe || exit /b 1
taskkill /F /FI "WINDOWTITLE eq cpp-java"
