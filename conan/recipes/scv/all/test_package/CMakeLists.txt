cmake_minimum_required(VERSION 3.15)
project(test_scv C)

find_package(scv REQUIRED CONFIG)

add_executable(test_scv test_scv.c)
target_link_libraries(test_scv scv::scv)
