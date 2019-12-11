#!/usr/bin/env bash

function removeBuildFolders {
    rm -rf buildSTATIC64Release
    rm -rf buildSHARED32Release
    rm -rf buildSTATIC32Release
    rm -rf buildSHARED64Release
    rm -rf buildSTATIC64Release_SSL
    rm -rf buildSHARED32Release_SSL
    rm -rf buildSTATIC32Release_SSL
    rm -rf buildSHARED64Release_SSL
}

function cleanup {
    removeBuildFolders
}

trap cleanup EXIT

rm -rf ./cpp

mkdir -p ./cpp/Mac_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Mac_64/hazelcast/lib/tls

cp -R hazelcast/include/hazelcast/* cpp/Mac_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Mac_64/hazelcast/include/hazelcast/

scripts/build-linux.sh 64 STATIC Release COMPILE_WITHOUT_SSL &> STATIC_64_linux.txt &
STATIC_64_pid=$!

scripts/build-linux.sh 64 SHARED Release COMPILE_WITHOUT_SSL &> SHARED_64_linux.txt &
SHARED_64_pid=$!

scripts/build-linux.sh 64 STATIC Release &> STATIC_64_SSL_linux.txt &
STATIC_64_SSL_pid=$!

scripts/build-linux.sh 64 SHARED Release &> SHARED_64_SSL_linux.txt &
SHARED_64_SSL_pid=$!

FAIL=0
wait ${STATIC_64_pid} || let "FAIL+=1"
wait ${STATIC_64_SSL_pid} || let "FAIL+=1"
wait ${SHARED_64_pid} || let "FAIL+=1"
wait ${SHARED_64_SSL_pid} || let "FAIL+=1"

if [ $FAIL -ne 0 ]; then
    echo "$FAIL builds FAILED!!!"
    exit $FAIL
fi

cp buildSTATIC64Release/libHazelcastClient* cpp/Mac_64/hazelcast/lib/

cp buildSHARED64Release/libHazelcastClient* cpp/Mac_64/hazelcast/lib/

cp buildSTATIC64Release_SSL/libHazelcastClient* cpp/Mac_64/hazelcast/lib/tls/

cp buildSHARED64Release_SSL/libHazelcastClient* cpp/Mac_64/hazelcast/lib/tls/

echo "Copying external libraries and the examples"
mkdir -p cpp/external
cp -R external/release_include cpp/external/include
mkdir -p cpp/examples
cp -r examples cpp/examples/src

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
