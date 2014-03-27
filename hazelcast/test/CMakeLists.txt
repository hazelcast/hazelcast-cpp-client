cmake_minimum_required (VERSION 2.6)
project (HazelcastClientTest)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/hazelcast/test)

#set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)

SET(HZ_EXE_NAME clientTest_${HZ_LIB_TYPE}_${HZ_BIT}.exe)

add_executable(${HZ_EXE_NAME} ${HZ_TEST_SOURCES})

IF(${HZ_CODE_COVERAGE} MATCHES "ON")
	message(STATUS "Code coverage is on")
	set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -fPIC -O0" LINK_FLAGS "-fprofile-arcs") #for code coverage tool
ENDIF(${HZ_CODE_COVERAGE} MATCHES "ON")

IF(${HZ_VALGRIND} MATCHES "ON")
	message(STATUS "Valgrind is on")
	set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS "-O0" LINK_FLAGS "-O0")#for valgrind
ENDIF(${HZ_VALGRIND} MATCHES "ON")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

	target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME} ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${HZ_BIT} MATCHES "32")
		set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	ENDIF(${HZ_BIT} MATCHES "32")

	target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME} ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	set_target_properties(${HZ_EXE_NAME} PROPERTIES COMPILE_FLAGS " /EHsc ")
	message(STATUS "ddddddd ${CMAKE_BUILD_TYPE} ")
	IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_libraries(${HZ_EXE_NAME} ${HZ_LIB_NAME} ${Hz_Boost_LIBRARIES_debug} ${Hz_ZLIB_LIBRARIES} )
	ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	
	IF(${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_link_libraries(${HZ_EXE_NAME}  ${HZ_LIB_NAME} ${Hz_Boost_LIBRARIES_release} ${Hz_ZLIB_LIBRARIES} )
	ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Release")
	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
