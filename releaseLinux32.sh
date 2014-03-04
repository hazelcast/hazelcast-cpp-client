#STANDART PART
mkdir -p ./cpp/Linux_32/hazelcast/include/hazelcast/
mkdir -p ./cpp/Linux_32/hazelcast/lib
mkdir -p ./cpp/Linux_32/external/include
mkdir -p ./cpp/Linux_32/external/lib

cp -R hazelcast/include/hazelcast/* cpp/Linux_32/hazelcast/include/hazelcast/
cp build/libHazelcastClientShared_32.so cpp/Linux_32/hazelcast/lib/libHazelcastClientShared_32.so
cp build/libHazelcastClientStatic_32.a     cpp/Linux_32/hazelcast/lib/libHazelcastClientStatic_32.a

cp -R external/include/* cpp/Linux_32/external/include/
cp -R external/lib/linux/32/* cpp/Linux_32/external/lib/
