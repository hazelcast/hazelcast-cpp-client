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

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/java/)
add_executable(clientTest "${CMAKE_SOURCE_DIR}/hazelcast/test/main.cpp")
target_link_libraries(clientTest cppClientTests cppClient ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
