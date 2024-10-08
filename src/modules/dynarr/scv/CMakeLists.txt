if (${ST_MODULE_DYNARR_SCV} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "scv")
set(ST_MODULE_TYPE ${ST_MODULE_DYNARR_SCV})
set(ST_MODULE_SUBSYSTEM "dynarr")
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

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_DYNARR_SCV})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

bb_find_library(
  VAR_PREFIX SCV
  NAME scv
  FILENAMES scv
  COMPILER_OPTIONS -lscv
  REQUIRED
)

target_compile_options(${ST_MODULE_TARGET} PRIVATE
    ${C_COMPILE_OPTIONS}
    -D_GNU_SOURCE # for qsort_r
)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "scv.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${SCV_LIBRARY}
)
