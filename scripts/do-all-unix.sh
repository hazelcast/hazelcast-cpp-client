#!/bin/bash

# Builds, tests and installs the library, and verifies the installation on UNIX-like platforms
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BIT_VERSION : target platform architecture (32 or 64)
# - BUILD_TYPE : Release, Debug, etc.
# - LIBRARY_TYPE : SHARED or STATIC
# - WITH_OPENSSL : ON or OFF
# - COVERAGE: ON or OFF
#

# exit if a command returns non-zero status
set -e

export BUILD_DIR=build
export INSTALL=ON

# treat compiler warnings as errors when the build type is Debug
if [ "$BUILD_TYPE" == "Debug" ]; then
  export WARN_AS_ERR=ON
fi

DESTINATION=$(pwd)/destination

# set BUILD_STATIC_LIB and BUILD_SHARED_LIB depending on LIBRARY_TYPE
BUILD_STATIC_LIB=OFF
BUILD_SHARED_LIB=OFF
if [ "$LIBRARY_TYPE" == "SHARED" ]; then
  BUILD_SHARED_LIB=ON;
elif [ "$LIBRARY_TYPE" == "STATIC" ]; then
  BUILD_STATIC_LIB=ON;
fi

./scripts/build-unix.sh                      \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE           \
    -DCMAKE_INSTALL_PREFIX=$DESTINATION      \
    -DBUILD_STATIC_LIB=$BUILD_STATIC_LIB     \
    -DBUILD_SHARED_LIB=$BUILD_SHARED_LIB     \
    -DWITH_OPENSSL=$WITH_OPENSSL             \
    -DBUILD_TESTS=ON                         \
    -DBUILD_EXAMPLES=OFF

./scripts/test-unix.sh

if [ "$COVERAGE" == "ON" ]; then
  gcovr --xml-pretty -o cpp_coverage.xml -r .                                                  \
        -e ".*boost.*" -e ".*examples.*" -e ".*test.*" -e ".*usr.*include.*" -e ".*asio.*" -d
fi

export BUILD_DIR=build-examples

# compute the library name depending on the parameters
# hazelcastcxx, hazelcastcxx_ssl_static, hazecast_static etc.
LIBRARY_FOR_EXAMPLES="hazelcastcxx"
if [ "$WITH_OPENSSL" == "ON" ]; then
  LIBRARY_FOR_EXAMPLES="${LIBRARY_FOR_EXAMPLES}_ssl"
fi
if [ "$LIBRARY_TYPE" == "STATIC" ]; then
  LIBRARY_FOR_EXAMPLES="${LIBRARY_FOR_EXAMPLES}_static"
fi

./scripts/verify-installation-unix.sh            \
  -DCMAKE_PREFIX_PATH=$DESTINATION               \
  -DLIBRARY_FOR_EXAMPLES=$LIBRARY_FOR_EXAMPLES
