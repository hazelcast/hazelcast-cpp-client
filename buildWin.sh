mkdir build;
cd build;
cmake ..;
msbuild /verbosity:quiet /AdditionalLibPaths=.
cd ..;
cd java;
javac -cp .:hazelcast-3.2-SNAPSHOT.jar ClientTCPIPListener.java