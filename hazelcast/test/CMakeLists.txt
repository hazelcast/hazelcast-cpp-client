cmake_minimum_required(VERSION 3.12.4)

include(FindGTest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    #required to suppress the warning:
    # `gtest-death-test.cc:1301:24: error: 'dummy' may be used uninitialized [-Werror=maybe-uninitialized]`
    #for googletest source compilation
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-maybe-uninitialized")
endif ()

find_package(GTest QUIET)
if (NOT GTest_FOUND)
    message(STATUS "No existing googletest library installation is found. Will now add googletest as external project by downloading it from github.")
    configure_file(CMakeLists.googletest.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
    if (result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL)
else()
    message(STATUS "Found an existing googletest installation and it will be used.")
endif(NOT GTest_FOUND)

add_subdirectory(src)
