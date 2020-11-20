#!/usr/bin/env bash

function removeBuildFolders {
    rm -rf buildSHARED${HZ_BIT_VERSION}Release
    rm -rf buildSTATIC${HZ_BIT_VERSION}Release
    rm -rf buildSHARED${HZ_BIT_VERSION}Release_SSL
    rm -rf buildSTATIC${HZ_BIT_VERSION}Release_SSL
    pkill tail
}

function cleanup {
    removeBuildFolders
}

trap cleanup EXIT

if [ $# -lt 2 ]
  then
    echo "No arguments supplied. Usage: release_linux_for_version.sh <HZ_BIT_VERSION[32 or 64] <relative_install_dir> >"
    exit 1
fi

HZ_BIT_VERSION=$1
relative_install_dir=$2

echo "Building for ${HZ_BIT_VERSION}-bit"

mkdir -p ${relative_install_dir}/Linux_${HZ_BIT_VERSION}/lib/tls

echo "Building ${HZ_BIT_VERSION}-bit STATIC library without SSL. See the output at STATIC_${HZ_BIT_VERSION}_linux.txt."
scripts/build-linux.sh ${HZ_BIT_VERSION} STATIC Release COMPILE_WITHOUT_SSL &> STATIC_${HZ_BIT_VERSION}_linux.txt &
STATIC_pid=$!

echo "Building ${HZ_BIT_VERSION}-bit SHARED library without SSL. See the output at SHARED_${HZ_BIT_VERSION}_linux.txt."
scripts/build-linux.sh ${HZ_BIT_VERSION} SHARED Release COMPILE_WITHOUT_SSL &> SHARED_${HZ_BIT_VERSION}_linux.txt &
SHARED_pid=$!

echo "Building ${HZ_BIT_VERSION}-bit STATIC library with SSL. See the output at STATIC_${HZ_BIT_VERSION}_SSL_linux.txt."
scripts/build-linux.sh ${HZ_BIT_VERSION} STATIC Release &> STATIC_${HZ_BIT_VERSION}_SSL_linux.txt &
STATIC_SSL_pid=$!

echo "Building ${HZ_BIT_VERSION}-bit SHARED library with SSL. See the output at SHARED_${HZ_BIT_VERSION}_SSL_linux.txt."
scripts/build-linux.sh ${HZ_BIT_VERSION} SHARED Release &> SHARED_${HZ_BIT_VERSION}_SSL_linux.txt &
SHARED_SSL_pid=$!

tail -f *_linux.txt &

FAIL=0
wait ${STATIC_pid} || let "FAIL+=1"
wait ${STATIC_SSL_pid} || let "FAIL+=1"
wait ${SHARED_pid} || let "FAIL+=1"
wait ${SHARED_SSL_pid} || let "FAIL+=1"

if [ $FAIL -ne 0 ]; then
    echo "$FAIL builds FAILED for ${HZ_BIT_VERSION}-bit build!!!"
    exit $FAIL
fi

echo "Copying the binary libraries to release folder"
cp buildSTATIC${HZ_BIT_VERSION}Release/libHazelcastClient* ${relative_install_dir}/Linux_${HZ_BIT_VERSION}/lib/
cp buildSHARED${HZ_BIT_VERSION}Release/libHazelcastClient* ${relative_install_dir}/Linux_${HZ_BIT_VERSION}/lib/
cp buildSTATIC${HZ_BIT_VERSION}Release_SSL/libHazelcastClient* ${relative_install_dir}/Linux_${HZ_BIT_VERSION}/lib/tls/
cp buildSHARED${HZ_BIT_VERSION}Release_SSL/libHazelcastClient* ${relative_install_dir}/Linux_${HZ_BIT_VERSION}/lib/tls/

CURRENT_DIRECTORY=`pwd`
${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/${relative_install_dir} ${HZ_BIT_VERSION} STATIC &> verify_${HZ_BIT_VERSION}_STATIC.txt &
STATIC_pid=$!

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/${relative_install_dir} ${HZ_BIT_VERSION} SHARED &> verify_${HZ_BIT_VERSION}_SHARED.txt &
SHARED_pid=$!

tail -f verify_${HZ_BIT_VERSION}_*.txt &

FAIL=0
wait ${STATIC_pid} || let "FAIL+=1"
wait ${SHARED_pid} || let "FAIL+=1"

if [ $FAIL -ne 0 ]; then
    echo "$FAIL verifications FAILED for ${HZ_BIT_VERSION}-bit build !!!"
else
    echo "All verifications PASSED for for ${HZ_BIT_VERSION}-bit build :)"
fi

exit $FAIL


