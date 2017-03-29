#!/usr/bin/env bash
set -e #abort the script at first faiure

echo "Cleanup release directories"
rm -rf ./Release*
rm -rf ./cpp

echo "Compiling Static Library"
mkdir ReleaseStatic
cd ReleaseStatic
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4 VERBOSE=1
cd ..

echo "Compiling Static Library with TLS support"
mkdir ReleaseStaticTLS
cd ReleaseStaticTLS
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
make -j 4 VERBOSE=1
cd ..

echo "Compiling Shared Library"
mkdir ReleaseShared
cd ReleaseShared
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4 VERBOSE=1
cd ..

echo "Compiling Shared Library with TLS support"
mkdir ReleaseSharedTLS
cd ReleaseSharedTLS
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
make -j 4 VERBOSE=1
cd ..

#STANDART PART
mkdir -p ./cpp/Mac_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Mac_64/hazelcast/lib/tls
mkdir -p ./cpp/Mac_64/external/include

echo "Moving headers to target"
cp -R hazelcast/include/hazelcast/ cpp/Mac_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Mac_64/hazelcast/include/hazelcast/
echo "Moving libraries to target"
cp ReleaseStatic/libHazelcastClient*.a cpp/Mac_64/hazelcast/lib/
cp ReleaseStaticTLS/libHazelcastClient*.a cpp/Mac_64/hazelcast/lib/tls/
cp ReleaseShared/libHazelcastClient*.dylib cpp/Mac_64/hazelcast/lib/
cp ReleaseSharedTLS/libHazelcastClient*.dylib cpp/Mac_64/hazelcast/lib/tls/

echo "Copying external libraries and the examples"
mkdir -p examples
cp -r examples examples/src

echo "Linking to external libraries and examples"
cd cpp/Mac_64
ln -s ../examples .
ln -s ../external .
cd -

#MAC SPECIFIC
cd cpp/Mac_64/hazelcast/lib/
export HAZELCAST_SHARED_LIB_NAME=$(echo *dylib)
install_name_tool -id ${HAZELCAST_SHARED_LIB_NAME} ${HAZELCAST_SHARED_LIB_NAME}

cd ../../../../

# Uncomment below if you want to generate doxygen docs
##mkdir -p ./cpp/docs/

#echo "Generating docs "
#doxygen docsConfig

#echo "Moving docs to target"
#mv  docs/ cpp

echo "Removing temporary files"
rm -rf ./ReleaseShared
rm -rf ./ReleaseSharedTLS
rm -rf ./ReleaseStatic
rm -rf ./ReleaseStaticTLS

# Verify release
scripts/verifyReleaseOSX.sh
