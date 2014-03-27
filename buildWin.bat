mkdir build32
cd .\build32
cd
cmake ..
::MSBuild.exe HazelcastClient.sln /clp:NoSummary;NoItemAndPropertyList;ErrorsOnly /nologo
cd ..

mkdir build64
cd .\build64
cd;
cmake .. -G "Visual Studio 10 Win64"
::MSBuild.exe HazelcastClient.sln /clp:NoSummary;NoItemAndPropertyList;ErrorsOnly /nologo
cd ..
