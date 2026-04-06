cmake_minimum_required(VERSION 3.15)
project(test_hash_table C)

find_package(hash_table REQUIRED CONFIG)

add_executable(test_hash_table test_hash_table.c)
target_link_libraries(test_hash_table hash_table::hash_table)
