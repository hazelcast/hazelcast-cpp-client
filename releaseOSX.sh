#STANDART PART
mkdir -p ./cpp/Mac_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Mac_64/hazelcast/lib
mkdir -p ./cpp/Mac_64/external/include
mkdir -p ./cpp/Mac_64/external/lib

cp -R hazelcast/include/hazelcast/ cpp/Mac_64/hazelcast/include/hazelcast/
cp build/libHazelcastClientShared_64.dylib cpp/Mac_64/hazelcast/lib/libHazelcastClientShared_64.dylib
cp build/libHazelcastClientStatic_64.a     cpp/Mac_64/hazelcast/lib/libHazelcastClientStatic_64.a

cp -R external/include/ cpp/Mac_64/external/include/
cp -R external/lib/darwin/64/ cpp/Mac_64/external/lib/

#MAC SPECIFIC
cd cpp/Mac_64/hazelcast/lib/
install_name_tool -id libHazelcastClientShared_64.dylib libHazelcastClientShared_64.dylib
cd ../../../../

#ONLY IN DEVELOPMENT MACHINE
mkdir -p ./cpp/docs/
mkdir -p ./cpp/examples/

cp -R docs/ cpp/docs/
cp -R examples/ cpp/examples/

cp enterprise-license.txt cpp/enterprise-license.txt


