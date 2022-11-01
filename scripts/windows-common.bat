@REM Sets Windows-spesific environment variables used in build-windows.bat and verify-installation.bat.
@REM This is a helper script and should not be run by itself.

if %BIT_VERSION% == 32 (
    @set PLATFORM="win32"
) else (
    @set PLATFORM="x64"
)
