cmake_minimum_required(VERSION 3.15)
project(test_ketopt C)

find_package(ketopt REQUIRED CONFIG)

add_executable(test_ketopt test_ketopt.c)
target_link_libraries(test_ketopt ketopt::ketopt)
