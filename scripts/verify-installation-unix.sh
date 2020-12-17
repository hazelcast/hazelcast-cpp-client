#!/bin/sh

# Builds the examples using an installed library
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BUILD_DIR : build directory
#
# Command line arguments are forwarded to CMake.
#

# exit if a command returns non-zero status
set -e

# remove the given build directory if already exists
if [ -d "$BUILD_DIR" ]; then
  echo "Given build directory $BUILD_DIR exists. Removing for a clean build."
  rm -rf $BUILD_DIR
fi

echo "Configuring..."
cmake -S ./examples -B "$BUILD_DIR" "$@"

echo "Building..."
cmake --build "$BUILD_DIR" --verbose --parallel 8

