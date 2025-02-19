#!/bin/bash

# Installs Boost from source
# usage: ./install-boost.sh <version>

set -o errexit ${RUNNER_DEBUG:+-x}

if [[ "$#" -ne 1 ]]; then
    echo "usage: $0 <version>"
    exit 1
fi

TARBALL_NAME=boost_$(echo "$1" | tr . _)

curl --silent --location "https://archives.boost.io/release/${1}/source/${TARBALL_NAME}.tar.gz" | tar xzf -
pushd "${TARBALL_NAME}"
./bootstrap.sh
./b2 --with-thread --with-chrono install
popd
