#!/usr/bin/env bash
set -e #abort the script at first faiure

echo "Cleanup release directories"
rm -rf ./Release*
rm -rf ./cpp

echo "Compiling Static Library"
mkdir ReleaseStatic
cd ReleaseStatic
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4
cd ..

echo "Compiling Shared Library"
mkdir ReleaseShared
cd ReleaseShared
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4
cd ..

#STANDART PART
mkdir -p ./cpp/Mac_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Mac_64/hazelcast/lib
mkdir -p ./cpp/Mac_64/external/include
mkdir -p ./cpp/Mac_64/examples/

echo "Moving headers to target"
cp -R hazelcast/include/hazelcast/ cpp/Mac_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Mac_64/hazelcast/include/hazelcast/
echo "Moving libraries to target"
cp ReleaseStatic/libHazelcastClient*.a cpp/Mac_64/hazelcast/lib/
cp ReleaseShared/libHazelcastClient*.dylib cpp/Mac_64/hazelcast/lib/

echo "Moving dependencies to target"
cp -R external/include/ cpp/Mac_64/external/include/

echo "Moving examples to target"
cp -r examples cpp/Mac_64/examples/src
find ReleaseStatic/examples -perm +111 -type f -exec cp {} cpp/Mac_64/examples/ \;

#MAC SPECIFIC
cd cpp/Mac_64/hazelcast/lib/
export HAZELCAST_SHARED_LIB_NAME=$(echo *dylib)
install_name_tool -id ${HAZELCAST_SHARED_LIB_NAME} ${HAZELCAST_SHARED_LIB_NAME}
cd ../../../../

#ONLY IN DEVELOPMENT MACHINE
mkdir -p ./cpp/docs/

echo "Generating docs "
doxygen docsConfig

echo "Moving docs to target"
mv  docs/ cpp

echo "Removing temporary files"
rm -rf ./ReleaseShared
rm -rf ./ReleaseStatic
