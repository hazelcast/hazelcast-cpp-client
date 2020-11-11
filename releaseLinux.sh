#!/usr/bin/env bash

docker build -t fedora_32 -f docker/hazelcast-fedora-i386.dockerfile docker
docker build -t fedora_64 -f docker/hazelcast-fedora-x86_64.dockerfile docker

echo "Starting the docker build for 32-bit"
docker run -d --rm --name linux_32_bit_release_build -v `pwd`:/hazelcast-cpp-client fedora_32 /bin/bash -l -c "cd hazelcast-cpp-client; scripts/release_linux_for_version.sh 32"

echo "Starting the docker build for 64-bit"
docker run -d --rm --name linux_64_bit_release_build -v `pwd`:/hazelcast-cpp-client fedora_64 /bin/bash -l -c "cd hazelcast-cpp-client; scripts/release_linux_for_version.sh 64"

echo "Waiting for 32-bit docker build to finish"
docker wait linux_32_bit_release_build
if [ $? -ne 0 ]; then
    echo "32-bit build failed.!"
    exit 1
fi

echo "Waiting for 64-bit docker build to finish"
docker wait linux_64_bit_release_build
if [ $? -ne 0 ]; then
    echo "64-bit build failed.!"
    exit 1
fi




