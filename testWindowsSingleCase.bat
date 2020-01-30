@SET HZ_BIT_VERSION=%1
@SET HZ_LIB_TYPE=%2
@SET HZ_BUILD_TYPE=%3
@SET COMPILE_WITHOUT_SSL=%4

@SET BUILD_DIR=build%HZ_LIB_TYPE%%HZ_BIT_VERSION%%HZ_BUILD_TYPE%

@SET EXECUTABLE_NAME=clientTest_%HZ_LIB_TYPE%_%HZ_BIT_VERSION%.exe

if %HZ_BIT_VERSION% == 32 (
    @SET HZ_OPENSSL_INCLUDE_DIR=C:\OpenSSL-Win64\include
    @SET HZ_OPENSSL_LIB_DIR=C:\OpenSSL-Win32\lib
    set PYTHON_LIB_DIR=C:\Python-2.7.14\PCbuild
) else (
    set SOLUTIONTYPE="Visual Studio 12 Win64"
    @SET HZ_OPENSSL_INCLUDE_DIR=C:\OpenSSL-Win64\include
    @SET HZ_OPENSSL_LIB_DIR=C:\OpenSSL-Win64\lib
    set PYTHON_LIB_DIR=C:\Python-2.7.14\PCbuild\amd64
)

if %HZ_BUILD_TYPE% == Debug (
    set PYTHON_LIB_FILE_NAME=python27_d.lib
) else (
    set PYTHON_LIB_FILE_NAME=python27.lib
)

set PYTHON_LIBRARY_PATH=%PYTHON_LIB_DIR%\%PYTHON_LIB_FILE_NAME%

call scripts/build-windows.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE% %COMPILE_WITHOUT_SSL% || (
    echo "Failed to build the project!"
    exit /b 1
)

call scripts/start-rc.bat || (
    echo "Failed to start the remote controller"
    exit /b 1
)

SET DEFAULT_TIMEOUT=300
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
