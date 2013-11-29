cmake_minimum_required (VERSION 2.6)
project (HazelcastClientTest)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/hazelcast/test)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_executable(clientTestStatic_64.exe ${HZ_TEST_SOURCES})
	add_executable(clientTestShared_64.exe ${HZ_TEST_SOURCES})

	target_link_libraries(clientTestStatic_64.exe HazelcastClientStatic_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
    target_link_libraries(clientTestShared_64.exe HazelcastClientShared_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})

ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties(clientTestStatic_32.exe PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	set_target_properties(clientTestShared_64.exe PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	
	add_executable(clientTestStatic_32.exe ${HZ_TEST_SOURCES})
	add_executable(clientTestShared_32.exe ${HZ_TEST_SOURCES})

	add_executable(clientTestStatic_64.exe ${HZ_TEST_SOURCES})
	add_executable(clientTestShared_64.exe ${HZ_TEST_SOURCES})
		
	target_link_libraries(clientTestStatic_32.exe HazelcastClientStatic_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(clientTestShared_32.exe HazelcastClientShared_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(clientTestStatic_64.exe HazelcastClientStatic_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(clientTestShared_64.exe HazelcastClientShared_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
