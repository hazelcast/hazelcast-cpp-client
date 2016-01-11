SET HZ_BIT_VERSION=32
SET HZ_LIB_TYPE=SHARED
SET HZ_BUILD_TYPE=Debug

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

SET HZ_BIT_VERSION=32
SET HZ_LIB_TYPE=SHARED
SET HZ_BUILD_TYPE=Release

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

SET HZ_BIT_VERSION=32
SET HZ_LIB_TYPE=STATIC
SET HZ_BUILD_TYPE=Debug

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

SET HZ_BIT_VERSION=32
SET HZ_LIB_TYPE=STATIC
SET HZ_BUILD_TYPE=Release

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

SET HZ_BIT_VERSION=64
SET HZ_LIB_TYPE=STATIC
SET HZ_BUILD_TYPE=Debug

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

SET HZ_BIT_VERSION=64
SET HZ_LIB_TYPE=STATIC
SET HZ_BUILD_TYPE=Release

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

SET HZ_BIT_VERSION=64
SET HZ_LIB_TYPE=SHARED
SET HZ_BUILD_TYPE=Debug

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

SET HZ_BIT_VERSION=64
SET HZ_LIB_TYPE=SHARED
SET HZ_BUILD_TYPE=Release

call testWindowsSingleCase.bat %HZ_BIT_VERSION% %HZ_LIB_TYPE% %HZ_BUILD_TYPE%
if %errorlevel% NEQ 0 (
    goto test_failed
)

call releaseWindows.bat
if %errorlevel% NEQ 0 (
    echo "******  Test FAILED. The release script failed!!!"
    exit /b 1
)

echo "Cleaning up release directories"
rm -rf ./Release*
rm -rf ./cpp

echo "All Tests PASSED"
exit /b 0

:test_failed
echo "******  Test FAILED. Bit Version:%HZ_BIT_VERSION%, Library type:%HZ_LIB_TYPE%, Build Version:%HZ_BUILD_TYPE%"
exit /b 1
