#!/bin/bash

# Installs Boost from source
# usage: ./install-boost.sh <version>

set -e

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <version>"
    exit 1
fi

TARBALL_NAME=boost_$(echo "$1" | tr . _)

curl --silent -Lo $TARBALL_NAME.tar.gz https://dl.bintray.com/boostorg/release/$1/source/$TARBALL_NAME.tar.gz
tar xzf $TARBALL_NAME.tar.gz
cd $TARBALL_NAME
./bootstrap.sh
./b2 --with-thread --with-chrono install
cd ..
