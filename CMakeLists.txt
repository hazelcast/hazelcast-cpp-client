cmake_minimum_required (VERSION 2.6)
project (HazelcastClient)

# FLAGS
#  -DHZ_LIB_TYPE=[STATIC | SHARED]
#  -DHZ_BIT=[32 | 64]
#  -DHZ_CODE_COVERAGE=ON
#  -DHZ_VALGRIND=ON
#  -DHZ_ADDRESS_SANITIZER=ON

SET(HZ_VERSION 3.5-SNAPSHOT)

message(STATUS "Preparing hazelcast client ..... ")

include_directories( ${CMAKE_SOURCE_DIR}/external/include/)


FILE(GLOB_RECURSE HZ_SOURCES "./hazelcast/src/*cpp")
FILE(GLOB_RECURSE HZ_HEADERS "./hazelcast/include/*h")

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include )

IF(NOT (${HZ_BIT} MATCHES "32") AND NOT (${HZ_BIT} MATCHES "64") )
  message( FATAL_ERROR "Build needs HZ_BIT. set -DHZ_BIT=32 or -DHZ_BIT=64" )
ENDIF(NOT (${HZ_BIT} MATCHES "32") AND NOT (${HZ_BIT} MATCHES "64"))

message(STATUS "${HZ_BIT} Bit")

IF(NOT (${HZ_LIB_TYPE} MATCHES "STATIC") AND NOT (${HZ_LIB_TYPE} MATCHES "SHARED") )
	message( FATAL_ERROR "Build needs HZ_LIB_TYPE. set -DHZ_LIB_TYPE=STATIC or -DHZ_LIB_TYPE=SHARED" )
ENDIF(NOT (${HZ_LIB_TYPE} MATCHES "STATIC") AND NOT (${HZ_LIB_TYPE} MATCHES "SHARED") )

message(STATUS "Generating ${HZ_LIB_TYPE} Library")

SET(HZ_LIB_NAME HazelcastClient${HZ_VERSION}_${HZ_BIT})

add_library(${HZ_LIB_NAME} ${HZ_LIB_TYPE} ${HZ_SOURCES} ${HZ_HEADERS})

IF(${HZ_CODE_COVERAGE} MATCHES "ON")
	message(STATUS "Code coverage is on")
	set_target_properties(${HZ_LIB_NAME} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -fPIC -O0" LINK_FLAGS "-fprofile-arcs") #for code coverage tool
ENDIF(${HZ_CODE_COVERAGE} MATCHES "ON")

IF(${HZ_VALGRIND} MATCHES "ON")
	message(STATUS "Valgrind is on")
	set_target_properties(${HZ_LIB_NAME} PROPERTIES COMPILE_FLAGS "-O0" LINK_FLAGS "-O0")#for valgrind
ENDIF(${HZ_VALGRIND} MATCHES "ON")


message(STATUS "${CMAKE_SYSTEM}")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "APPLE ENVIRONMENT DETECTED")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror -g -rdynamic  ")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")


	set(Hz_ZLIB_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/darwin/${HZ_BIT}/libz.a)

	target_link_libraries(${HZ_LIB_NAME} ${Hz_ZLIB_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "LINUX ENVIRONMENT DETECTED")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror ")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

	set(Hz_ZLIB_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/linux/${HZ_BIT}/libz.a)

	IF(${HZ_BIT} MATCHES "32")
		set_target_properties(${HZ_LIB_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	ENDIF(${HZ_BIT} MATCHES "32")

	target_link_libraries(${HZ_LIB_NAME} pthread rt ${Hz_ZLIB_LIBRARIES})


	IF(${HZ_ADDRESS_SANITIZER} MATCHES "ON")
	  message(STATUS "Address Sanitizer is on")
	  set_target_properties(${HZ_LIB_NAME} PROPERTIES COMPILE_FLAGS "-O1 -fsanitize=address -fno-omit-frame-pointer " LINK_FLAGS "-O1 -fsanitize=address -fno-omit-frame-pointer ")

	  # use, i.e. don't skip the full RPATH for the build tree
	  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

	  # when building, don't use the install RPATH already
	  # (but later on when installing)
	  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

	  SET(CMAKE_INSTALL_RPATH "/usr/lib/gcc/x86_64-linux-gnu/4.8/")

	  target_link_libraries(${HZ_LIB_NAME} asan)	
	ENDIF(${HZ_ADDRESS_SANITIZER} MATCHES "ON")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "WINDOWS ENVIRONMENT DETECTED ${CMAKE_GENERATOR} ${CMAKE_BUILD_TYPE} ")
	add_definitions(-D__WIN${HZ_BIT}__ -DWIN${HZ_BIT} -D_WIN${HZ_BIT} -DHAZELCAST_EXPORTS)

	set(Hz_ZLIB_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${HZ_BIT}/zlibstaticd.lib)

	set_target_properties(${HZ_LIB_NAME} PROPERTIES COMPILE_FLAGS " /EHsc " )

	IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_libraries(${HZ_LIB_NAME} ${Hz_ZLIB_LIBRARIES} )
	ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

	IF(${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_link_libraries(${HZ_LIB_NAME} ${Hz_ZLIB_LIBRARIES} )
	ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Release")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

ADD_SUBDIRECTORY(hazelcast/test)
ADD_SUBDIRECTORY(examples)
