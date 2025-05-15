#!/bin/bash

# Builds the library using CMake on UNIX-like platforms
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BUILD_DIR : build directory
# - BIT_VERSION : target platform architecture (32 or 64)
# - COVERAGE : add compiler flags necessary for test coverage (set to ON)
# - INSTALL : install after the build finishes (set to ON)
# - CXXFLAGS : additional compiler flags
# - BUILD_TYPE : Debug or Release
#
# Command line arguments are forwarded to CMake.
#

# exit if a command returns non-zero status
set -e

# add compiler flags necessary for test coverage
if [ "$COVERAGE" = "ON" ]; then
  CXXFLAGS="$CXXFLAGS -fprofile-arcs -fprofile-update=atomic -ftest-coverage -fPIC -O0"
fi

# set -m32 or -m64 if a BIT_VERSION is given
if [ -n "$BIT_VERSION" ]; then
  CXXFLAGS="$CXXFLAGS -m$BIT_VERSION"
fi

# enable all compiler warnings
CXXFLAGS="$CXXFLAGS -Wall"

if [ "${BUILD_TYPE,,}" = "debug" ]; then
  # treat compiler warnings as errors when the build type is Debug
  CXXFLAGS="$CXXFLAGS -Werror"
  # enable address sanitizer to provide meaningful stack traces
  CXXFLAGS="$CXXFLAGS -fsanitize=address -fno-omit-frame-pointer"
fi

# remove the given build directory if already exists
if [ -d "$BUILD_DIR" ]; then
  echo "Given build directory $BUILD_DIR exists. Removing for a clean build."
  rm -rf $BUILD_DIR
fi

# print variables for debugging
echo "BUILD_DIR       = $BUILD_DIR"
echo "BIT_VERSION     = $BIT_VERSION"
echo "COVERAGE        = $COVERAGE"
echo "INSTALL         = $INSTALL"
echo "CXXFLAGS        = $CXXFLAGS"
echo "BUILD_TYPE      = $BUILD_TYPE"
echo "CMake arguments = $@"

# export flags variable to be used by CMake
export CXXFLAGS

SOURCE_DIR=$(pwd)

mkdir $BUILD_DIR
cd $BUILD_DIR

echo "Configuring..."
cmake $SOURCE_DIR ${BUILD_TYPE:+-DCMAKE_BUILD_TYPE=${BUILD_TYPE}} "$@"

echo "Building..."
VERBOSE=1 cmake --build .

if [ "$INSTALL" = "ON" ]; then
  echo "Installing..."
  cmake --build . --target install
fi
