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

mkdir -p ./cpp/Linux_32/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_32/hazelcast/lib/tls
mkdir -p ./cpp/Linux_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_64/hazelcast/lib/tls

cp -R hazelcast/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp -R hazelcast/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/

scripts/build-linux.sh 32 STATIC Release COMPILE_WITHOUT_SSL &> STATIC_32_linux.txt &
STATIC_32_pid=$!

scripts/build-linux.sh 32 SHARED Release COMPILE_WITHOUT_SSL &> SHARED_32_linux.txt &
SHARED_32_pid=$!

scripts/build-linux.sh 64 STATIC Release COMPILE_WITHOUT_SSL &> STATIC_64_linux.txt &
STATIC_64_pid=$!

scripts/build-linux.sh 64 SHARED Release COMPILE_WITHOUT_SSL &> SHARED_64_linux.txt &
SHARED_64_pid=$!

scripts/build-linux.sh 32 STATIC Release &> STATIC_32_SSL_linux.txt &
STATIC_32_SSL_pid=$!

scripts/build-linux.sh 32 SHARED Release &> SHARED_32_SSL_linux.txt &
SHARED_32_SSL_pid=$!

scripts/build-linux.sh 64 STATIC Release &> STATIC_64_SSL_linux.txt &
STATIC_64_SSL_pid=$!

scripts/build-linux.sh 64 SHARED Release &> SHARED_64_SSL_linux.txt &
SHARED_64_SSL_pid=$!

FAIL=0
wait ${STATIC_32_pid} || let "FAIL+=1"
wait ${STATIC_32_SSL_pid} || let "FAIL+=1"
wait ${SHARED_32_pid} || let "FAIL+=1"
wait ${SHARED_32_SSL_pid} || let "FAIL+=1"
wait ${STATIC_64_pid} || let "FAIL+=1"
wait ${STATIC_64_SSL_pid} || let "FAIL+=1"
wait ${SHARED_64_pid} || let "FAIL+=1"
wait ${SHARED_64_SSL_pid} || let "FAIL+=1"

if [ $FAIL -ne 0 ]; then
    echo "$FAIL builds FAILED!!!"
    exit $FAIL
fi

cp buildSTATIC32Release/libHazelcastClient* cpp/Linux_32/hazelcast/lib/

cp buildSHARED32Release/libHazelcastClient* cpp/Linux_32/hazelcast/lib/

cp buildSTATIC64Release/libHazelcastClient* cpp/Linux_64/hazelcast/lib/

cp buildSHARED64Release/libHazelcastClient* cpp/Linux_64/hazelcast/lib/

cp buildSTATIC32Release_SSL/libHazelcastClient* cpp/Linux_32/hazelcast/lib/tls/

cp buildSHARED32Release_SSL/libHazelcastClient* cpp/Linux_32/hazelcast/lib/tls/

cp buildSTATIC64Release_SSL/libHazelcastClient* cpp/Linux_64/hazelcast/lib/tls/

cp buildSHARED64Release_SSL/libHazelcastClient* cpp/Linux_64/hazelcast/lib/tls/

echo "Copying the examples"
mkdir -p cpp/examples
cp -r examples cpp/examples/src

echo "Linking to examples for 32-bit release"
cd cpp/Linux_32
ln -s ../examples .
cd -

echo "Linking to examples for 64-bit release"
cd cpp/Linux_64
ln -s ../examples .
cd -

# Verify release
scripts/verifyReleaseLinux.sh

