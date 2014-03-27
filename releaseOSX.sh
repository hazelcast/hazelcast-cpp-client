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

echo "Moving headers to target"
cp -R hazelcast/include/hazelcast/ cpp/Mac_64/hazelcast/include/hazelcast/
echo "Moving libraries to target"
cp ReleaseStatic/libHazelcastClient*.a cpp/Mac_64/hazelcast/lib/
cp ReleaseShared/libHazelcastClient*.dylib cpp/Mac_64/hazelcast/lib/

echo "Moving dependencies to target"
cp -R external/include/ cpp/Mac_64/external/include/
cp -R external/lib/darwin/64/ cpp/Mac_64/external/lib/

#MAC SPECIFIC
#cd cpp/Mac_64/hazelcast/lib/
#install_name_tool -id libHazelcastClientShared_64.dylib libHazelcastClientShared_64.dylib
#cd ../../../../

#ONLY IN DEVELOPMENT MACHINE
mkdir -p ./cpp/docs/
mkdir -p ./cpp/examples/

echo "Moving docs to target"
cp -R docs/ cpp/docs/
echo "Moving examples to target"
cp -R examples/ cpp/examples/

cp enterprise-license.txt cpp/enterprise-license.txt
echo "Do not forget to install_name_tool -id libHazelcastClientShared_64.dylib libHazelcastClientShared_64.dylib"
