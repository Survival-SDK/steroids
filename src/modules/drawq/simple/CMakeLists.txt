if (${ST_MODULE_DRAWQ_SIMPLE} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "simple")
set(ST_MODULE_TYPE ${ST_MODULE_DRAWQ_SIMPLE})
set(ST_MODULE_SUBSYSTEM "drawq")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

set(ST_MODULE_PREREQS [=[
    {
        .subsystem = "dynarr",
        .name = NULL,
    },
    {
        .subsystem = "logger",
        .name = NULL,
    },
    {
        .subsystem = "sprite",
        .name = NULL,
    },
]=])
set(ST_MODULE_PREREQS_COUNT 3)

configure_file(
    "config.h.in"
    "config.h"
)

bb_find_library(
  VAR_PREFIX M
  NAME libm
  FILENAMES m
  COMPILER_OPTIONS -lm
  REQUIRED
)

bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
bb_add_more_warnings(
    LANG C
    CATEGORIES
        basic
        asciiz
        array
        format
        preprocessor
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_DRAWQ_SIMPLE})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS})
target_sources(${ST_MODULE_TARGET} PRIVATE
    "simple.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${M_LIBRARY}
)
