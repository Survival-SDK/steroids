cmake_minimum_required(VERSION 3.15)
project(test_cmake_barebones NONE)

# Try to include one of the barebones modules
# This will fail if CMAKE_MODULE_PATH is not set correctly
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lib/cmake/barebones")

# Just verify that we can find the modules directory
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../lib/cmake/barebones")
    message(FATAL_ERROR "barebones CMake modules not found")
endif()

message(STATUS "cmake_barebones modules found successfully")
