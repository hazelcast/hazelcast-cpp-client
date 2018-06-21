echo "Cleanup release directories"
for /d %%G in ("Release*") do rd /s /q "%%G"
rd /s /q cpp

mkdir .\cpp\Windows_32\hazelcast\lib\static
mkdir .\cpp\Windows_32\hazelcast\lib\tls\static
mkdir .\cpp\Windows_32\hazelcast\lib\shared
mkdir .\cpp\Windows_32\hazelcast\lib\tls\shared
mkdir .\cpp\Windows_32\hazelcast\include\hazelcast\

mkdir .\cpp\Windows_64\hazelcast\lib\static
mkdir .\cpp\Windows_64\hazelcast\lib\tls\static
mkdir .\cpp\Windows_64\hazelcast\lib\shared
mkdir .\cpp\Windows_64\hazelcast\lib\tls\shared
mkdir .\cpp\Windows_64\hazelcast\include\hazelcast

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\include\hazelcast\* cpp\Windows_32\hazelcast\include\hazelcast\

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\

call scripts\build-windows.bat 32 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\static\
rd /s /q buildSTATIC32Release

call scripts\build-windows.bat 32 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\shared\
rd /s /q buildSHARED32Release

call scripts\build-windows.bat 64 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\static\
rd /s /q buildSTATIC64Release

call scripts\build-windows.bat 64 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\shared\
rd /s /q buildSHARED64Release

echo "Generating TLS enabled libraries"
call scripts\build-windows.bat 32 STATIC Release || exit /b 1
copy buildSTATIC32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\tls\static\
rd /s /q buildSTATIC32Release

call scripts\build-windows.bat 32 SHARED Release || exit /b 1
copy buildSHARED32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\tls\shared\
rd /s /q buildSHARED32Release

call scripts\build-windows.bat 64 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\tls\static\
rd /s /q buildSTATIC64Release

call scripts\build-windows.bat 64 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\tls\shared\
rd /s /q buildSHARED64Release

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

echo "Linking to external libraries and examples for 64-bit release"
pushd cpp\Windows_64
mklink /j examples ..\examples
mklink /j external ..\external
popd

@REM Verify release
call scripts/verifyReleaseWindows.bat || exit /b 1
