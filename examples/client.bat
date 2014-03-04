mkdir build
cd build
cmake ..
msbuild TestAppProject.sln /p:Configuration=Release /clp:NoSummary;NoItemAndPropertyList;ErrorsOnly /nologo
cd ..
Release\TestApp.exe