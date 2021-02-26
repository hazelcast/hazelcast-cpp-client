#!/bin/bash

# Installs Apache Thrift from source
# usage ./install-thrift.sh <version>

set -e

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <version>"
    exit 1
fi

curl --silent -Lo thrift-$1.tar.gz https://archive.apache.org/dist/thrift/$1/thrift-$1.tar.gz
tar xzf thrift-$1.tar.gz
rm thrift-$1.tar.gz
cd thrift-$1
./configure --disable-debug --disable-tests --disable-tutorial --disable-static \
            --without-as3 --without-erlang --without-haskell --without-java --without-perl \
            --without-php --without-php_extension --without-rs --without-python --without-ruby \
            --without-swift --without-csharp --without-dart --without-qt5 --without-c_glib \
            --without-nodejs --without-nodets --without-lua --without-py3 --without-ruby \
            --without-go --without-cl --without-haxe --without-dotnetcore --without-d \
            --without-zlib --without-libevent
make -j 2
make install
cd ..
rm -rf thrift-$1
