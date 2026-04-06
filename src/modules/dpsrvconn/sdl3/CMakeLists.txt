if (${ST_MODULE_DPSRVCONN_SDL3} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "sdl3")
set(ST_MODULE_TYPE ${ST_MODULE_DPSRVCONN_LIBXRANDR})
set(ST_MODULE_SUBSYSTEM "dpsrvconn")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

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

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_DPSRVCONN_SDL3})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

find_package(SDL3 CONFIG REQUIRED)
if(TARGET SDL3::Headers)
    set(ST_DPSRVCONN_SDL3_INCLUDE_TARGET SDL3::Headers)
elseif(TARGET SDL3::SDL3)
    set(ST_DPSRVCONN_SDL3_INCLUDE_TARGET SDL3::SDL3)
elseif(TARGET SDL3::SDL3-shared)
    set(ST_DPSRVCONN_SDL3_INCLUDE_TARGET SDL3::SDL3-shared)
elseif(TARGET SDL3::SDL3-static)
    set(ST_DPSRVCONN_SDL3_INCLUDE_TARGET SDL3::SDL3-static)
else()
    message(FATAL_ERROR "No SDL3 include provider target found")
endif()
get_target_property(ST_DPSRVCONN_SDL3_INCLUDE_DIRS
    ${ST_DPSRVCONN_SDL3_INCLUDE_TARGET}
    INTERFACE_INCLUDE_DIRECTORIES
)

target_compile_options(${ST_MODULE_TARGET} PRIVATE 
    ${C_COMPILE_OPTIONS} -fms-extensions
)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "sdl3.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    ${ST_DPSRVCONN_SDL3_INCLUDE_DIRS}
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    ${CMAKE_CURRENT_BINARY_DIR}
)
