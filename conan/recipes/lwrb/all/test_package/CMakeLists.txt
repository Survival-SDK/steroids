cmake_minimum_required(VERSION 3.15)
project(test_lwrb C)

find_package(lwrb REQUIRED CONFIG)

add_executable(test_lwrb test_lwrb.c)
target_link_libraries(test_lwrb lwrb::lwrb)
