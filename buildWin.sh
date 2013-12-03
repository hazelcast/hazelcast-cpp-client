mkdir build;
cd build;
cmake ..;
MSBuild.exe HazelcastClient.sln;
#"C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
#MSBuild.exe HazelcastClient.sln /verbosity:quiet /p:PlatformTarget=x86;
cd ..;
cd java;
javac ClientTCPIPListener.java -cp .\hazelcast-3.2-SNAPSHOT.jar;