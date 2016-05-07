# Hazelcast C++ Client

[![Build Status](https://travis-ci.org/maxbeutel/hazelcast-cpp-client.svg?branch=upstream-pr-travis-ci)](https://travis-ci.org/maxbeutel/hazelcast-cpp-client)

C++ client implementation for [Hazelcast](https://github.com/hazelcast/hazelcast), the open source in-memory data grid. A comparison of features supported by the C++ Client vs the Java client can be found [here](http://docs.hazelcast.org/docs/latest-dev/manual/html-single/index.html#hazelcast-clients-feature-comparison).


---
*This is a new client being developed for the Hazelcast Open Client Protocol v1.0. This work is incomplete but is expected to be finalized around March 2016.*  

*We do not recommend you fork this client until it is complete and passes all tests.* 

*If you would like to assist with the development of this C++ client, please feel free to fork and send us pull requests. Please complete the [Hazelcast Contributor Agreement](https://hazelcast.atlassian.net/wiki/display/COM/Hazelcast+Contributor+Agreement) if you plan to do this. In the meantime we have published our C++ client which uses the older Hazelcast client protocol on our downloads page where it can be freely used.* 

---

## Documentation

You can generate API Documentation via doxygen from root with the following command.

```
doxygen docsConfig
```

## How to download

Clone the project from github:

git clone --recursive git@github.com:hazelcast/hazelcast-cpp-client.git

We use --recursive flag for our dependency on googletest framework.

## How to build

First create a build directory from the root of the project. In the build directory, run the following commands depending on your environment.

### For mac release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

### For mac code coverage

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_CODE_COVERAGE=ON
	gcovr -u -e .*external.* -e SimpleMapTest.h --html --html-details -o coverage.html 

### For mac Xcode development

  	cmake .. -G Xcode -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug

	cmake .. -G Xcode -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=archive -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=library

### valgrind sample run with suppressions

        valgrind --leak-check=yes  --gen-suppressions=all --suppressions=../test.sup  ./hazelcast/test/clientTest_STATIC_64.exe  > log.t

### For linux release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

### For building the tests

    Add the -DHZ_BUILD_TESTS=ON flag to the cmake flags. e.g.:
    cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_BUILD_TESTS=ON

### For building the examples

    Add the -DHZ_BUILD_EXAMPLES=ON flag to the cmake flags. e.g.:
    cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_BUILD_EXAMPLES=ON

### For linux valgrind

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_VALGRIND=ON

### For windows release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

	MSBuild.exe HazelcastClient.sln /property:Configuration=Release

### For windows debug

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug

	MSBuild.exe HazelcastClient.sln /property:TreatWarningsAsErrors=true /property:Configuration=Debug

## Mail Group

Please join the mail group if you are interested in using or developing Hazelcast.

[http://groups.google.com/group/hazelcast](http://groups.google.com/group/hazelcast)

## License

Hazelcast C++ Client is available under the Apache 2 License. Please see the [Licensing appendix](http://docs.hazelcast.org/docs/latest/manual/html-single/hazelcast-documentation.html#license-questions) for more information.

## Copyright

Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.

Visit [www.hazelcast.com](http://www.hazelcast.com/) for more info.
