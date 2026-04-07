cmake_minimum_required(VERSION 3.15)
project(test_ezxml C)

find_package(ezxml REQUIRED CONFIG)

add_executable(test_ezxml test_ezxml.c)
target_link_libraries(test_ezxml ezxml::ezxml)
