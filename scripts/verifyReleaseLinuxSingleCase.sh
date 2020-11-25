#!/usr/bin/env bash

HZ_INSTALL_DIR=$1
HZ_VERSION="4.0-BETA"
HZ_BIT_VERSION=$2
HZ_LIB_TYPE=$3

BUILD_DIR=${HZ_BIT_VERSION}_${HZ_LIB_TYPE}

echo "Verifying the release located at ${HZ_INSTALL_DIR} for ${HZ_LIB_TYPE} ${HZ_BIT_VERSION} bit library."

cd examples
echo "Inside examples directory:"
pwd
rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}
echo "Created examples build directory ${BUILD_DIR}"
ls -altr
cd ${BUILD_DIR}

echo "Generating the solution files for compilation for non-SSL ${HZ_BIT_VERSION}-bit-${HZ_LIB_TYPE} build."
cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DCMAKE_CXX_FLAGS="-m${HZ_BIT_VERSION}"
if [ $? -ne 0 ];then
    echo "Cmake failed for 'cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DCMAKE_CXX_FLAGS=-m${HZ_BIT_VERSION}'"
    exit 1
fi

echo "Compiling non-ssl build for ${HZ_BIT_VERSION}-bit-${HZ_LIB_TYPE} build now."
make -j 8
if [ $? -ne 0 ];then
    echo "make failed for -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE}"
    exit 2
fi

echo "Non-ssl build successfully finished for ${HZ_BIT_VERSION}-bit-${HZ_LIB_TYPE} build."

# clean the directory for a fresh ${BUILD_DIR}
rm -rf *

echo "Generating the solution files for compilation for SSL ${HZ_BIT_VERSION}-bit-${HZ_LIB_TYPE} build."
cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_COMPILE_WITH_SSL=ON -DCMAKE_CXX_FLAGS="-m${HZ_BIT_VERSION}"
if [ $? -ne 0 ];then
    echo "Cmake failed for 'cmake .. -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_COMPILE_WITH_SSL=ON -DCMAKE_CXX_FLAGS=-m${HZ_BIT_VERSION}'"
    exit 1
fi

echo "Compiling ssl build for ${HZ_BIT_VERSION}-bit-${HZ_LIB_TYPE} build now."
make -j 8
if [ $? -ne 0 ];then
    echo "make failed for -DHAZELCAST_INSTALL_DIR=${HZ_INSTALL_DIR} -DHZ_VERSION=${HZ_VERSION} -DHZ_BIT=${HZ_BIT_VERSION} -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_COMPILE_WITH_SSL=ON -DCMAKE_CXX_FLAGS=-m${HZ_BIT_VERSION}"
    exit 2
fi

echo "Ssl build successfully finished for ${HZ_BIT_VERSION}-bit-${HZ_LIB_TYPE} build."

echo "Verification of the release located at ${HZ_INSTALL_DIR} for version ${HZ_VERSION} ${HZ_LIB_TYPE} ${HZ_BIT_VERSION}-bit library is finished."

exit 0

