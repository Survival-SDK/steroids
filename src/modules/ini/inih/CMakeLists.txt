if (${ST_MODULE_INI_INIH} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "inih")
set(ST_MODULE_TYPE ${ST_MODULE_INI_INIH})
set(ST_MODULE_SUBSYSTEM "ini")
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

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_INI_INIH})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

bb_find_library(
  VAR_PREFIX INIH
  NAME inih
  FILENAMES inih
  COMPILER_OPTIONS -linih
  REQUIRED
)

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS} -fms-extensions)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "inih.c"
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
    ${INIH_LIBRARY}
)
