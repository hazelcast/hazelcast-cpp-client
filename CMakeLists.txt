cmake_minimum_required (VERSION 2.6)
project (hz_cppClient)

#SET(BOOST_LIB_DIR "${CMAKE_SOURCE_DIR}/external/lib")
SET(BOOST_INC_DIR "/usr/local/include")

SET(ZLIB_INC_DIR "${CMAKE_SOURCE_DIR}/external/zlib")
SET(ZLIB_LIB_DIR "${CMAKE_SOURCE_DIR}/build/external/zlib")


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


HEADER_DIRECTORIES(SUBDIRS ${CMAKE_SOURCE_DIR}/hazelcast/include)


FILE(GLOB_RECURSE HZ_SOURCES "./hazelcast/src/*cpp")

include_directories(${SUBDIRS} ${CMAKE_SOURCE_DIR}/hazelcast/include ${CMAKE_SOURCE_DIR}/external ${BOOST_INC_DIR} ${ZLIB_INC_DIR} ${ZLIB_LIB_DIR})

message(STATUS "Preparing zlib ..... ")
ADD_SUBDIRECTORY(external/zlib)
set_property(TARGET zlib PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/build/external/zlib/libz.a)

message(STATUS "Preparing hazelcast client ..... ")

add_library(cppClient ${HZ_SOURCES})

message(STATUS "${CMAKE_SYSTEM}")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "APPLE ENVIRONMENT DETECTED")
	target_link_libraries(cppClient zlib)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "LINUX ENVIRONMENT DETECTED")
	target_link_libraries(cppClient zlib pthread rt)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "UNSUPPORTED ENVIRONMENT")
    return()
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	
ADD_SUBDIRECTORY(hazelcast/test)

if(0)
message(STATUS "SOURCES")
foreach (file ${HZ_CLIENT_TEST_SUBDIRS})
    message(STATUS "${file}")
endforeach()
foreach (file ${HZ_CLIENT_SUBDIRS})
    message(STATUS "${file}")
endforeach()
endif(0)

if(0)
message(STATUS "DIRECTORIES")
foreach (file ${SUBDIRS})
    message(STATUS "${file}")
endforeach()

message(STATUS "SOURCES")
foreach (file ${HZ_SOURCES})
    message(STATUS "${file}")
endforeach()

message(STATUS "HEADERS")
foreach (file ${HZ_HEADERS})
    message(STATUS "${file}")
endforeach()

message(STATUS ${CMAKE_SOURCE_DIR}/hazelcast/include)
endif(0)
