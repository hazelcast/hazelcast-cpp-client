cmake_minimum_required (VERSION 2.6)
project (TestAppProject)

SET(HZ_TESTAPP_NAME testApp${HZ_LIB_TYPE}_${HZ_BIT}.exe)

add_executable(${HZ_TESTAPP_NAME} ./TestApp.cpp)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ_LIB_NAME} ${Hz_ZLIB_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	IF(${HZ_BIT} MATCHES "32")
		set_target_properties(${HZ_TESTAPP_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
	ENDIF(${HZ_BIT} MATCHES "32")

	target_link_libraries(${HZ_TESTAPP_NAME} ${HZ_LIB_NAME} ${Hz_ZLIB_LIBRARIES})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

	set_target_properties(${HZ_TESTAPP_NAME} PROPERTIES COMPILE_FLAGS " /EHsc ")
	message(STATUS "${CMAKE_BUILD_TYPE} ")
	IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
		target_link_libraries(${HZ_TESTAPP_NAME} ${HZ_LIB_NAME} ${Hz_ZLIB_LIBRARIES} )
	ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

	IF(${CMAKE_BUILD_TYPE} MATCHES "Release")
		target_link_libraries(${HZ_TESTAPP_NAME}  ${HZ_LIB_NAME} ${Hz_ZLIB_LIBRARIES} )
	ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Release")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
