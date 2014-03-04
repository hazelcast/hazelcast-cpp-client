#STANDART PART
mkdir -p ./cpp/Linux_64/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_64/hazelcast/lib
mkdir -p ./cpp/Linux_64/external/include
mkdir -p ./cpp/Linux_64/external/lib

cp -R hazelcast/include/hazelcast/* cpp/Linux_64/hazelcast/include/hazelcast/
cp build/libHazelcastClientShared_64.so cpp/Linux_64/hazelcast/lib/libHazelcastClientShared_64.so
cp build/libHazelcastClientStatic_64.a     cpp/Linux_64/hazelcast/lib/libHazelcastClientStatic_64.a

cp -R external/include/* cpp/Linux_64/external/include/
cp -R external/lib/linux/64/* cpp/Linux_64/external/lib/
