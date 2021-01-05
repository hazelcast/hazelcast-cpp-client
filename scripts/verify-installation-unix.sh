#!/bin/sh

# Builds the examples using an installed library
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BUILD_DIR : build directory
# - BIT_VERSION : target platform architecture (32 or 64)
# - CXXFLAGS : additional compiler flags
#
# Command line arguments are forwarded to CMake.
#

# exit if a command returns non-zero status
set -e

# set -m32 or -m64 if a BIT_VERSION is given
if [ -n "$BIT_VERSION" ]; then
  CXXFLAGS="$CXXFLAGS -m$BIT_VERSION"
fi

# remove the given build directory if already exists
if [ -d "$BUILD_DIR" ]; then
  echo "Given build directory $BUILD_DIR exists. Removing for a clean build."
  rm -rf $BUILD_DIR
fi

# export flags variable to be used by CMake
export CXXFLAGS

echo "Configuring..."
cmake -S ./examples -B "$BUILD_DIR" "$@"

echo "Building..."
cmake --build "$BUILD_DIR" --verbose --parallel 8

