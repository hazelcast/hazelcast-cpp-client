# For mac release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

# For mac code coverage

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_CODE_COVERAGE=ON
	gcovr -u -e .*external.* -e SimpleMapTest.h --html --html-details -o coverage.html 

# For mac Xcode development

  	cmake .. -G Xcode -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug

# For Linux Address Sanitizer

        cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_ADDRESS_SANITIZER=ON

# valgrind sample run with suppresions

        valgrind --leak-check=yes  --gen-suppressions=all --suppressions=../test.sup  ./hazelcast/test/clientTest_STATIC_64.exe  > log.t

# For linux release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

# For linux valgrind

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug -DHZ_VALGRIND=ON

# For windows release

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Release

	MSBuild.exe HazelcastClient.sln /property:Configuration=Release

# For windows debug

	cmake .. -DHZ_LIB_TYPE=STATIC -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -DHZ_LIB_TYPE=SHARED -DHZ_BIT=32 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=STATIC -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug
	cmake .. -G "Visual Studio 10 Win64"  -DHZ_LIB_TYPE=SHARED -DHZ_BIT=64 -DCMAKE_BUILD_TYPE=Debug

	MSBuild.exe HazelcastClient.sln /property:TreatWarningsAsErrors=true /property:Configuration=Debug
