@REM Sets Windows-spesific environment variables used in build-windows.bat and verify-installation.bat.
@REM This is a helper script and should not be run by itself.

@set VCPKG_ROOT=C:\vcpkg
@set SOLUTION_TYPE="Visual Studio 16 2019"

if %BIT_VERSION% == 32 (
    @set PLATFORM="win32"
) else (
    @set PLATFORM="x64"
)
