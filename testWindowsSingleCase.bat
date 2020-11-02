@SET HZ_BIT_VERSION=%1
@SET HZ_LIB_TYPE=%2
@SET HZ_BUILD_TYPE=%3
@SET COMPILE_WITHOUT_SSL=%4

@SET BUILD_DIR=build%HZ_LIB_TYPE%%HZ_BIT_VERSION%%HZ_BUILD_TYPE%

@SET EXECUTABLE_NAME=clientTest_%HZ_LIB_TYPE%_%HZ_BIT_VERSION%.exe

if %HZ_BIT_VERSION% == 32 (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
) else (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)

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

SET PATH=%BUILD_DIR%\%HZ_BUILD_TYPE%;%PATH%

%BUILD_DIR%\hazelcast\test\src\%HZ_BUILD_TYPE%\%EXECUTABLE_NAME% --gtest_output="xml:CPP_Client_Test_Report.xml"
set result=%errorlevel%

taskkill /T /F /FI "WINDOWTITLE eq hazelcast-remote-controller"

exit /b %result%
