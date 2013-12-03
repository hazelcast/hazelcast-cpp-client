cmake_minimum_required (VERSION 2.6)
project (HazelcastClient)

include_directories( ${CMAKE_SOURCE_DIR}/external/include/)

set(ZLIB_ROOT "C:\\Users\\sancar\\Downloads\\zlib128-dll")
find_package( ZLIB REQUIRED )
include_directories( ${ZLIB_INCLUDE_DIRS} )

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
	add_library(HazelcastClientShared_32 SHARED ${HZ_SOURCES})
	add_library(HazelcastClientStatic_32 STATIC ${HZ_SOURCES})
	add_library(HazelcastClientShared_64 SHARED ${HZ_SOURCES})
	add_library(HazelcastClientStatic_64 STATIC ${HZ_SOURCES})
	
	set_target_properties(HazelcastClientShared_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	set_target_properties(HazelcastClientStatic_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	
    target_link_libraries(HazelcastClientShared_64 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(HazelcastClientShared_32 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})	
	target_link_libraries(HazelcastClientStatic_64 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(HazelcastClientStatic_32 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "WINDOWS ENVIRONMENT DETECTED")
	add_definitions(-DWIN32 -D_WIN32 -D__WIN32__)
#	add_library(HazelcastClientShared_32 SHARED ${HZ_SOURCES})
#	add_library(HazelcastClientStatic_32 STATIC ${HZ_SOURCES})
#	add_library(HazelcastClientShared_64 SHARED ${HZ_SOURCES})
	add_library(HazelcastClientStatic_64 STATIC ${HZ_SOURCES})
	
	
	set_target_properties(HazelcastClientStatic_64 PROPERTIES COMPILE_FLAGS " /EHsc " )
#	set_target_properties(HazelcastClientShared_64 PROPERTIES COMPILE_FLAGS " /EHsc " LINK_FLAGS "  /EHsc ")
    #set_target_properties(HazelcastClientStatic_32 PROPERTIES COMPILE_FLAGS "/p:PlatformTarget=x86" LINK_FLAGS "/p:PlatformTarget=x86")
	#set_target_properties(HazelcastClientShared_32 PROPERTIES COMPILE_FLAGS "/p:PlatformTarget=x86" LINK_FLAGS "/p:PlatformTarget=x86")

#	target_link_libraries(HazelcastClientShared_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
#	target_link_libraries(HazelcastClientShared_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})	
	target_link_libraries(HazelcastClientStatic_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
#	target_link_libraries(HazelcastClientStatic_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})	
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
