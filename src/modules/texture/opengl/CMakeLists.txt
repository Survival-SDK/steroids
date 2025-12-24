if (${ST_MODULE_TEXTURE_OPENGL} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "opengl")
set(ST_MODULE_TYPE ${ST_MODULE_TEXTURE_OPENGL})
set(ST_MODULE_SUBSYSTEM "texture")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

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
find_package(PkgConfig REQUIRED)
pkg_check_modules(GL REQUIRED gl)

bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
bb_add_more_warnings(
    LANG C
    CATEGORIES
        basic
        array
        asciiz
        format
        preprocessor
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_TEXTURE_OPENGL})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS} -fms-extensions)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "opengl.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    # "${CMAKE_SOURCE_DIR}/src"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${M_LIBRARY}
    ${GL_LINK_LIBRARIES}
)
