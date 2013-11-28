cmake_minimum_required (VERSION 2.6)
project (HazelcastClient)

set(Boost_USE_STATIC_LIBS   ON)
find_package( Boost REQUIRED COMPONENTS atomic system chrono thread date_time)
include_directories( ${Boost_INCLUDE_DIRS})

find_package( ZLIB REQUIRED )
include_directories( ${ZLIB_INCLUDE_DIRS} )

message(STATUS "Preparing hazelcast client ..... ")
FILE(GLOB_RECURSE HZ_SOURCES "./hazelcast/src/*cpp")
include_directories(${CMAKE_SOURCE_DIR}/hazelcast/include )

add_library(hazelcastClientShared SHARED ${HZ_SOURCES})
add_library(hazelcastClientStatic STATIC ${HZ_SOURCES})

message(STATUS "${CMAKE_SYSTEM}")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "APPLE ENVIRONMENT DETECTED")
    target_link_libraries(hazelcastClientShared_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(hazelcastClientStatic_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "LINUX ENVIRONMENT DETECTED")
	set_target_properties(hazelcastClientShared_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    target_link_libraries(hazelcastClientShared_64 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(hazelcastClientShared_32 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	set_target_properties(hazelcastClientStatic_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	target_link_libraries(hazelcastClientStatic_64 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(hazelcastClientStatic_32 pthread rt ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "WINDOWS ENVIRONMENT DETECTED")
	set_target_properties(hazelcastClientShared_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    target_link_libraries(hazelcastClientShared_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(hazelcastClientShared_32 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	set_target_properties(hazelcastClientStatic_32 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	target_link_libraries(hazelcastClientStatic_64 ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
	target_link_libraries(hazelcastClientStatic_32  ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

ADD_SUBDIRECTORY(hazelcast/test)		


