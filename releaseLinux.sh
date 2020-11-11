#!/usr/bin/env bash

docker build -t fedora_32 -f docker/hazelcast-fedora-i386.dockerfile docker
docker build -t fedora_64 -f docker/hazelcast-fedora-x86_64.dockerfile docker

docker run --rm --name linux_32_bit_release_build -v `pwd`:/hazelcast-cpp-client fedora_32 /bin/bash -l -c "cd hazelcast-cpp-client; scripts/release_linux_for_version.sh 32"
if [ $? -ne 0 ]; then
    echo "32-bit build failed.!"
    exit 1
fi

docker run --rm --name linux_64_bit_release_build -v `pwd`:/hazelcast-cpp-client fedora_64 /bin/bash -l -c "cd hazelcast-cpp-client; scripts/release_linux_for_version.sh 64"
if [ $? -ne 0 ]; then
    echo "64-bit build failed.!"
    exit 2
fi




