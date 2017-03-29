#!/usr/bin/env bash
set -e #abort the script at first failure

echo "Cleanup release directories"
rm -rf ./Release*
rm -rf ./cpp

echo "Compiling Static 32bit library"
mkdir ReleaseStatic32;
cd ./ReleaseStatic32;
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4 VERBOSE=1;
cd ..;

echo "Compiling Static 32bit library with TLS support"
mkdir ReleaseStatic32TLS;
cd ./ReleaseStatic32TLS;
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
make -j 4 VERBOSE=1;
cd ..;

echo "Compiling Shared 32bit library"
mkdir ReleaseShared32;
cd ./ReleaseShared32;
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4 VERBOSE=1;
cd ..;

echo "Compiling Shared 32bit library with TLS support"
mkdir ReleaseShared32TLS;
cd ./ReleaseShared32TLS;
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
make -j 4 VERBOSE=1;
cd ..;


echo "Moving 32bit libraries to target"
mkdir -p ./cpp/Linux_32/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_32/hazelcast/lib/tls

cp -R hazelcast/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp ReleaseShared32/libHazelcastClient* cpp/Linux_32/hazelcast/lib/
cp ReleaseShared32TLS/libHazelcastClient* cpp/Linux_32/hazelcast/lib/tls/
cp ReleaseStatic32/libHazelcastClient* cpp/Linux_32/hazelcast/lib/
cp ReleaseStatic32TLS/libHazelcastClient* cpp/Linux_32/hazelcast/lib/tls/

echo "Copying external libraries and the examples"
mkdir -p cpp/examples
cp -r examples cpp/examples/src

echo "Linking to external libraries and examples for 32-bit release"
cd cpp/Linux_32
ln -s ../examples .
ln -s ../external .
cd -

echo "Clearing temporary 32bit libraries"
rm -rf ./ReleaseShared32
rm -rf ./ReleaseShared32TLS
rm -rf ./ReleaseStatic32
rm -rf ./ReleaseStatic32TLS

echo "Compiling Static 64bit library"
mkdir ReleaseStatic64;
cd ./ReleaseStatic64;
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4 VERBOSE=1;
cd ..;

echo "Compiling Static 64bit library with TLS support"
mkdir ReleaseStatic64TLS;
cd ./ReleaseStatic64TLS;
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
make -j 4 VERBOSE=1;
cd ..;

echo "Compiling Shared 64bit library"
mkdir ReleaseShared64;
cd ./ReleaseShared64;
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON
make -j 4 VERBOSE=1;
cd ..;

echo "Compiling Shared 64bit library with TLS support"
mkdir ReleaseShared64TLS;
cd ./ReleaseShared64TLS;
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release -DHZ_BUILD_TESTS=ON -DHZ_BUILD_EXAMPLES=ON -DHZ_COMPILE_WITH_SSL=ON
make -j 4 VERBOSE=1;
cd ..;

echo "Moving 64bit libraries to target"
mkdir -p ./cpp/Linux_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_64/hazelcast/lib/tls

cp -R hazelcast/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/
cp -R hazelcast/generated-sources/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/
cp ReleaseShared64/libHazelcastClient* cpp/Linux_64/hazelcast/lib/
cp ReleaseShared64TLS/libHazelcastClient* cpp/Linux_64/hazelcast/lib/tls/
cp ReleaseStatic64/libHazelcastClient* cpp/Linux_64/hazelcast/lib/
cp ReleaseStatic64TLS/libHazelcastClient* cpp/Linux_64/hazelcast/lib/tls/

echo "Linking to external libraries and examples for 64-bit release"
cd cpp/Linux_64
ln -s ../examples .
ln -s ../external .
cd -

echo "Clearing temporary 64bit libraries"
rm -rf ./ReleaseShared64
rm -rf ./ReleaseShared64TLS
rm -rf ./ReleaseStatic64
rm -rf ./ReleaseStatic64TLS

# Verify release
scripts/verifyReleaseLinux.sh

