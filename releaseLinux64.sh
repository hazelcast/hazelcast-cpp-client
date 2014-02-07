mkdir -p ./releaseBundle64/hazelcast/lib
mkdir -p ./releaseBundle64/hazelcast/include/hazelcast/
mkdir -p ./releaseBundle64/external/lib
mkdir -p ./releaseBundle64/external/include
mkdir -p ./releaseBundle64/docs/

cp build/libHazelcastClientShared_64.so releaseBundle64/hazelcast/lib/libHazelcastClientShared_64.so
cp build/libHazelcastClientStatic_64.a  releaseBundle64/hazelcast/lib/libHazelcastClientStatic_64.a

cp -R docs/ releaseBundle64/docs/
cp -R external/include/ releaseBundle64/external/include/
cp -R hazelcast/include/hazelcast/ releaseBundle64/hazelcast/include/hazelcast/
cp -R external/lib/linux/64/ releaseBundle64/external/lib/
cp enterprise-license.txt releaseBundle64/enterprise-license.txt

ln -s ./docs/html/index.html releaseBundle64/readme.html

