#!/usr/bin/env bash

function cleanup {
    echo "cleanup is being performed."
    echo "32-bit build logs:"
    docker logs linux_32_bit_release_build

    echo "64-bit build logs:"
    docker logs linux_64_bit_release_build

    echo "Removing docker container linux_32_bit_release_build"
    docker stop linux_32_bit_release_build
    docker rm linux_32_bit_release_build

    echo "Removing docker container linux_64_bit_release_build"
    docker stop linux_64_bit_release_build
    docker rm linux_64_bit_release_build

    exit
}

# Disables printing security sensitive data to the logs
set +x

trap cleanup EXIT

docker build -t fedora_32 -f docker/hazelcast-fedora-i386.dockerfile docker
docker build -t fedora_64 -f docker/hazelcast-fedora-x86_64.dockerfile docker

# cleanup containers if exists before start
docker stop linux_32_bit_release_build
docker rm linux_32_bit_release_build
docker stop linux_64_bit_release_build
docker rm linux_64_bit_release_build

echo "Starting the docker build for 32-bit"
docker run -d --rm --name linux_32_bit_release_build -v `pwd`:/hazelcast-cpp-client fedora_32 /bin/bash -l -c "cd hazelcast-cpp-client; scripts/release_linux_for_version.sh 32"

echo "Starting the docker build for 64-bit"
docker run -d --name linux_64_bit_release_build -v `pwd`:/hazelcast-cpp-client fedora_64 /bin/bash -l -c "cd hazelcast-cpp-client; scripts/release_linux_for_version.sh 64"

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




