cmake_minimum_required (VERSION 2.6)
project (hz_cppClient_test)

FUNCTION(HEADER_DIRECTORIES result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
      IF(IS_DIRECTORY ${curdir}/${child})
          SET(dirlist ${dirlist} ${curdir}/${child})		 	
      ENDIF()
    ENDFOREACH()

	FOREACH(dir ${dirlist})
		HEADER_DIRECTORIES(subdirs ${dir})
		SET(dirlist ${dirlist} ${subdirs})		 		
	ENDFOREACH()

    SET(${result} ${dirlist} PARENT_SCOPE) 

ENDFUNCTION()

HEADER_DIRECTORIES(SUBDIRS ${CMAKE_SOURCE_DIR}/../)

FILE(GLOB_RECURSE HZ_TEST_SOURCES "./*cpp")
add_library(cppClientTests ${HZ_TEST_SOURCES})

include_directories(${SUBDIRS})
include_directories("${CMAKE_SOURCE_DIR}/../../external")

# add imported targets
add_library(boost_atomic STATIC IMPORTED)
set_property(TARGET boost_atomic PROPERTY IMPORTED_LOCATION /usr/local/lib/libboost_atomic.a)
add_library(boost_thread STATIC IMPORTED)
set_property(TARGET boost_thread PROPERTY IMPORTED_LOCATION /usr/local/lib/libboost_thread.a)
add_library(boost_system STATIC IMPORTED)
set_property(TARGET boost_system PROPERTY IMPORTED_LOCATION /usr/local/lib/libboost_system.a)
add_library(boost_chrono STATIC IMPORTED)
set_property(TARGET boost_chrono PROPERTY IMPORTED_LOCATION /usr/local/lib/libboost_chrono.a)
#add_library(zlib STATIC IMPORTED)
#set_property(TARGET zlib PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../build/external/zlib/libz.a)
#add_library(cppClient STATIC IMPORTED)
#set_property(TARGET cppClient PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../build/libcppClient.a)
				
add_executable(clientTest "${CMAKE_SOURCE_DIR}/hazelcast/test/main.cpp")
target_link_libraries(clientTest cppClientTests cppClient boost_atomic boost_thread boost_system boost_chrono zlib)
