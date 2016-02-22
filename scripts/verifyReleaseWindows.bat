REM Verify release
SET CURRENT_DIRECTORY=%cd%

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 SHARED || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 SHARED || exit /b 1

exit 0


