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

set DESTINATION=%cd%\destination

@REM set BUILD_SHARED_LIBS depending on LIBRARY_TYPE
set BUILD_SHARED_LIBS=ON
if "%LIBRARY_TYPE%" == "STATIC" (
    set BUILD_SHARED_LIBS=OFF
)

call .\scripts\build-windows.bat                                         ^
        -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ^
        -DCMAKE_INSTALL_PREFIX=%DESTINATION%                             ^
        -DBUILD_SHARED_LIBS=%BUILD_SHARED_LIBS%                          ^
        -DWITH_OPENSSL=%WITH_OPENSSL%                                    ^
        -DBUILD_TESTS=ON                                                 ^
        -DBUILD_EXAMPLES=OFF                                             ^
    || exit /b 1

call .\scripts\test-windows.bat || exit /b 1

set BUILD_DIR=build-examples

call .\scripts\verify-installation-windows.bat                            ^
        -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake  ^
        -DCMAKE_PREFIX_PATH=%DESTINATION%                                 ^
        -DWITH_OPENSSL=%WITH_OPENSSL%                                     ^
    || exit /b 1
