#Dependencies:
    - [google benchmark](https://github.com/google/benchmark): Mac OS: brew install google-benchmark
    - hazelcast-cpp-client 
    - Boost

#Build Instructions:
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/path/to/boost/install/dir;/path/to/hazelcast-cpp-client/install/dir
make

#Running Instructions
Run inside build directory as:
```shell
./hazelcast_benchmark
```

Print the options:
```shell
./hazelcast_benchmark --help
```
You can play with the number of repetitions, output format, aggregates, minimum bencmark time, etc.

The test is using 32 threads by default, you can change it in the source code.
