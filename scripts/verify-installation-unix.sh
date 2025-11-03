#!/bin/sh

# Builds the examples using an installed library
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BUILD_DIR : build directory
# - BUILD_TYPE : build type (Release, Debug, etc.)
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

if [ "${BUILD_TYPE}" = "Debug" ]; then
  # treat compiler warnings as errors when the build type is Debug
  CXXFLAGS="$CXXFLAGS -Werror"
  # enable address sanitizer to provide meaningful stack traces
  CXXFLAGS="$CXXFLAGS -fsanitize=address -fno-omit-frame-pointer"
fi

# treat compiler warnings as errors when the build type is Debug
CXXFLAGS="$CXXFLAGS -Werror"
# enable address sanitizer to provide meaningful stack traces
CXXFLAGS="$CXXFLAGS -fsanitize=address -fno-omit-frame-pointer"

# export flags variable to be used by CMake
export CXXFLAGS

# remove the given build directory if already exists
if [ -d "$BUILD_DIR" ]; then
  echo "Given build directory $BUILD_DIR exists. Removing for a clean build."
  rm -rf $BUILD_DIR
fi

SOURCE_DIR=$(pwd)/examples

# print variables for debugging
echo "BUILD_DIR       = $BUILD_DIR"
echo "BUILD_TYPE       = BUILD_TYPE"
echo "$SOURCE_DIR     = $SOURCE_DIR"
echo "BIT_VERSION     = $BIT_VERSION"
echo "CXXFLAGS        = $CXXFLAGS"
echo "CMake arguments = $@"

mkdir $BUILD_DIR
cd $BUILD_DIR

echo "Configuring... with cmake parameters: cmake $SOURCE_DIR $@"
cmake $SOURCE_DIR -DCMAKE_BUILD_TYPE=${BUILD_TYPE} "$@"

echo "Building..."
VERBOSE=1 cmake --build .

