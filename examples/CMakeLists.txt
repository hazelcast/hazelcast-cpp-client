cmake_minimum_required (VERSION 2.6)
project (TestAppProject)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/)

message(STATUS "${CMAKE_SYSTEM}")
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(STATUS "APPLE ENVIRONMENT DETECTED")
	set(HZ_LIB_ROOT ${CMAKE_SOURCE_DIR}/../Mac_64)
	include_directories(${HZ_LIB_ROOT}/external/include/)
	include_directories(${HZ_LIB_ROOT}/hazelcast/include/)
		
	add_executable(TestApp.exe ./TestApp.cpp)
					
	set(Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_date_time.a)

	set(Hz_ZLIB_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libz.a)
	set(Hazelcast_LIBRARIES ${HZ_LIB_ROOT}/hazelcast/lib/libHazelcastClientStatic_64.a)	

	target_link_libraries(TestApp.exe  ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES} ${Hazelcast_LIBRARIES})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	message(STATUS "LINUX ENVIRONMENT DETECTED")
	set(HZ_LIB_ROOT ${CMAKE_SOURCE_DIR}/../Linux_32)
	include_directories(${HZ_LIB_ROOT}/external/include/)
	include_directories(${HZ_LIB_ROOT}/hazelcast/include/)
		
	add_executable(TestApp.exe ./TestApp.cpp)
				
	set_target_properties(TestApp.exe PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
		
	set(Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_atomic.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_system.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_chrono.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_thread.a)
	LIST(APPEND Hz_Boost_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libboost_date_time.a)

	set(Hz_ZLIB_LIBRARIES ${HZ_LIB_ROOT}/external/lib/libz.a)
	set(Hazelcast_LIBRARIES ${HZ_LIB_ROOT}/hazelcast/lib/libHazelcastClientStatic_32.a)	

	target_link_libraries(TestApp.exe  ${Hz_Boost_LIBRARIES} ${Hz_ZLIB_LIBRARIES} ${Hazelcast_LIBRARIES})	
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
	include_directories(${HZ_LIB_ROOT}/external/include/)
	include_directories(${HZ_LIB_ROOT}/hazelcast/include/)
	
	add_executable(TestApp ./TestApp.cpp)
	
	set(Hz_Boost_LIBRARIES_release ${HZ_LIB_ROOT}/external/lib/libboost_atomic-vc100-mt-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES_release ${HZ_LIB_ROOT}/external/lib/libboost_system-vc100-mt-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES_release ${HZ_LIB_ROOT}/external/lib/libboost_chrono-vc100-mt-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES_release ${HZ_LIB_ROOT}/external/lib/libboost_thread-vc100-mt-1_55.lib)
	LIST(APPEND Hz_Boost_LIBRARIES_release ${HZ_LIB_ROOT}/external/lib/libboost_date_time-vc100-mt-1_55.lib)
	

	set(Hz_ZLIB_LIBRARIES ${HZ_LIB_ROOT}/external/lib/zlibstaticd.lib)
	set(Hazelcast_LIBRARIES ${HZ_LIB_ROOT}/hazelcast/lib/HazelcastClientStatic_${Hz_Bit}.lib)	

	set_target_properties(TestApp PROPERTIES COMPILE_FLAGS " /EHsc ")

	target_link_libraries(TestApp  ${Hz_Boost_LIBRARIES_release} ${Hz_ZLIB_LIBRARIES}  ${Hazelcast_LIBRARIES})
	
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
