cmake_minimum_required (VERSION 2.6)
project (HazelcastClientTest)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")

include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/hazelcast/test)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_executable(clientTestStatic_64.exe ${HZ_TEST_SOURCES})
	add_executable(clientTestShared_64.exe ${HZ_TEST_SOURCES})

	target_link_libraries(clientTestStatic_64.exe HazelcastClientStatic_64 ${Hz_Boost_LIBRARIES} ${ZLIB_LIBRARIES})
    target_link_libraries(clientTestShared_64.exe HazelcastClientShared_64 ${Hz_Boost_LIBRARIES} ${ZLIB_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_executable(clientTestStatic_32.exe ${HZ_TEST_SOURCES})
	add_executable(clientTestShared_32.exe ${HZ_TEST_SOURCES})

	add_executable(clientTestStatic_64.exe ${HZ_TEST_SOURCES})
	add_executable(clientTestShared_64.exe ${HZ_TEST_SOURCES})
	
	set_target_properties(clientTestStatic_32.exe PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	set_target_properties(clientTestShared_32.exe PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
		
	target_link_libraries(clientTestStatic_32.exe HazelcastClientStatic_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(clientTestShared_32.exe HazelcastClientShared_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(clientTestStatic_64.exe HazelcastClientStatic_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(clientTestShared_64.exe HazelcastClientShared_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#	add_executable(clientTestStatic_32.exe ${HZ_TEST_SOURCES})
#	add_executable(clientTestShared_32.exe ${HZ_TEST_SOURCES})
	add_executable(clientTestStatic_64.exe ${HZ_TEST_SOURCES})
#	add_executable(clientTestShared_64.exe ${HZ_TEST_SOURCES})
	
	set_target_properties(clientTestStatic_64.exe PROPERTIES COMPILE_FLAGS " /EHsc ")
#	set_target_properties(clientTestShared_64.exe PROPERTIES COMPILE_FLAGS " /EHsc " LINK_FLAGS "  /EHsc ")
#	set_target_properties(clientTestStatic_32.exe PROPERTIES COMPILE_FLAGS "/p:PlatformTarget=x86" LINK_FLAGS "/p:PlatformTarget=x86")
#	set_target_properties(clientTestShared_32.exe PROPERTIES COMPILE_FLAGS "/p:PlatformTarget=x86" LINK_FLAGS "/p:PlatformTarget=x86")
		
#	target_link_libraries(clientTestStatic_32.exe HazelcastClientStatic_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
#	target_link_libraries(clientTestShared_32.exe HazelcastClientShared_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(clientTestStatic_64.exe HazelcastClientStatic_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
#	target_link_libraries(clientTestShared_64.exe HazelcastClientShared_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

