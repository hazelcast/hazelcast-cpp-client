SET(BUILD_GMOCK OFF)
SET(BUILD_TEST ON)

add_subdirectory(src)
add_subdirectory(googletest)
IF(${HZ_NEARCAHCE_PERFORMANCE} MATCHES "ON")
    add_subdirectory(performance)
endif()
