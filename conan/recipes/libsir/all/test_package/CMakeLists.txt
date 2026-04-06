cmake_minimum_required(VERSION 3.15)
project(test_libsir C)

find_package(libsir REQUIRED CONFIG)

add_executable(test_libsir test_libsir.c)
target_link_libraries(test_libsir libsir::libsir)
