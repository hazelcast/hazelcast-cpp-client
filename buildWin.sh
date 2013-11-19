Remove-Item -Force -Recurse build/;
Remove-Item ./hazelcast/test/java/clientTest;
mkdir build;
cd build;
cmake ..;
msbuild /verbosity:quiet  /property:Configuration=Debug /property:Platform=x86 /AdditionalLibPaths=.
../hazelcast/test/java/clientTest;
