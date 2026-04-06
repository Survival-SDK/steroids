if (${ST_MODULE_BMCODEC_STB_IMAGE} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "stb_image")
set(ST_MODULE_TYPE ${ST_MODULE_BMCODEC_STB_IMAGE})
set(ST_MODULE_SUBSYSTEM "bmcodec")
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
        array
        asciiz
        format
        preprocessor
        alloc
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_BMCODEC_STB_IMAGE})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

find_package(stb CONFIG REQUIRED)
get_target_property(STB_INCLUDE_DIRS stb::stb INTERFACE_INCLUDE_DIRECTORIES)

target_compile_options(${ST_MODULE_TARGET} PRIVATE
    ${C_COMPILE_OPTIONS}
    -fms-extensions
)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "stb_image.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    ${STB_INCLUDE_DIRS}
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)
