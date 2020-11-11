REM Verify release
SET CURRENT_DIRECTORY=%cd%

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 32 SHARED || exit /b 1

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 STATIC || exit /b 1

call scripts\verifyReleaseWindowsSingleCase.bat "%CURRENT_DIRECTORY%\cpp" 64 SHARED || exit /b 1

exit 0


