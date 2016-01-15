#!/usr/bin/env bash

function cleanup {
    echo "cleanup is being performed."
    if [ "x${serverPid}" != "x" ]
    then
        echo "Killing server with pid ${serverPid}"
        kill -9 ${serverPid}
    fi
    exit
}

trap cleanup EXIT

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

BUILD_DIR=build${HZ_LIB_TYPE}${HZ_BIT_VERSION}${HZ_BUILD_TYPE}

EXECUTABLE_NAME=clientTest_${HZ_LIB_TYPE}_${HZ_BIT_VERSION}

echo HZ_BIT_VERSION=${HZ_BIT_VERSION}
echo HZ_LIB_TYPE=${HZ_LIB_TYPE}
echo HZ_BUILD_TYPE=${HZ_BUILD_TYPE}
echo BUILD_DIR=${BUILD_DIR}
echo EXECUTABLE_NAME=${EXECUTABLE_NAME}

if [ "$4" == "WITH_COVERAGE" ]; then
echo "Code coverage is ON. Cmake flag: ${HZ_COVERAGE_STRING}"
fi

rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}

echo "Running cmake to compose Makefiles for compilation."
cmake .. -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_BIT=${HZ_BIT_VERSION} -DCMAKE_BUILD_TYPE=${HZ_BUILD_TYPE} ${HZ_COVERAGE_STRING}

echo "Running make. Building the project."
make -j 8 -l 4  # run 8 jobs in parallel and a maximum load of 4
if [ $? -ne 0 ]
then
    echo "Client compilation failed!!!"
    exit 1
fi

cd ..
cd java

echo "Compiling the java test server"
mvn -U -q clean install
if [ $? -ne 0 ]
then
    echo "Server compilation and install failed!!!"
    exit 1
fi

echo "Starting the java test server"
mvn exec:java -Dexec.mainClass="CppClientListener" &
serverPid=$!

echo "Spawned server with pid ${serverPid}"

DEFAULT_TIMEOUT=30 #seconds
SERVER_PORT=6543

timeout=${DEFAULT_TIMEOUT}

echo "Waiting for the test server to start"

while [ ${timeout} -gt 0 ]
do
    netstat -an  | grep ${SERVER_PORT}
    if [ $? -eq 0 ]; then
        break
    fi

    echo "Sleeping 1 second. Remaining ${timeout} seconds"
    sleep 1

    timeout=$((timeout-1))
done

if [ ${timeout} -eq 0 ]; then
    echo "Server could not start on port ${SERVER_PORT} in $DEFAULT_TIMEOUT seconds. Test FAILED."
    exit 1
else
    echo "Server started in $((DEFAULT_TIMEOUT - timeout)) seconds"
fi
 
cd ..

echo "Starting the client test now."
${BUILD_DIR}/hazelcast/test/${EXECUTABLE_NAME}
result=$?


if [ ${result} -eq 0 ]
then
    echo "Test PASSED."
else
    echo "Test FAILED. Result:${result}"
fi

exit ${result}
