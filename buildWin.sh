Remove-Item -Force -Recurse build/;
Remove-Item ./java/clientTest;
mkdir build;
cd build;
cmake ..;
msbuild /verbosity:quiet  /property:Configuration=Debug /property:Platform=x86 /AdditionalLibPaths=.
cd ..;
cd java;
javac -cp .:hazelcast-3.2-SNAPSHOT.jar ClientTCPIPListener.java
./clientTestStatic.exe;
#./clientTestShared.exe;

