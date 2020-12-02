#!/usr/bin/env bash

SRC_FILES=$(find hazelcast/ | grep -v generated-sources | grep -v cpp-controller | grep -v soak-test | grep -E "\.cpp$" );

echo "Clang-tidy is going to work on these source files:";
for file in $SRC_FILES
do
    echo "  | $file";
done
echo;

HEADER_FILTER="($(pwd)/hazelcast/include/.*)|($(pwd)/hazelcast/test/.*)|($(pwd)/examples/.*)";

echo "Header filter:";
echo "  | $HEADER_FILTER";
echo;


echo "Effective checks:"
for check in $(clang-tidy --list-checks | tail -n +2)
do
    echo "  | $check"
done
echo;


clang-tidy -p $(pwd)/build-debug --header-filter=$HEADER_FILTER $SRC_FILES -fix