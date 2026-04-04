if (${ST_MODULE_GLLOADER_EGL} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "egl")
set(ST_MODULE_TYPE ${ST_MODULE_GLLOADER_EGL})
set(ST_MODULE_SUBSYSTEM "glloader")
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

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_GLLOADER_EGL})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

find_package(PkgConfig REQUIRED)
pkg_check_modules(EGL REQUIRED egl)

target_compile_options(${ST_MODULE_TARGET} PRIVATE
    ${C_COMPILE_OPTIONS}
    -fms-extensions
)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "egl.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${EGL_LINK_LIBRARIES}
)
