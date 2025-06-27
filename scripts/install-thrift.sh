#!/bin/bash

# Installs Apache Thrift from source
# usage ./install-thrift.sh <version>

set -o errexit ${RUNNER_DEBUG:+-x}

if [[ "$#" -ne 1 ]]; then
    echo "usage: $0 <version>"
    exit 1
fi

curl --fail --silent --show-error --location https://archive.apache.org/dist/thrift/$1/thrift-$1.tar.gz  | tar xzf -
pushd thrift-$1/build
cmake .. -DBUILD_COMPILER=OFF -DBUILD_TESTING=OFF -DBUILD_TUTORIALS=OFF -DBUILD_LIBRARIES=ON \
         -DBUILD_CPP=ON -DBUILD_AS3=OFF -DBUILD_C_GLIB=OFF -DBUILD_JAVA=OFF -DBUILD_PYTHON=OFF \
         -DBUILD_HASKELL=OFF -DWITH_OPENSSL=OFF -DWITH_LIBEVENT=OFF -DWITH_ZLIB=OFF \
         -DWITH_QT5=OFF
cmake --build . --target install
popd
