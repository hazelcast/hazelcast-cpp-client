#!/usr/bin/env bash

set -e #abort the script at first failure

# Verify release
CURRENT_DIRECTORY=`pwd`

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 32 STATIC

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 32 SHARED

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 64 STATIC

${CURRENT_DIRECTORY}/scripts/verifyReleaseLinuxSingleCase.sh ${CURRENT_DIRECTORY}/cpp 64 SHARED

exit 0


