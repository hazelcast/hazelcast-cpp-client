#!/bin/bash

# Runs the tests on UNIX-like platforms
# The script should be run from the project's root directory
#
# This environment variables are the parameters to this script:
# - BUILD_DIR : build directory
#

function cleanup {
    echo "cleanup is being performed."
    if [ "x${rcPid}" != "x" ]
    then
        echo "Killing client with pid ${testPid}"
        kill -9 ${testPid} || true
        echo "Killing start-rc.sh with pid ${rcPid}"
        kill -9 ${rcPid} || true
    fi
    exit
}

trap cleanup EXIT

RC_START_TIMEOUT_IN_SECS=300
RC_PORT=9701

TEST_EXECUTABLE="$BUILD_DIR/hazelcast/test/src/client_test"

scripts/start-rc.sh &
rcPid=$!

echo "Spawned remote controller with pid ${rcPid}"

timeout=${RC_START_TIMEOUT_IN_SECS}

echo "Waiting for remote-controller to start..."

while [ ${timeout} -gt 0 ]; do
  netstat -an | grep "[^0-9]$RC_PORT[^0-9]" | grep LISTEN
  if [ $? -eq 0 ]; then
    break
  fi

  echo "Sleeping 1 second. Remaining $timeout seconds."
  sleep 1

  timeout=$((timeout - 1))
done

if [ $timeout -eq 0 ]; then
  echo "Server could not start on port $RC_PORT in $RC_START_TIMEOUT_IN_SECS seconds. Test FAILED."
  exit 1
else
  echo "Server started in $((RC_START_TIMEOUT_IN_SECS - timeout)) seconds"
fi

echo "Starting the client test now."

${TEST_EXECUTABLE} --gtest_filter="*SqlTest*" --gtest_output="xml:CPP_Client_Test_Report.xml" &
testPid=$!
wait ${testPid}
result=$?

if [ ${result} -eq 0 ]; then
  echo "Test PASSED."
else
  echo "Test FAILED. Result:${result}"
fi

#get stack trace of core if exists
if [ -f core.${testPid} ]; then
  echo "Found core file for the test process: core.${testPid}"
  gdb --batch --quiet -ex "thread apply all bt full" -ex "quit" $TEST_EXECUTABLE core.${testPid}
fi

exit ${result}
