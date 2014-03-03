cmake_minimum_required (VERSION 2.6)
project (TestApp)

include_directories( ${CMAKE_SOURCE_DIR}/external/include/)
include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include )

message(STATUS "${CMAKE_SYSTEM}")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "APPLE ENVIRONMENT DETECTED")
	set(HZ_LIB_ROOT ${CMAKE_SOURCE_DIR}/../Mac_64)
	
	add_executable(TestApp.exe ./TestApp.cpp)
					
	set(Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_date_time.a)

	set(Hz_ZLIB_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libz.a)
	set(Hazelcast_LIBRARIES ${HZ_LIB_ROOT}/hazelcast/lib/libHazelcastClientStatic_64.a)	

	target_link_libraries(TestApp.exe  ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES} ${HZ_LIB_ROOT})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "LINUX ENVIRONMENT DETECTED")
	set(HZ_LIB_ROOT ${CMAKE_SOURCE_DIR}/../Linux_32)
	
	add_executable(TestApp.exe ./TestApp.cpp)
					
	set(Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_date_time.a)

	set(Hz_ZLIB_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libz.a)
	set(Hazelcast_LIBRARIES ${HZ_LIB_ROOT}/hazelcast/lib/libHazelcastClientStatic_32.a)	

	target_link_libraries(TestApp.exe  ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES} ${HZ_LIB_ROOT})	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "WINDOWS ENVIRONMENT DETECTED ${CMAKE_GENERATOR}")
	add_definitions(-DWIN32 -D_WIN32 -D__WIN32__ -DHAZELCAST_EXPORTS)
	
	IF(${CMAKE_GENERATOR} MATCHES "Win64")
		set(Hz_Bit "64")
	ELSE(${CMAKE_GENERATOR} MATCHES "Win64")
		set(Hz_Bit "32")
	ENDIF(${CMAKE_GENERATOR} MATCHES "Win64")
	
	set(HZ_LIB_ROOT ${CMAKE_SOURCE_DIR}/../Windows_${Hz_Bit})
	
	add_executable(TestApp.exe ./TestApp.cpp)
					
	set(Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_date_time.a)

	set(Hz_ZLIB_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${Hz_Bit}/zlibstaticd.lib)
	set(Hazelcast_LIBRARIES ${HZ_LIB_ROOT}/hazelcast/lib/libHazelcastClientStatic_${Hz_Bit}.a)	

	
	set_target_properties(TestApp.exe PROPERTIES COMPILE_FLAGS " /EHsc ")

	target_link_libraries(TestApp.exe  ${Hazelcast_LIBRARIES} optimized ${Hz_Boost_LIBRARIES_release} ${Hz_ZLIB_LIBRARIES} debug ${Hz_Boost_LIBRARIES_debug} ${Hz_ZLIB_LIBRARIES} )		
	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")


#enable_testing()

#IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#   add_test (NAME TestStatic COMMAND clientTestStatic_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
#    add_test (NAME TestShared COMMAND clientTestShared_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
#ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    add_test (NAME TestStatic_32 COMMAND clientTestStatic_32.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
#    add_test (NAME TestStatic_64 COMMAND clientTestStatic_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
#    add_test (NAME TestShared_32 COMMAND clientTestShared_32.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
#    add_test (NAME TestShared_64 COMMAND clientTestShared_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
#ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
