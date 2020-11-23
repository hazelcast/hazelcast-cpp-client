#!/usr/bin/env bash

function removeBuildFolders {
    rm -rf buildSTATIC64Release
    rm -rf buildSHARED64Release
    rm -rf buildSTATIC64Release_SSL
    rm -rf buildSHARED64Release_SSL
    pkill tail
}

function cleanup {
    removeBuildFolders
}

trap cleanup EXIT

rm -rf ./cpp

mkdir -p ./cpp/include
mkdir -p ./cpp/Mac_64/lib/tls

echo "Copying the include files"
cp -R hazelcast/include/hazelcast cpp/include/
cp -R hazelcast/generated-sources/src/hazelcast/client/protocol/codec/*.h cpp/include/hazelcast/client/protocol/codec/

echo "Copying the examples"
cp -r examples cpp/

echo "Building 64-bit STATIC library without SSL. See the output at STATIC_64_macos.txt."
scripts/build-linux.sh 64 STATIC Release COMPILE_WITHOUT_SSL &> STATIC_64_macos.txt &
STATIC_64_pid=$!

echo "Building 64-bit SHARED library without SSL. See the output at SHARED_64_macos.txt."
scripts/build-linux.sh 64 SHARED Release COMPILE_WITHOUT_SSL &> SHARED_64_macos.txt &
SHARED_64_pid=$!

echo "Building 64-bit STATIC library with SSL. See the output at STATIC_64_SSL_macos.txt."
scripts/build-linux.sh 64 STATIC Release &> STATIC_64_SSL_macos.txt &
STATIC_64_SSL_pid=$!

echo "Building 64-bit SHARED library with SSL. See the output at STATIC_64_SSL_macos.txt."
scripts/build-linux.sh 64 SHARED Release &> SHARED_64_SSL_macos.txt &
SHARED_64_SSL_pid=$!

tail -f *_macos.txt &

FAIL=0
wait ${STATIC_64_pid} || let "FAIL+=1"
wait ${STATIC_64_SSL_pid} || let "FAIL+=1"
wait ${SHARED_64_pid} || let "FAIL+=1"
wait ${SHARED_64_SSL_pid} || let "FAIL+=1"

if [ $FAIL -ne 0 ]; then
    echo "$FAIL builds FAILED!!!"
    exit $FAIL
fi

echo "Copying the binary libraries to release folder"
cp buildSTATIC64Release/libHazelcastClient* cpp/Mac_64/lib/
cp buildSHARED64Release/libHazelcastClient* cpp/Mac_64/lib/
cp buildSTATIC64Release_SSL/libHazelcastClient* cpp/Mac_64/lib/tls/
cp buildSHARED64Release_SSL/libHazelcastClient* cpp/Mac_64/lib/tls/

#MAC SPECIFIC
cd cpp/Mac_64/lib/
export HAZELCAST_SHARED_LIB_NAME=$(echo *dylib)
install_name_tool -id ${HAZELCAST_SHARED_LIB_NAME} ${HAZELCAST_SHARED_LIB_NAME}

cd -

# Uncomment below if you want to generate doxygen docs

#echo "Generating docs "
doxygen docsConfig

#echo "Moving docs to target"
mv  docs cpp/

# Verify release
scripts/verifyReleaseOSX.sh
ls
