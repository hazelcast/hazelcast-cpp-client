echo "Compiling Static Library"
mkdir ReleaseStatic
cd ReleaseStatic
cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
make
cd ..

echo "Compiling Shared Library"
mkdir ReleaseShared
cd ReleaseShared
cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
make
cd ..

#STANDART PART
mkdir -p ./cpp/Mac_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Mac_64/hazelcast/lib
mkdir -p ./cpp/Mac_64/external/include
mkdir -p ./cpp/Mac_64/external/lib
mkdir -p ./cpp/Mac_64/examples/

echo "Moving headers to target"
cp -R hazelcast/include/hazelcast/ cpp/Mac_64/hazelcast/include/hazelcast/
echo "Moving libraries to target"
cp ReleaseStatic/libHazelcastClient*.a cpp/Mac_64/hazelcast/lib/
cp ReleaseShared/libHazelcastClient*.dylib cpp/Mac_64/hazelcast/lib/

echo "Moving dependencies to target"
cp -R external/include/ cpp/Mac_64/external/include/
cp -R external/lib/darwin/64/ cpp/Mac_64/external/lib/

echo "Moving examples to target"
cp examples/*cpp cpp/Mac_64/examples/
cp ReleaseStatic/examples/*exe cpp/Mac_64/examples/

#MAC SPECIFIC
#cd cpp/Mac_64/hazelcast/lib/
#install_name_tool -id libHazelcastClientShared_64.dylib libHazelcastClientShared_64.dylib
#cd ../../../../

#ONLY IN DEVELOPMENT MACHINE
mkdir -p ./cpp/docs/

echo "Moving docs to target"
cp -R docs/ cpp/docs/

cp enterprise-license.txt cpp/enterprise-license.txt

echo "Removing temporary files"
rm -rf ./ReleaseShared
rm -rf ./ReleaseStatic

echo "WARNING!!!!!  Do not forget to install_name_tool -id libHazelcastClient.*.dylib libHazelcastClient.*.dylib"
