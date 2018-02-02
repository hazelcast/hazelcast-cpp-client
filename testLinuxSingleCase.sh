#!/usr/bin/env bash

function cleanup {
    echo "cleanup is being performed."
    if [ "x${rcPid}" != "x" ]
    then
        echo "Killing client with pid ${testPid}"
        kill -9 ${testPid}
        echo "Killing start-rc.sh with pid ${rcPid}"
        kill ${rcPid}
    fi
    exit
}

# Disables printing security sensitive data to the logs
set +x

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

if [ "$4" == "COMPILE_WITHOUT_SSL" ] || [ "$5" == "COMPILE_WITHOUT_SSL" ]; then
    HZ_COMPILE_WITH_SSL=OFF
else
    HZ_COMPILE_WITH_SSL=ON
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

# Let the submodule code be downloaded
git submodule update --init

rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}

echo "Running cmake to compose Makefiles for compilation."
cmake .. -DHZ_LIB_TYPE=${HZ_LIB_TYPE} -DHZ_BIT=${HZ_BIT_VERSION} -DCMAKE_BUILD_TYPE=${HZ_BUILD_TYPE} ${HZ_COVERAGE_STRING} -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=${HZ_COMPILE_WITH_SSL}

echo "Running make. Building the project."
make -j 8 -l 4 VERBOSE=1  # run 8 jobs in parallel and a maximum load of 4
if [ $? -ne 0 ]
then
    echo "Client compilation failed!!!"
    exit 1
fi

cd ..

pip install --user -r hazelcast/test/test_requirements.txt
if [ $? -ne 0 ]; then
    echo "Failed to install python hazelcast-remote-controller library."
    exit 1
fi

scripts/start-rc.sh &
rcPid=$!

echo "Spawned remote controller with pid ${rcPid}"

DEFAULT_TIMEOUT=300 #seconds
SERVER_PORT=9701

timeout=${DEFAULT_TIMEOUT}

echo "Waiting for the test server to start"

while [ ${timeout} -gt 0 ]
do
    netstat -an  | grep "${SERVER_PORT} "
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

echo "Starting the client test now."
${BUILD_DIR}/hazelcast/test/src/${EXECUTABLE_NAME} --gtest_output="xml:CPP_Client_Test_Report.xml" &
testPid=$!
wait ${testPid}
result=$?

if [ ${result} -eq 0 ]
then
    echo "Test PASSED."
else
    echo "Test FAILED. Result:${result}"
fi

#get stack trace of core if exists
if [ -f core.${testPid} ]
then
    echo "Found core file for the test process: core.${testPid}"
    gdb --batch --quiet -ex "thread apply all bt full" -ex "quit" ${BUILD_DIR}/hazelcast/test/src/${EXECUTABLE_NAME} core.${testPid}
fi

exit ${result}
