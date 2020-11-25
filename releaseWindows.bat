echo "Cleanup release directories"
for /d %%G in ("Release*") do rd /s /q "%%G"
rd /s /q cpp

mkdir .\cpp\Windows_32\lib\static
mkdir .\cpp\Windows_32\lib\tls\static
mkdir .\cpp\Windows_32\lib\shared
mkdir .\cpp\Windows_32\lib\tls\shared
mkdir .\cpp\include\hazelcast\

mkdir .\cpp\Windows_64\lib\static
mkdir .\cpp\Windows_64\lib\tls\static
mkdir .\cpp\Windows_64\lib\shared
mkdir .\cpp\Windows_64\lib\tls\shared

xcopy /S /Q hazelcast\include\hazelcast\* cpp\include\hazelcast\
xcopy /S /Q hazelcast\generated-sources\src\hazelcast\client\protocol\codec\*.h cpp\include\hazelcast\client\protocol\codec\

echo "Copying the examples"
mkdir cpp\examples
xcopy examples\* cpp\examples\ /s

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

call scripts\build-windows.bat 32 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC32Release\Release\HazelcastClient*  cpp\Windows_32\lib\static\
rd /s /q buildSTATIC32Release

call scripts\build-windows.bat 32 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED32Release\Release\HazelcastClient*  cpp\Windows_32\lib\shared\
rd /s /q buildSHARED32Release

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

call scripts\build-windows.bat 64 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC64Release\Release\HazelcastClient*  cpp\Windows_64\lib\static\
rd /s /q buildSTATIC64Release

call scripts\build-windows.bat 64 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED64Release\Release\HazelcastClient*  cpp\Windows_64\lib\shared\
rd /s /q buildSHARED64Release

echo "Generating TLS enabled libraries"

SET CURRENT_DIRECTORY=%cd%

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

call scripts\build-windows.bat 32 STATIC Release || exit /b 1
copy buildSTATIC32Release\Release\HazelcastClient*  cpp\Windows_32\lib\tls\static\
rd /s /q buildSTATIC32Release

call scripts\build-windows.bat 32 SHARED Release || exit /b 1
copy buildSHARED32Release\Release\HazelcastClient*  cpp\Windows_32\lib\tls\shared\
rd /s /q buildSHARED32Release

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 SHARED || exit /b 1

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

call scripts\build-windows.bat 64 STATIC Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSTATIC64Release\Release\HazelcastClient*  cpp\Windows_64\lib\tls\static\
rd /s /q buildSTATIC64Release

call scripts\build-windows.bat 64 SHARED Release COMPILE_WITHOUT_SSL || exit /b 1
copy buildSHARED64Release\Release\HazelcastClient*  cpp\Windows_64\lib\tls\shared\
rd /s /q buildSHARED64Release

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 SHARED || exit /b 1

