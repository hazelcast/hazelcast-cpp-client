#!/bin/bash

# Builds the library using CMake on UNIX-like platforms
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BUILD_DIR : build directory
# - BIT_VERSION : target platform architecture (32 or 64)
# - COVERAGE : add compiler flags necessary for test coverage (set to ON)
# - INSTALL : install after the build finishes (set to ON)
# - WARN_AS_ERR : treat compiler warnings as errors (set to ON)
# - CXXFLAGS : additional compiler flags
#
# Command line arguments are forwarded to CMake.
#

# exit if a command returns non-zero status
set -e

# add compiler flags necessary for test coverage
if [ "$COVERAGE" = "ON" ]; then
  CXXFLAGS="$CXXFLAGS -fprofile-arcs -ftest-coverage -fPIC -O0"
fi

# set -m32 or -m64 if a BIT_VERSION is given
if [ -n "$BIT_VERSION" ]; then
  CXXFLAGS="$CXXFLAGS -m$BIT_VERSION"
fi

# enable all compiler warnings
CXXFLAGS="$CXXFLAGS -Wall"

if [ "$WARN_AS_ERR" = "ON" ]; then
  CXXFLAGS="$CXXFLAGS -Werror"
fi

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

# export flags variable to be used by CMake
export CXXFLAGS

SOURCE_DIR=$(pwd)

mkdir $BUILD_DIR
cd $BUILD_DIR

echo "Configuring..."
cmake $SOURCE_DIR "$@"

echo "Building..."
VERBOSE=1 cmake --build .

if [ "$INSTALL" = "ON" ]; then
  echo "Installing..."
  cmake --build . --target install
fi
