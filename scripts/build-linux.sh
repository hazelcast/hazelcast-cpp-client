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
    HZ_COVERAGE_STRING="-DHZ_CODE_COVERAGE=ON"
fi

if [ "$4" == "COMPILE_WITHOUT_SSL" ] || [ "$5" == "COMPILE_WITHOUT_SSL" ]; then
    HZ_COMPILE_WITH_SSL=OFF
    BUILD_DIR=build${HZ_LIB_TYPE}${HZ_BIT_VERSION}${HZ_BUILD_TYPE}
else
    HZ_COMPILE_WITH_SSL=ON
    BUILD_DIR=build${HZ_LIB_TYPE}${HZ_BIT_VERSION}${HZ_BUILD_TYPE}_SSL
fi

EXECUTABLE_NAME=clientTest_${HZ_LIB_TYPE}_${HZ_BIT_VERSION}

echo HZ_BIT_VERSION=${HZ_BIT_VERSION}
echo HZ_LIB_TYPE=${HZ_LIB_TYPE}
echo HZ_BUILD_TYPE=${HZ_BUILD_TYPE}
echo BUILD_DIR=${BUILD_DIR}
echo EXECUTABLE_NAME=${EXECUTABLE_NAME}

if [ "$4" == "WITH_COVERAGE" ]; then
echo "Code coverage is ON. Cmake flag: ${HZ_COVERAGE_STRING}"
fi

# Let the submodule code be downloaded
git submodule update --init

rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}

cd ${BUILD_DIR}

echo "Running cmake to compose Makefiles for compilation."
cmake .. -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_BIT=${HZ_BIT_VERSION} -DCMAKE_BUILD_TYPE=${HZ_BUILD_TYPE} ${HZ_COVERAGE_STRING} -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=${HZ_COMPILE_WITH_SSL}
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


