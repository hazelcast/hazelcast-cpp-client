cd build;
msbuild /verbosity:quiet  /property:Configuration=Debug /property:Platform=x86 /AdditionalLibPaths=.
cd ..;
cd java;
javac -cp .:hazelcast-3.2-SNAPSHOT.jar ClientTCPIPListener.java
./clientTestStatic.exe;
#./clientTestShared.exe;

