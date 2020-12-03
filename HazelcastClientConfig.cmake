include(CMakeFindDependencyMacro)
find_dependency(Boost COMPONENTS thread chrono)
find_dependency(OpenSSL)
include(${CMAKE_CURRENT_LIST_DIR}/HazelcastClientTargets.cmake)