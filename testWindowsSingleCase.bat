@SET HZ_BIT_VERSION=%1
@SET HZ_LIB_TYPE=%2
@SET HZ_BUILD_TYPE=%3
@SET COMPILE_WITHOUT_SSL=%3

@SET BUILD_DIR=build%HZ_LIB_TYPE%%HZ_BIT_VERSION%%HZ_BUILD_TYPE%

@SET EXECUTABLE_NAME=clientTest_%HZ_LIB_TYPE%_%HZ_BIT_VERSION%.exe

@echo HZ_BIT_VERSION=%HZ_BIT_VERSION%
@echo HZ_LIB_TYPE=%HZ_LIB_TYPE%
@echo HZ_BUILD_TYPE=%HZ_BUILD_TYPE%
@echo BUILD_DIR=%BUILD_DIR%
@echo EXECUTABLE_NAME=%EXECUTABLE_NAME%

@REM Let the submodule code be downloaded
git submodule update --init

if %HZ_BIT_VERSION% == 32 (
    set BUILDFORPLATFORM="win32"
    set SOLUTIONTYPE="Visual Studio 12"
    @SET HZ_OPENSSL_INCLUDE_DIR=C:\OpenSSL-Win64\include
    @SET HZ_OPENSSL_LIB_DIR=C:\OpenSSL-Win32\lib
    set PYTHON_LIB_DIR=C:\Python-2.7.14\PCbuild
) else (
    set BUILDFORPLATFORM="x64"
    set SOLUTIONTYPE="Visual Studio 12 Win64"
    @SET HZ_OPENSSL_INCLUDE_DIR=C:\OpenSSL-Win64\include
    @SET HZ_OPENSSL_LIB_DIR=C:\OpenSSL-Win64\lib
    set PYTHON_LIB_DIR=C:\Python-2.7.14\PCbuild\amd64
)

if %COMPILE_WITHOUT_SSL% == "COMPILE_WITHOUT_SSL" (
    set HZ_COMPILE_WITH_SSL=OFF
) else (
    set HZ_COMPILE_WITH_SSL=ON
)

if %HZ_BUILD_TYPE% == Debug (
    set PYTHON_LIB_FILE_NAME=python27_d.lib
) else (
    set PYTHON_LIB_FILE_NAME=python27.lib
)

set PYTHON_LIBRARY_PATH=%PYTHON_LIB_DIR%\%PYTHON_LIB_FILE_NAME%

echo "Using Python library at %PYTHON_LIBRARY_PATH%"

RD /S /Q %BUILD_DIR%
mkdir %BUILD_DIR%

pushd %BUILD_DIR%

echo "Generating the solution files for compilation"
cmake .. -G %SOLUTIONTYPE% -DHZ_LIB_TYPE=%HZ_LIB_TYPE% -DHZ_BIT=%HZ_BIT_VERSION% -DCMAKE_BUILD_TYPE=%HZ_BUILD_TYPE% -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_OPENSSL_INCLUDE_DIR=%HZ_OPENSSL_INCLUDE_DIR% -DHZ_OPENSSL_LIB_DIR=%HZ_OPENSSL_LIB_DIR% -DHZ_COMPILE_WITH_SSL=%HZ_COMPILE_WITH_SSL% -DPYTHON_INCLUDE_DIR=C:\Python27\include -DPYTHON_LIBRARY=%PYTHON_LIBRARY_PATH%

echo "Building for platform %BUILDFORPLATFORM%"

MSBuild.exe HazelcastClient.sln /m /p:Flavor=%HZ_BUILD_TYPE%;Configuration=%HZ_BUILD_TYPE%;VisualStudioVersion=12.0;Platform=%BUILDFORPLATFORM%;PlatformTarget=%BUILDFORPLATFORM% /verbosity:n || exit /b 1

popd


pip install -r hazelcast/test/test_requirements.txt || (
    echo "Failed to install python hazelcast-remote-controller library."
    exit /b 1
)

call scripts/start-rc.bat || (
    echo "Failed to start the remote controller"
    exit /b 1
)

SET DEFAULT_TIMEOUT=30
SET SERVER_PORT=9701

SET timeout=%DEFAULT_TIMEOUT%

echo "Waiting for the test server to start. Timeout: %timeout% seconds"

:loop
    netstat -an  | findstr /C:":%SERVER_PORT% "
    if %errorlevel% == 0 (
        set /a remainingTime = DEFAULT_TIMEOUT - timeout
        echo "Server started in %remainingTime% seconds"
        goto server_started
    ) else (
        set /a timeout-=1

        if %timeout% gtr 0 (
            echo "Sleeping 1 second. Remaining %timeout% seconds"
            CHOICE /c x /D x /T 1 > NUL
            goto loop
        ) else (
            goto server_failed_to_start
        )
    )

:server_failed_to_start
echo "The test server did not start in %DEFAULT_TIMEOUT% seconds. Test FAILED."
call taskkill /F /FI "WINDOWTITLE eq hazelcast-remote-controller"
exit /b 1

:server_started

echo "Starting the client test now."

set PYTHONHOME=C:\Python27
set PYTHONPATH=%PYTHON_LIB_DIR%
SET PATH=%BUILD_DIR%\%HZ_BUILD_TYPE%;%PATH%
SET PATH=%PYTHON_LIB_DIR%;%PATH%

%BUILD_DIR%\hazelcast\test\src\%HZ_BUILD_TYPE%\%EXECUTABLE_NAME% --gtest_output="xml:CPP_Client_Test_Report.xml"
set result=%errorlevel%

taskkill /T /F /FI "WINDOWTITLE eq hazelcast-remote-controller"

exit /b %result%
