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

scripts/start-rc.sh &
rcPid=$!

echo "Spawned remote controller with pid ${rcPid}"

. scripts/build-linux.sh $1 $2 $3 $4
if [ $? -ne 0 ]; then
    echo "Build failed !"
    exit 1
fi

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
