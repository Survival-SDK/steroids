if (${ST_MODULE_HTABLE_HASH_TABLE} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "hash_table")
set(ST_MODULE_TYPE ${ST_MODULE_HTABLE_HASH_TABLE})
set(ST_MODULE_SUBSYSTEM "htable")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

set(ST_MODULE_PREREQS [=[
    {
        .subsystem = "logger",
        .name = NULL,
    },
]=])
set(ST_MODULE_PREREQS_COUNT 1)

configure_file(
    "config.h.in"
    "config.h"
)

bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
bb_add_more_warnings(
    LANG C
    CATEGORIES
        basic
        alloc
        array
        asciiz
        format
        preprocessor
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_HTABLE_HASH_TABLE})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

bb_find_library(
  VAR_PREFIX HASH_TABLE
  NAME hash_table
  FILENAMES hash_table
  COMPILER_OPTIONS -lhash_table
  REQUIRED
)

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS} -fms-extensions)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "hash_table.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

bb_add_iwyu_mapping_file(
    ${ST_MODULE_TARGET}
    ${CMAKE_CURRENT_SOURCE_DIR}/iwyu.imp
)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${HASH_TABLE_LIBRARY}
)
