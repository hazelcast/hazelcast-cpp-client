#!/usr/bin/env bash
set -e #abort the script at first faiure

rm -rf ./cpp

mkdir -p ./cpp/Mac_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Mac_64/hazelcast/lib/tls

cp -R hazelcast/include/hazelcast/* cpp/Mac_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Mac_64/hazelcast/include/hazelcast/

scripts/build-linux.sh 64 STATIC Release COMPILE_WITHOUT_SSL
cp buildSTATIC64Release/libHazelcastClient* cpp/Mac_64/hazelcast/lib/
rm -rf buildSTATIC64Release

scripts/build-linux.sh 64 SHARED Release COMPILE_WITHOUT_SSL
cp buildSHARED64Release/libHazelcastClient* cpp/Mac_64/hazelcast/lib/
rm -rf buildSHARED64Release

echo "Generating TLS enabled libraries"

scripts/build-linux.sh 64 STATIC Release
cp buildSTATIC64Release/libHazelcastClient* cpp/Mac_64/hazelcast/lib/tls/
rm -rf buildSTATIC64Release

scripts/build-linux.sh 64 SHARED Release
cp buildSHARED64Release/libHazelcastClient* cpp/Mac_64/hazelcast/lib/tls/
rm -rf buildSHARED64Release

echo "Copying external libraries and the examples"
mkdir -p cpp/external
cp -R external/release_include cpp/external/include
mkdir -p cpp/examples
cp -r examples cpp/examples/src

echo "Linking to external libraries and examples for 32-bit release"
cd cpp/Mac_32
ln -s ../examples .
ln -s ../external .
cd -

echo "Linking to external libraries and examples for 64-bit release"
cd cpp/Mac_64
ln -s ../examples .
ln -s ../external .
cd -

#MAC SPECIFIC
cd cpp/Mac_64/hazelcast/lib/
export HAZELCAST_SHARED_LIB_NAME=$(echo *dylib)
install_name_tool -id ${HAZELCAST_SHARED_LIB_NAME} ${HAZELCAST_SHARED_LIB_NAME}

cd ../../../../

# Uncomment below if you want to generate doxygen docs

#echo "Generating docs "
doxygen docsConfig

#echo "Moving docs to target"
mv  docs cpp/

# Verify release
scripts/verifyReleaseOSX.sh
