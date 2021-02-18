cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    #required to suppress the warning:
    # `gtest-death-test.cc:1301:24: error: 'dummy' may be used uninitialized [-Werror=maybe-uninitialized]`
    #for googletest source compilation
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-maybe-uninitialized")
endif ()

find_package(GTest 1.10)

if (GTest_FOUND)
    set(GTEST_TARGET GTest::gtest)
else()
    message(STATUS "Downloading googletest from Github")

    configure_file(CMakeLists.googletest.in googletest-download/CMakeLists.txt)
    execute_process(
            COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
    )
    if (result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(
            COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
    )
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL
    )

    set(GTEST_TARGET gtest)
endif()

add_subdirectory(src)
