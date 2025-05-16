@REM Builds the library using CMake on Windows
@REM The script should be run from the project's root directory
@REM This environment variables are the parameters to this script:
@REM - BUILD_DIR : build directory
@REM - BIT_VERSION : target platform architecture (32 or 64)
@REM - INSTALL : install after the build finishes (set to ON)
@REM - BUILD_TYPE : config to use when building (Release, Debug, etc.)
@REM - CXXFLAGS : additional compiler flags
@REM
@REM Command line arguments are forwarded to CMake.
@REM

@call .\scripts\windows-common.bat

REM print variables for debugging
@echo SOLUTION_TYPE = %SOLUTION_TYPE%
@echo PLATFORM      = %PLATFORM%
@echo BUILD_DIR     = %BUILD_DIR%
@echo BUILD_TYPE    = %BUILD_TYPE%

if "%BUILD_TYPE%"=="Debug" (
    REM Treat compiler warnings as errors when the build type is Debug
    set CXXFLAGS="%CXXFLAGS% -Werror"
    REM Enable address sanitizer to provide meaningful stack traces
    set CXXFLAGS="%CXXFLAGS% -fsanitize=address -fno-omit-frame-pointer"
)

REM remove the given build directory if already exists
@rd /s /q %BUILD_DIR%
@mkdir %BUILD_DIR%

@echo Configuring...
cmake -S . -B %BUILD_DIR% ^
      -A %PLATFORM% ^
      -DCMAKE_CONFIGURATION_TYPES=%BUILD_TYPE%  ^
      %* ^
      || exit /b 1 

@echo Building...
cmake --build %BUILD_DIR% --verbose --parallel --config %BUILD_TYPE% || exit /b 1


if "%INSTALL%" == "ON" (
    @echo Installing...
    cmake --install %BUILD_DIR% --config %BUILD_TYPE% || exit /b 1
)


exit /b 0
