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

# set BUILD_SHARED_LIBS depending on LIBRARY_TYPE
BUILD_SHARED_LIBS=ON
if [ "$LIBRARY_TYPE" == "STATIC" ]; then
  BUILD_SHARED_LIBS=OFF;
fi

./scripts/build-unix.sh                      \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE           \
    -DCMAKE_INSTALL_PREFIX=$DESTINATION      \
    -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS     \
    -DWITH_OPENSSL=$WITH_OPENSSL             \
    -DBUILD_TESTS=ON                         \
    -DBUILD_EXAMPLES=OFF

./scripts/test-unix.sh

if [ "$COVERAGE" == "ON" ]; then
  gcovr --xml-pretty -o cpp_coverage.xml                    \
        --delete                                            \
        -r "$(pwd)/hazelcast" -e "$(pwd)/hazelcast/test"    \
        "$(pwd)/$BUILD_DIR"
fi

export BUILD_DIR=build-examples

./scripts/verify-installation-unix.sh            \
  -DCMAKE_PREFIX_PATH=$DESTINATION -DWITH_OPENSSL=$WITH_OPENSSL
