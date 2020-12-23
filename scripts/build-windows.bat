REM Builds the library using CMake on Windows
REM The script should be run from the project's root directory
REM This environment variables are the parameters to this script:
REM - BUILD_DIR : build directory
REM - BIT_VERSION : target platform architecture (32 or 64)
REM - INSTALL : install after the build finishes (set to ON)
REM - CXXFLAGS : additional compiler flags
REM - BUILD_CONFIGURATION : config to use when building (Release, Debug, etc.)
REM
REM Command line arguments are forwarded to CMake.
REM

@set VCPKG_ROOT=C:\dev\vcpkg
@set SOLUTION_TYPE="Visual Studio 16 2019"

if %BIT_VERSION% == 32 (
    @set PLATFORM="win32"
) else (
    @set PLATFORM="x64"
)

REM remove the given build directory if already exists
@rd /s /q %BUILD_DIR%
@mkdir %BUILD_DIR%

REM print variables for debugging
@echo SOLUTION_TYPE = %SOLUTION_TYPE%
@echo PLATFORM      = %PLATFORM%
@echo BUILD_DIR     = %BUILD_DIR%


@echo Configuring...
cmake -S . -B %BUILD_DIR% ^
      -G %SOLUTION_TYPE% -A %PLATFORM% ^
      -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
      %* ^
      || exit /b 1 

@echo Building...
cmake --build %BUILD_DIR% --verbose --parallel --config %BUILD_CONFIGURATION% || exit /b 1


if "%INSTALL%" == "ON" (
    @echo Installing...
    cmake --install %BUILD_DIR% || exit /b 1
)


exit /b 0
