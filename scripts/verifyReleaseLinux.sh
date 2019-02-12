#!/usr/bin/env bash

# Verify release
CURRENT_DIRECTORY=`pwd`

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 32 STATIC &> verify_32_STATIC.txt &
STATIC_32_pid=$!

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 32 SHARED &> verify_32_SHARED.txt &
SHARED_32_pid=$!

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 64 STATIC &> verify_64_STATIC.txt &
STATIC_64_pid=$!

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 64 SHARED &> verify_64_SHARED.txt &
SHARED_64_pid=$!

FAIL=0
wait ${STATIC_32_pid} || let "FAIL+=1"
wait ${SHARED_32_pid} || let "FAIL+=1"
wait ${STATIC_64_pid} || let "FAIL+=1"
wait ${SHARED_64_pid} || let "FAIL+=1"

if [ $FAIL -ne 0 ]; then
    echo "$FAIL verifications FAILED !!!"
else
    echo "All verifications PASSED"
fi

exit $FAIL




