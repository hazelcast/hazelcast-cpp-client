#!/bin/bash

# Installs Boost from source
# usage: ./install-boost.sh <version>

set -o errexit ${RUNNER_DEBUG:+-x}

if [[ "$#" -ne 1 ]]; then
    echo "usage: $0 <version>"
    exit 1
fi

TARBALL_NAME=boost_$(echo "$1" | tr . _)

curl --fail --silent --show-error --location "https://archives.boost.io/release/${1}/source/${TARBALL_NAME}.tar.gz" | tar xzf -
pushd "${TARBALL_NAME}"
./bootstrap.sh
# Build the libs for:
# - Thread - https://www.boost.org/libs/thread/
# - Chrono - https://www.boost.org/libs/chrono/
# - Atomic - https://www.boost.org/libs/atomic/
# Used the c++11 language level intentionally to support old boost versions.
./b2 -d0 cxxflags="-std=c++11 -Wno-enum-constexpr-conversion" --with-thread --with-chrono --with-atomic install
popd
