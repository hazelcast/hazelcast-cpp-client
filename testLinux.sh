#!/usr/bin/env bash

HZ_BIT_VERSION=32
HZ_LIB_TYPE=SHARED
HZ_BUILD_TYPE=Debug

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

HZ_BIT_VERSION=32
HZ_LIB_TYPE=SHARED
HZ_BUILD_TYPE=Release

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

HZ_BIT_VERSION=32
HZ_LIB_TYPE=STATIC
HZ_BUILD_TYPE=Debug

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

HZ_BIT_VERSION=32
HZ_LIB_TYPE=STATIC
HZ_BUILD_TYPE=Release

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

HZ_BIT_VERSION=64
HZ_LIB_TYPE=SHARED
HZ_BUILD_TYPE=Debug

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

HZ_BIT_VERSION=64
HZ_LIB_TYPE=SHARED
HZ_BUILD_TYPE=Release

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

HZ_BIT_VERSION=64
HZ_LIB_TYPE=STATIC
HZ_BUILD_TYPE=Debug

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

HZ_BIT_VERSION=64
HZ_LIB_TYPE=STATIC
HZ_BUILD_TYPE=Release

./testLinuxSingleCase.sh ${HZ_BIT_VERSION} ${HZ_LIB_TYPE} ${HZ_BUILD_TYPE}
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. Bit Version:${HZ_BIT_VERSION}, Library type:${HZ_LIB_TYPE}, Build Version:${HZ_BUILD_TYPE}"
    exit 1
fi

# Test if the release script is still working
./releaseLinux.sh
if [ $? -ne 0 ]
then
    echo "******  Test FAILED. The release script failed!!!"
    exit 1
fi

# Remove the release related folder
rm -rf ./Release*
rm -rf ./cpp

echo "All tests PASSED"

exit 0


