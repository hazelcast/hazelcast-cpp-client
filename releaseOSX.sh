mkdir -p ./releaseBundle/hazelcast/lib
mkdir -p ./releaseBundle/hazelcast/include/hazelcast/
mkdir -p ./releaseBundle/external/lib
mkdir -p ./releaseBundle/external/include
mkdir -p ./releaseBundle/docs/

cp build/libHazelcastClientShared_64.dylib releaseBundle/hazelcast/lib/libHazelcastClientShared_64.dylib
cp build/libHazelcastClientStatic_64.a     releaseBundle/hazelcast/lib/libHazelcastClientStatic_64.a

cd releaseBundle/hazelcast/lib/
install_name_tool -id libHazelcastClientShared_64.dylib libHazelcastClientShared_64.dylib
cd ../../../

ln -s docs/html/index.html releaseBundle/readme.html
cp -R docs/ releaseBundle/docs/
cp -R external/include/ releaseBundle/external/include/
cp -R hazelcast/include/hazelcast/ releaseBundle/hazelcast/include/hazelcast/
cp -R external/lib/darwin/64/ releaseBundle/external/lib/

