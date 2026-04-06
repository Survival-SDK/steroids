if (${ST_MODULE_DPSRVCONN_LIBXRANDR} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "libxrandr")
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

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_DPSRVCONN_LIBXRANDR})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

find_package(X11 REQUIRED)

if(NOT X11_Xrandr_FOUND)
    message(FATAL_ERROR "libXrandr not found.")
endif()

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS} 
    -D_GNU_SOURCE -fms-extensions)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "libxrandr.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${X11_LIBRARIES}
    ${X11_Xrandr_LIB}
)
