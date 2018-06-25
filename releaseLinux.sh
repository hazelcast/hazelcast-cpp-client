#!/usr/bin/env bash
set -e #abort the script at first failure

rm -rf ./cpp

mkdir -p ./cpp/Linux_32/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_32/hazelcast/lib/tls
mkdir -p ./cpp/Linux_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_64/hazelcast/lib/tls

cp -R hazelcast/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp -R hazelcast/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/

scripts/build-linux.sh 32 STATIC Release COMPILE_WITHOUT_SSL
cp buildSTATIC32Release/libHazelcastClient* cpp/Linux_32/hazelcast/lib/
rm -rf buildSTATIC32Release

scripts/build-linux.sh 32 SHARED Release COMPILE_WITHOUT_SSL
cp buildSHARED32Release/libHazelcastClient* cpp/Linux_32/hazelcast/lib/
rm -rf buildSHARED32Release

scripts/build-linux.sh 64 STATIC Release COMPILE_WITHOUT_SSL
cp buildSTATIC64Release/libHazelcastClient* cpp/Linux_64/hazelcast/lib/
rm -rf buildSTATIC64Release

scripts/build-linux.sh 64 SHARED Release COMPILE_WITHOUT_SSL
cp buildSHARED64Release/libHazelcastClient* cpp/Linux_64/hazelcast/lib/
rm -rf buildSHARED64Release

echo "Generating TLS enabled libraries"

scripts/build-linux.sh 32 STATIC Release
cp buildSTATIC32Release/libHazelcastClient* cpp/Linux_32/hazelcast/lib/tls/
rm -rf buildSTATIC32Release

scripts/build-linux.sh 32 SHARED Release
cp buildSHARED32Release/libHazelcastClient* cpp/Linux_32/hazelcast/lib/tls/
rm -rf buildSHARED32Release

scripts/build-linux.sh 64 STATIC Release
cp buildSTATIC64Release/libHazelcastClient* cpp/Linux_64/hazelcast/lib/tls/
rm -rf buildSTATIC64Release

scripts/build-linux.sh 64 SHARED Release
cp buildSHARED64Release/libHazelcastClient* cpp/Linux_64/hazelcast/lib/tls/
rm -rf buildSHARED64Release

echo "Copying external libraries and the examples"
mkdir -p cpp/external
cp -R external/release_include cpp/external/include
mkdir -p cpp/examples
cp -r examples cpp/examples/src

echo "Linking to external libraries and examples for 32-bit release"
cd cpp/Linux_32
ln -s ../examples .
ln -s ../external .
cd -

echo "Linking to external libraries and examples for 64-bit release"
cd cpp/Linux_64
ln -s ../examples .
ln -s ../external .
cd -

# Verify release
scripts/verifyReleaseLinux.sh

