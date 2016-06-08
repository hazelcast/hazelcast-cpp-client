echo "Cleanup release directories"
rm -rf ./Release*
rm -rf ./cpp

echo "Compiling Static 32bit library"
mkdir ReleaseStatic32
cd .\ReleaseStatic32
cd
cmake .. -G "Visual Studio 12" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=32;Platform=win32;PlatformTarget=win32
cd ..

echo "Compiling Shared 32bit library"
mkdir ReleaseShared32
cd .\ReleaseShared32
cd;
cmake .. -G "Visual Studio 12" -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=32;Platform=win32;PlatformTarget=win32
cd ..

echo "Moving 32bit libraries to target"
mkdir .\cpp\Windows_32\hazelcast\lib\static
mkdir .\cpp\Windows_32\hazelcast\lib\shared
mkdir .\cpp\Windows_32\hazelcast\include\hazelcast\
mkdir .\cpp\Windows_32\external\include
mkdir .\cpp\Windows_32\examples

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\

copy ReleaseShared32\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\shared\
copy ReleaseStatic32\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\static\

echo "Moving 32bit external libraries to target"
xcopy /S /Q external\release_include\* cpp\Windows_32\external\include\

echo "Moving 32bit examples to target"
xcopy /S /Q examples  cpp\Windows_32\examples\src\
for /R ReleaseStatic32\examples %%G IN (*.exe) DO xcopy "%%G" cpp\Windows_32\examples

echo "Clearing temporary 32bit librares"
rm -rf .\ReleaseShared32
rm -rf .\ReleaseStatic32

echo "Compiling Static 64bit library"
mkdir ReleaseStatic64
cd .\ReleaseStatic64
cd;
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=64;Platform=x64;PlatformTarget=x64
cd ..

echo "Compiling Shared 64bit library"
mkdir ReleaseShared64
cd .\ReleaseShared64
cd;
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=64;Platform=x64;PlatformTarget=x64
cd ..

echo "Moving 64bit libraries to target"
mkdir .\cpp\Windows_64\hazelcast\lib\static
mkdir .\cpp\Windows_64\hazelcast\lib\shared
mkdir .\cpp\Windows_64\hazelcast\include\hazelcast\
mkdir .\cpp\Windows_64\external\include
mkdir .\cpp\Windows_64\examples

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\

copy ReleaseShared64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\shared\
copy ReleaseStatic64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\static\

echo "Moving 64bit external libraries to target"
xcopy /S /Q external\release_include\* cpp\Windows_64\external\include\

echo "Moving 64bit examples to target"
xcopy /S /Q examples  cpp\Windows_64\examples\src\
for /R ReleaseStatic64\examples %%G IN (*.exe) DO xcopy "%%G" cpp\Windows_64\examples

echo "Clearing tempraroy 64bit librares"
rm -rf ./ReleaseShared64
rm -rf ./ReleaseStatic64

@REM Verify release
call scripts/verifyReleaseWindows.bat || exit /b 1
