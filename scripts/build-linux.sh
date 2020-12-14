#!/usr/bin/env bash

# Disables printing security sensitive data to the logs
set +x

HZ_BIT_VERSION=$1
HZ_LIB_TYPE=$2
HZ_BUILD_TYPE=$3

if [ "$4" == "WITH_COVERAGE" ]; then
    if [ ${HZ_BUILD_TYPE} != Debug ]; then
        echo "WITH_COVERAGE is requested. The build type should be Debug but it is provided as ${HZ_BUILD_TYPE}."
        exit 1
    fi
    COVERAGE_FLAGS="-fprofile-arcs -ftest-coverage -fPIC -O0"
fi

if [ "$4" == "COMPILE_WITHOUT_SSL" ] || [ "$5" == "COMPILE_WITHOUT_SSL" ]; then
    HZ_COMPILE_WITH_SSL=OFF
    BUILD_DIR=build${HZ_LIB_TYPE}${HZ_BIT_VERSION}${HZ_BUILD_TYPE}
else
    HZ_COMPILE_WITH_SSL=ON
    BUILD_DIR=build${HZ_LIB_TYPE}${HZ_BIT_VERSION}${HZ_BUILD_TYPE}_SSL
fi

HZ_COMPILE_WITH_TESTS=ON
if [ "$4" == "WITHOUT_TESTS" ] || [ "$5" == "WITHOUT_TESTS" ] || [ "$6" == "WITHOUT_TESTS" ]; then
    HZ_COMPILE_WITH_TESTS=OFF
fi

if [ "$HZ_LIB_TYPE" == "SHARED" ]; then
    HZ_BUILD_SHARED=ON
    HZ_BUILD_STATIC=OFF
else
    HZ_BUILD_SHARED=OFF
    HZ_BUILD_STATIC=ON
fi

EXECUTABLE_NAME=client_test

echo HZ_BIT_VERSION=${HZ_BIT_VERSION}
echo HZ_LIB_TYPE=${HZ_LIB_TYPE}
echo HZ_BUILD_SHARED=${HZ_BUILD_SHARED}
echo HZ_BUILD_STATIC=${HZ_BUILD_STATIC}
echo HZ_BUILD_TYPE=${HZ_BUILD_TYPE}
echo BUILD_DIR=${BUILD_DIR}
echo EXECUTABLE_NAME=${EXECUTABLE_NAME}

if [ "$4" == "WITH_COVERAGE" ]; then
  echo "Code coverage is ON. Additional compiler flags: ${COVERAGE_FLAGS}"
fi

rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}

cd ${BUILD_DIR}

echo "Running cmake to compose Makefiles for compilation."
cmake .. -DBUILD_SHARED_LIB=${HZ_BUILD_SHARED} -DBUILD_STATIC_LIB=${HZ_BUILD_STATIC} \
         -DCMAKE_BUILD_TYPE=${HZ_BUILD_TYPE} \
         -DBUILD_TESTS=${HZ_COMPILE_WITH_TESTS} -DBUILD_EXAMPLES=ON \
         -DWITH_OPENSSL=${HZ_COMPILE_WITH_SSL} -DINSTALL_GTEST=OFF \
         -DCMAKE_CXX_FLAGS="-m${HZ_BIT_VERSION} ${COVERAGE_FLAGS}"

if [ $? -ne 0 ]; then
    echo "Cmake failed !"
    exit 1
fi


echo "Running make. Building the project."
make -j 8 -l 4 VERBOSE=1  # run 8 jobs in parallel and a maximum load of 4
if [ $? -ne 0 ]; then
    echo "Build failed !"
    exit 1
fi

cd ..
