#!/bin/bash

# Builds the library using CMake on UNIX-like platforms
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BUILD_DIR : build directory
# - BIT_VERSION : target platform architecture (32 or 64)
# - COVERAGE : add compiler flags necessary for test coverage (set to ON)
# - TEST : run tests (set to ON)
# - INSTALL : install after the build finishes (set to ON)
# - CXXFLAGS : additional compiler flags
#
# Command line arguments are forwarded to CMake.
#

# kill background processes on exit
trap "kill $(jobs -p)" EXIT

# exit if a command returns non-zero status
set -e

# add test coverage flags
if [ "$COVERAGE" = "ON" ]; then
  CXXFLAGS="${CXXFLAGS} -fprofile-arcs -ftest-coverage -fPIC -O0"
fi

# set -m32 or -m64 if a BIT_VERSION is given
if [ -n "$BIT_VERSION" ]; then
  CXXFLAGS="${CXXFLAGS} -m${BIT_VERSION}"
fi

# enable all compiler warnings and treat them as errors
CXXFLAGS="${CXXFLAGS} -Wall -Werror"

# remove the given build directory if already exists
if [ -d "$BUILD_DIR" ]; then
  echo "Given build directory $BUILD_DIR exists. Removing for a clean build."
  rm -rf $BUILD_DIR
fi

# print variables for debugging
echo "BUILD_DIR   = $BUILD_DIR"
echo "BIT_VERSION = $BIT_VERSION"
echo "COVERAGE    = $COVERAGE"
echo "INSTALL     = $INSTALL"
echo "CXXFLAGS    = $CXXFLAGS"

echo "Configuring..."
cmake -S . -B "$BUILD_DIR" "$@"

echo "Building..."
cmake --build "$BUILD_DIR" --verbose --parallel 8

if [ "$TEST" = "ON" ]; then
  echo "Testing..."

  TEST_EXECUTABLE="$BUILD_DIR/hazelcast/test/src/client_test"

  if [ ! -f "$TEST_EXECUTABLE" ]; then
    echo "Test executable cannot be found, make sure tests are configured to be built!"
    exit 1
  fi

  ./scripts/start-rc.sh &
  while [ -z "$(netstat -an | grep 9701)" ]; do
    sleep 1;
  done

  $TEST_EXECUTABLE --gtest_output="xml:test_report.xml" --gtest_filter="*log*"
fi

if [ "$INSTALL" = "ON" ]; then
  echo "Installing..."
  cmake --install "$BUILD_DIR"
fi


