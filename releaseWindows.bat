echo "Compiling Static 32bit library"
mkdir ReleaseStatic32
cd .\ReleaseStatic32
cd
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release 
MSBuild.exe HazelcastClient.sln /property:Configuration=Release
cd ..

echo "Compiling Shared 32bit library"
mkdir ReleaseShared32
cd .\ReleaseShared32
cd;
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release 
MSBuild.exe HazelcastClient.sln /property:Configuration=Release
cd ..

echo "Moving 32bit libraries to target"
mkdir .\cpp\Windows_32\hazelcast\lib
mkdir .\cpp\Windows_32\hazelcast\include\hazelcast\
mkdir .\cpp\Windows_32\external\lib
mkdir .\cpp\Windows_32\external\include

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\

copy ReleaseShared32\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\
copy ReleaseStatic32\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\

echo "Moving 32bit external libraries to target"
xcopy /S /Q external\include\* cpp\Windows_32\external\include\
xcopy /S /Q external\lib\windows\32\* cpp\Windows_32\external\lib\

echo "Clearing tempraroy 32bit librares"
rm -rf .\ReleaseShared32
rm -rf .\ReleaseStatic32

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
mkdir .\cpp\Windows_64\external\lib
mkdir .\cpp\Windows_64\external\include

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\

copy ReleaseShared64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\
copy ReleaseStatic64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\

echo "Moving 64bit external libraries to target"
xcopy /S /Q external\include\* cpp\Windows_64\external\include\
xcopy /S /Q external\lib\windows\64\* cpp\Windows_64\external\lib\

echo "Clearing tempraroy 64bit librares"
rm -rf ./ReleaseShared64
rm -rf ./ReleaseStatic64

