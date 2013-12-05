cmake_minimum_required (VERSION 2.6)
project (HazelcastClient)

include_directories( ${CMAKE_SOURCE_DIR}/external/include/)

include(GenerateExportHeader)

message(STATUS "Preparing hazelcast client ..... ")
FILE(GLOB_RECURSE HZ_SOURCES "./hazelcast/src/*cpp")
include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include )

message(STATUS "${CMAKE_SYSTEM}")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "APPLE ENVIRONMENT DETECTED")
	add_library(HazelcastClientShared_64 SHARED ${HZ_SOURCES})
	add_library(HazelcastClientStatic_64 STATIC ${HZ_SOURCES})
	
	set(Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/darwin/64/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/darwin/64/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/darwin/64/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/darwin/64/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/darwin/64/libboost_date_time.a)

    	target_link_libraries(HazelcastClientShared_64 ${Hz_Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(HazelcastClientStatic_64 ${Hz_Boost_LIBRARIES} ${ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "LINUX ENVIRONMENT DETECTED")

	set(Hz_Boost_LIBRARIES_64 ${CMAKE_SOURCE_DIR}/external/lib/linux/64/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES_64 ${CMAKE_SOURCE_DIR}/external/lib/linux/64/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES_64 ${CMAKE_SOURCE_DIR}/external/lib/linux/64/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES_64 ${CMAKE_SOURCE_DIR}/external/lib/linux/64/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES_64 ${CMAKE_SOURCE_DIR}/external/lib/linux/64/libboost_date_time.a)
	set(Hz_ZLIB_LIBRARIES_64 ${CMAKE_SOURCE_DIR}/external/lib/linux/64/zlib.a)

	set(Hz_Boost_LIBRARIES_32 ${CMAKE_SOURCE_DIR}/external/lib/linux/32/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES_32 ${CMAKE_SOURCE_DIR}/external/lib/linux/32/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES_32 ${CMAKE_SOURCE_DIR}/external/lib/linux/32/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES_32 ${CMAKE_SOURCE_DIR}/external/lib/linux/32/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES_32 ${CMAKE_SOURCE_DIR}/external/lib/linux/32/libboost_date_time.a)
	set(Hz_ZLIB_LIBRARIES_32 ${CMAKE_SOURCE_DIR}/external/lib/linux/32/zlib.a)

	add_library(HazelcastClientShared_32 SHARED ${HZ_SOURCES})
	add_library(HazelcastClientStatic_32 STATIC ${HZ_SOURCES})
	add_library(HazelcastClientShared_64 SHARED ${HZ_SOURCES})
	add_library(HazelcastClientStatic_64 STATIC ${HZ_SOURCES})
	
	set_target_properties(HazelcastClientShared_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	set_target_properties(HazelcastClientStatic_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	
    	target_link_libraries(HazelcastClientShared_64 pthread rt ${Hz_Boost_LIBRARIES_64} ${Hz_ZLIB_LIBRARIES_64})
	target_link_libraries(HazelcastClientShared_32 pthread rt ${Hz_Boost_LIBRARIES_32} ${Hz_ZLIB_LIBRARIES_32})	
	target_link_libraries(HazelcastClientStatic_64 pthread rt ${Hz_Boost_LIBRARIES_64} ${Hz_ZLIB_LIBRARIES_64})
	target_link_libraries(HazelcastClientStatic_32 pthread rt ${Hz_Boost_LIBRARIES_32} ${Hz_ZLIB_LIBRARIES_32})	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "WINDOWS ENVIRONMENT DETECTED ${CMAKE_GENERATOR}")
	add_definitions(-DWIN32 -D_WIN32 -D__WIN32__ -DHAZELCAST_EXPORTS)
	
	IF(${CMAKE_GENERATOR} MATCHES "Win64")
		set(Hz_Bit "64")
	ELSE(${CMAKE_GENERATOR} MATCHES "Win64")
		set(Hz_Bit "32")
	ENDIF(${CMAKE_GENERATOR} MATCHES "Win64")
	
	set(Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${Hz_Bit}/libboost_atomic-vc100-mt-gd-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${Hz_Bit}/libboost_system-vc100-mt-gd-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${Hz_Bit}/libboost_chrono-vc100-mt-gd-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${Hz_Bit}/libboost_thread-vc100-mt-gd-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${Hz_Bit}/libboost_date_time-vc100-mt-gd-1_55.lib)

	set(Hz_ZLIB_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/windows/${Hz_Bit}/zlibstaticd.lib)
	
	
	add_library(HazelcastClientStatic STATIC ${HZ_SOURCES})
	add_library(HazelcastClientShared SHARED ${HZ_SOURCES})
	
	GENERATE_EXPORT_HEADER( HazelcastClientShared
             BASE_NAME HazelcastClientShared
             EXPORT_MACRO_NAME HazelcastClientShared_EXPORT
             EXPORT_FILE_NAME HazelcastClientShared_Export.h
             STATIC_DEFINE HazelcastClientShared_BUILT_AS_STATIC
	)
#	set_target_properties(HazelcastClientShared PROPERTIES SUFFIX "1.dll")
#	set_target_properties(HazelcastClientShared PROPERTIES DEFINE_SYMBOL HzClientDLL)
#
	set_target_properties(HazelcastClientShared PROPERTIES COMPILE_FLAGS " /EHsc " )    	
	set_target_properties(HazelcastClientStatic PROPERTIES COMPILE_FLAGS " /EHsc " )

	target_link_libraries(HazelcastClientShared ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES})
	target_link_libraries(HazelcastClientStatic ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES})
	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

ADD_SUBDIRECTORY(hazelcast/test)

enable_testing()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_test (NAME TestStatic COMMAND clientTestStatic_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
    add_test (NAME TestShared COMMAND clientTestShared_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_test (NAME TestStatic_32 COMMAND clientTestStatic_32.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
    add_test (NAME TestStatic_64 COMMAND clientTestStatic_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
    add_test (NAME TestShared_32 COMMAND clientTestShared_32.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
    add_test (NAME TestShared_64 COMMAND clientTestShared_64.exe WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/java/)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
