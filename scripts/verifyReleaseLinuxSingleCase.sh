#!/usr/bin/env bash

HZ_INSTALL_DIR=$1
HZ_VERSION="3.12.1-SNAPSHOT"
HZ_BIT_VERSION=$2
HZ_LIB_TYPE=$3

BUILD_DIR=${HZ_BIT_VERSION}_${HZ_LIB_TYPE}

echo "Verifying the release located at ${HZ_INSTALL_DIR} for ${HZ_LIB_TYPE} ${HZ_BIT_VERSION} bit library."

cd examples
rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}

echo "Generating the solution files for compilation without TLS"
cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE}
if [ $? -ne 0 ];then
    echo "Cmake failed for 'cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE}'"
    exit 1
fi

make -j 4
if [ $? -ne 0 ];then
    echo "make failed for -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE}"
    exit 2
fi

# clean the directory for a fresh ${BUILD_DIR}
rm -rf *

cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_COMPILE_WITH_SSL=ON
if [ $? -ne 0 ];then
    echo "Cmake failed for 'cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_COMPILE_WITH_SSL=ON'"
    exit 1
fi

make -j 4
if [ $? -ne 0 ];then
    echo "make failed for -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_COMPILE_WITH_SSL=ON"
    exit 2
fi

echo "Verification of the release located at ${HZ_INSTALL_DIR} for version ${HZ_VERSION} ${HZ_LIB_TYPE} ${HZ_BIT_VERSION}-bit library is finished."

exit 0

