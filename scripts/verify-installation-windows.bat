@REM Builds the examples using an installed library
@REM The script should be run from the project's root directory
@REM
@REM This environment variables are the parameters to this script:
@REM - BUILD_DIR : build directory
@REM - BIT_VERSION : target platform architecture (32 or 64)
@REM - BUILD_CONFIGURATION : config to use when building (Release, Debug, etc.)
@REM
@REM Command line arguments are forwarded to CMake.
@REM

@call .\scripts\windows-common.bat

REM remove the given build directory if already exists
@rd /s /q %BUILD_DIR%
@mkdir %BUILD_DIR%

echo "Configuring..."
cmake -S .\examples -B %BUILD_DIR% ^
      -G %SOLUTION_TYPE% -A %PLATFORM% ^
      -DCMAKE_CONFIGURATION_TYPES=%BUILD_CONFIGURATION%  ^
      -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
      %* ^
      || exit /b 1

echo "Building..."
cmake --build %BUILD_DIR% --verbose --parallel --config %BUILD_CONFIGURATION% || exit /b 1
