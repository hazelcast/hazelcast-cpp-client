@REM Runs the tests on Windows
@REM The script should be run from the project's root directory
@REM
@REM This environment variables are the parameters to this script:
@REM - BUILD_DIR : build directory
@REM - BUILD_CONFIGURATION : config to use when building (Release, Debug, etc.)
@REM

call scripts/start-rc.bat || (
    echo "Failed to start the remote controller"
    exit /b 1
)

set RC_START_TIMEOUT_IN_SECS=300
set RC_PORT=9701

set TEST_EXECUTABLE=%BUILD_DIR%\hazelcast\test\src\%BUILD_CONFIGURATION%\client_test.exe


set timeout=%RC_START_TIMEOUT_IN_SECS%

echo "Waiting for the test server to start. Timeout: %timeout% seconds"

:loop
    netstat -an  | findstr /C:":%RC_PORT% "
    if %errorlevel% == 0 (
        set /a remainingTime = RC_START_TIMEOUT_IN_SECS - timeout
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
echo "The test server did not start in %RC_START_TIMEOUT_IN_SECS% seconds. Test FAILED."
call taskkill /F /FI "WINDOWTITLE eq hazelcast-remote-controller"
exit /b 1

:server_started

echo "Starting the client test now."

set PATH=%BUILD_DIR%\%BUILD_CONFIGURATION%;%BUILD_DIR%\bin\%BUILD_CONFIGURATION%;%PATH%

echo %TEST_EXECUTABLE%
%TEST_EXECUTABLE% --gtest_output="xml:CPP_Client_Test_Report.xml" --gtest_filter=-*ReplicatedMap*:*AwsClient*:*ClientMapTest*
set result=%errorlevel%

taskkill /T /F /FI "WINDOWTITLE eq hazelcast-remote-controller"

exit /b %result%
