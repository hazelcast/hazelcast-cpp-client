cmake_minimum_required (VERSION 2.6)
project (TestAppProject)

SET(HZ_TESTAPP_NAME testApp${HZ_LIB_TYPE}_${HZ_BIT}.exe)

add_executable(${HZ_TESTAPP_NAME} ./TestApp.cpp)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ_LIB_NAME} )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	
	set_target_properties(${HZ_TESTAPP_NAME} PROPERTIES COMPILE_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_COMPILE_FLAGS} ${HZ_VALGRIND_COMPILE_FLAGS}" LINK_FLAGS "${HZ_BIT_FLAG} ${HZ_CODE_COVERAGE_LINK_FLAGS} ${HZ_VALGRIND_LINK_FLAGS}")
	
	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ_LIB_NAME} )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	IF(${HAZELCAST_LIBRARY_TYPE} MATCHES "STATIC")
		message( STATUS "Shall use the STATIC Hazelcast library for the examples.")
		add_definitions(-DHAZELCAST_USE_STATIC)
	ELSE(${HAZELCAST_LIBRARY_TYPE} MATCHES "SHARED")
		message( STATUS "Shall use the SHARED Hazelcast library for the examples.")
		add_definitions(-DHAZELCAST_USE_SHARED)
	ENDIF(${HAZELCAST_LIBRARY_TYPE} MATCHES "STATIC")

	set_target_properties(${HZ_TESTAPP_NAME} PROPERTIES COMPILE_FLAGS " /EHsc ")
	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ_LIB_NAME}  )

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
