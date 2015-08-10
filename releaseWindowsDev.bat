echo "Compiling Static 64bit library"
mkdir ReleaseStatic64
cd .\ReleaseStatic64
cd;
cmake .. -G "Visual Studio 10 Win64" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release 
MSBuild.exe HazelcastClient.sln /property:Configuration=Release 
cd ..

echo "Compiling Shared 64bit library"
mkdir ReleaseShared64
cd .\ReleaseShared64
cd;
cmake .. -G "Visual Studio 10 Win64" -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release 
MSBuild.exe HazelcastClient.sln /property:Configuration=Release 
cd ..

echo "Moving 64bit libraries to target"
mkdir .\cpp\Windows_64\hazelcast\lib
mkdir .\cpp\Windows_64\hazelcast\include\hazelcast\
mkdir .\cpp\Windows_64\external\include
mkdir .\cpp\Windows_64\examples

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\

copy ReleaseShared64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\
copy ReleaseStatic64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\

echo "Moving 64bit external libraries to target"
xcopy /S /Q external\include\* cpp\Windows_64\external\include\

echo "Moving 64bit examples to target"
copy examples\*cpp  cpp\Windows_64\examples
copy ReleaseStatic64\examples\Release\*exe  cpp\Windows_64\examples

echo "Clearing tempraroy 64bit librares"
rm -rf ./ReleaseShared64
rm -rf ./ReleaseStatic64

