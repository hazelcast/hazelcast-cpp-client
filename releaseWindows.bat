echo "Cleanup release directories"
for /d %%G in ("Release*") do rd /s /q "%%G"
rd /s /q cpp

echo "Compiling Static 32bit library"
mkdir ReleaseStatic32
cd .\ReleaseStatic32
cd
cmake .. -G "Visual Studio 12" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=32;Platform=win32;PlatformTarget=win32
cd ..

echo "Compiling Static 32bit library with TLS support"
mkdir ReleaseStatic32TLS
cd .\ReleaseStatic32TLS
cd
cmake .. -G "Visual Studio 12" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=32;Platform=win32;PlatformTarget=win32
cd ..

echo "Compiling Shared 32bit library"
mkdir ReleaseShared32
cd .\ReleaseShared32
cd;
cmake .. -G "Visual Studio 12" -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=32;Platform=win32;PlatformTarget=win32
cd ..

echo "Compiling Shared 32bit library with TLS support"
mkdir ReleaseShared32TLS
cd .\ReleaseShared32TLS
cd;
cmake .. -G "Visual Studio 12" -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32  -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=32;Platform=win32;PlatformTarget=win32
cd ..

echo "Moving 32bit libraries to target"
mkdir .\cpp\Windows_32\hazelcast\lib\static
mkdir .\cpp\Windows_32\hazelcast\lib\tls\static
mkdir .\cpp\Windows_32\hazelcast\lib\shared
mkdir .\cpp\Windows_32\hazelcast\lib\tls\shared
mkdir .\cpp\Windows_32\hazelcast\include\hazelcast\

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\

copy ReleaseShared32\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\shared\
copy ReleaseShared32TLS\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\tls\shared\
copy ReleaseStatic32\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\static\
copy ReleaseStatic32TLS\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\tls\static\

echo "Copying external libraries and the examples"
mkdir cpp\external\include
xcopy external\release_include\* cpp\external\include\ /s
mkdir cpp\examples\src
xcopy examples\* cpp\examples\src\ /s

echo "Linking to external libraries and examples for 32-bit release"
pushd cpp\Windows_32
mklink /j examples ..\examples
mklink /j external ..\external
popd

echo "Clearing temporary 32bit librares"
rd /s /q .\ReleaseShared32
rd /s /q .\ReleaseShared32TLS
rd /s /q .\ReleaseStatic32
rd /s /q .\ReleaseStatic32TLS

echo "Compiling Static 64bit library"
mkdir ReleaseStatic64
cd .\ReleaseStatic64
cd;
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=64;Platform=x64;PlatformTarget=x64
cd ..

echo "Compiling Static 64bit library with TLS support"
mkdir ReleaseStatic64TLS
cd .\ReleaseStatic64TLS
cd;
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=64;Platform=x64;PlatformTarget=x64
cd ..

echo "Compiling Shared 64bit library"
mkdir ReleaseShared64
cd .\ReleaseShared64
cd;
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=64;Platform=x64;PlatformTarget=x64
cd ..

echo "Compiling Shared 64bit library with TLS support"
mkdir ReleaseShared64TLS
cd .\ReleaseShared64TLS
cd;
cmake .. -G "Visual Studio 12 Win64" -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release  -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
MSBuild.exe HazelcastClient.sln /m /property:Configuration=Release /p:VisualStudioVersion=12.0;Flavor=64;Platform=x64;PlatformTarget=x64
cd ..

echo "Moving 64bit libraries to target"
mkdir .\cpp\Windows_64\hazelcast\lib\static
mkdir .\cpp\Windows_64\hazelcast\lib\tls\static
mkdir .\cpp\Windows_64\hazelcast\lib\shared
mkdir .\cpp\Windows_64\hazelcast\lib\tls\shared
mkdir .\cpp\Windows_64\hazelcast\include\hazelcast

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\

copy ReleaseShared64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\shared\
copy ReleaseShared64TLS\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\tls\shared\
copy ReleaseStatic64\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\static\
copy ReleaseStatic64TLS\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\tls\static\

echo "Linking to external libraries and examples for 64-bit release"
pushd cpp\Windows_64
mklink /j examples ..\examples
mklink /j external ..\external
popd

echo "Clearing temporary 64bit librares"
rd /s /q ./ReleaseShared64
rd /s /q ./ReleaseShared64TLS
rd /s /q ./ReleaseStatic64
rd /s /q ./ReleaseStatic64TLS

@REM Verify release
call scripts/verifyReleaseWindows.bat || exit /b 1
