mkdir -p ./releaseBundle32/hazelcast/lib
mkdir -p ./releaseBundle32/hazelcast/include/hazelcast/
mkdir -p ./releaseBundle32/external/lib
mkdir -p ./releaseBundle32/external/include
mkdir -p ./releaseBundle32/docs/

cp build/libHazelcastClientShared_32.so releaseBundle32/hazelcast/lib/libHazelcastClientShared_32.so
cp build/libHazelcastClientStatic_32.a  releaseBundle32/hazelcast/lib/libHazelcastClientStatic_32.a

cp -R docs/* releaseBundle32/docs/
cp -R external/include/* releaseBundle32/external/include/
cp -R hazelcast/include/hazelcast/* releaseBundle32/hazelcast/include/hazelcast/
cp -R external/lib/linux/32/* releaseBundle32/external/lib/

ln -s ./docs/html/index.html releaseBundle32/readme.html

