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
xcopy /S /Q hazelcast\generated-sources\src\hazelcast\client\protocol\codec\*.h cpp\Windows_32\hazelcast\include\hazelcast\client\protocol\codec\

xcopy /S /Q hazelcast\include\hazelcast\* cpp\Windows_64\hazelcast\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\src\hazelcast\client\protocol\codec\*.h cpp\Windows_64\hazelcast\include\hazelcast\client\protocol\codec\

echo "Copying the examples"
mkdir cpp\examples\src
xcopy examples\* cpp\examples\src\ /s

echo "Linking to examples for 32-bit release"
pushd cpp\Windows_32
mklink /j examples ..\examples
popd

echo "Linking to examples for 64-bit release"
pushd cpp\Windows_64
mklink /j examples ..\examples
popd

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

call scripts\build-windows.bat 32 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\static\
rd /s /q buildSTATIC32Release

call scripts\build-windows.bat 32 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\shared\
rd /s /q buildSHARED32Release

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

call scripts\build-windows.bat 64 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\static\
rd /s /q buildSTATIC64Release

call scripts\build-windows.bat 64 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\shared\
rd /s /q buildSHARED64Release

echo "Generating TLS enabled libraries"

SET CURRENT_DIRECTORY=%cd%

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

call scripts\build-windows.bat 32 STATIC Release || exit /b 1
copy buildSTATIC32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\tls\static\
rd /s /q buildSTATIC32Release

call scripts\build-windows.bat 32 SHARED Release || exit /b 1
copy buildSHARED32Release\Release\HazelcastClient*  cpp\Windows_32\hazelcast\lib\tls\shared\
rd /s /q buildSHARED32Release

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 SHARED || exit /b 1

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

call scripts\build-windows.bat 64 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\tls\static\
rd /s /q buildSTATIC64Release

call scripts\build-windows.bat 64 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED64Release\Release\HazelcastClient*  cpp\Windows_64\hazelcast\lib\tls\shared\
rd /s /q buildSHARED64Release

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 SHARED || exit /b 1

