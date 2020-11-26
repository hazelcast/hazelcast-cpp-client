SET HZ_INSTALL_DIR=%1
SET HZ_VERSION="4.0-SNAPSHOT"
SET HZ_BIT_VERSION=%2
SET HZ_LIB_TYPE=%3

echo "Verifying the release located at %HZ_INSTALL_DIR% for version %HZ_VERSION% %HZ_LIB_TYPE% %HZ_BIT_VERSION%-bit library."

cd examples
rd /s /q build
mkdir build
cd build

SET HZ_BUILD_TYPE=Release
set VCPKG_ROOT=C:\\vcpkg

set SOLUTIONTYPE="Visual Studio 16 2019"

if %HZ_BIT_VERSION% == 32 (
    set BUILDFORPLATFORM="win32"
) else (
    set BUILDFORPLATFORM="x64"
)

echo "Generating the solution files for compilation without TLS"
cmake .. -G %SOLUTIONTYPE% -A %BUILDFORPLATFORM% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake -DCMAKE_CONFIGURATION_TYPES=%HZ_BUILD_TYPE% -DHZ_LIB_TYPE=%HZ_LIB_TYPE% -DHZ_BIT=%HZ_BIT_VERSION% -DCMAKE_BUILD_TYPE=%HZ_BUILD_TYPE% -DHZ_BUILD_TESTS=ON -DBUILD_GMOCK=OFF -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=OFF -DBUILD_GMOCK=OFF -DINSTALL_GTEST=OFF || exit /b 1

echo "Building for platform %BUILDFORPLATFORM%"
cmake --build . --parallel -v --config %HZ_BUILD_TYPE%

# clean the directory for a fresh build
cd ..
rd /s /q build
mkdir build
cd build

echo "Generating the solution files for compilation with TLS support"
cmake .. -G %SOLUTIONTYPE% -A %BUILDFORPLATFORM% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\\scripts\\buildsystems\\vcpkg.cmake -DCMAKE_CONFIGURATION_TYPES=%HZ_BUILD_TYPE% -DHZ_LIB_TYPE=%HZ_LIB_TYPE% -DHZ_BIT=%HZ_BIT_VERSION% -DCMAKE_BUILD_TYPE=%HZ_BUILD_TYPE% -DHZ_BUILD_TESTS=ON -DBUILD_GMOCK=OFF -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON -DBUILD_GMOCK=OFF -DINSTALL_GTEST=OFF || exit /b 1

echo "Building for platform %BUILDFORPLATFORM%"
cmake --build . --parallel -v --config %HZ_BUILD_TYPE%

echo "Verification of the release located at %HZ_INSTALL_DIR% for version %HZ_VERSION% %HZ_LIB_TYPE% %HZ_BIT_VERSION%-bit library is finished."

exit 0


