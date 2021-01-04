@REM Builds, tests and installs the library, and verifies the installation on Windows
@REM The script should be run from the project's root directory
@REM This environment variables are the parameters to this script:
@REM - BIT_VERSION : target platform architecture (32 or 64)
@REM - BUILD_TYPE : Release, Debug, etc.
@REM - LIBRARY_TYPE : SHARED or STATIC
@REM - WITH_OPENSSL : ON or OFF
@REM

set BUILD_DIR=build
set INSTALL=ON
set BUILD_CONFIGURATION=%BUILD_TYPE%

set DESTINATION=%cd%\destination

@REM set BUILD_STATIC_LIB and BUILD_SHARED_LIB depending on LIBRARY_TYPE
set BUILD_STATIC_LIB=OFF
set BUILD_SHARED_LIB=OFF
if "%LIBRARY_TYPE%" == "SHARED" (
    set BUILD_SHARED_LIB=ON
)
if "%LIBRARY_TYPE%" == "STATIC" (
    set BUILD_STATIC_LIB=ON
)

call .\scripts\build-windows.bat                  ^
        -DCMAKE_CONFIGURATION_TYPES=%BUILD_TYPE%  ^
        -DCMAKE_INSTALL_PREFIX=%DESTINATION%      ^
        -DBUILD_STATIC_LIB=%BUILD_STATIC_LIB%     ^
        -DBUILD_SHARED_LIB=%BUILD_SHARED_LIB%     ^
        -DWITH_OPENSSL=%WITH_OPENSSL%             ^
        -DBUILD_TESTS=ON                          ^
        -DBUILD_EXAMPLES=OFF                      ^
    || exit /b 1

call .\scripts\test-windows.bat || exit /b 1

set BUILD_DIR=build-examples

@REM compute the library name depending on the parameters
@REM hazelcastcxx, hazelcastcxx_ssl_static, hazecast_static etc.
set LIBRARY_FOR_EXAMPLES=hazelcastcxx
if "%WITH_OPENSSL%" == "ON" (
    set LIBRARY_FOR_EXAMPLES=%LIBRARY_FOR_EXAMPLES%_ssl
)
if "%LIBRARY_TYPE%" == "STATIC" (
    set LIBRARY_FOR_EXAMPLES=%LIBRARY_FOR_EXAMPLES%_static
)

call .\scripts\verify-installation-windows.bat     ^
	-DCMAKE_PREFIX_PATH=%DESTINATION%              ^
	-DLIBRARY_FOR_EXAMPLES=%LIBRARY_FOR_EXAMPLES%  ^
     || exit /b 1
