cd build;
make;
cd ..;
cd java;
javac -cp .:hazelcast-3.2-SNAPSHOT.jar ClientTCPIPListener.java
./clientTestStatic.exe;
#./clientTestShared.exe;

