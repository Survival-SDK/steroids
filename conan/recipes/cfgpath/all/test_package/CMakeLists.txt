cmake_minimum_required(VERSION 3.15)
project(test_cfgpath C)

find_package(cfgpath REQUIRED CONFIG)

add_executable(test_cfgpath test_cfgpath.c)
target_link_libraries(test_cfgpath cfgpath::cfgpath)
