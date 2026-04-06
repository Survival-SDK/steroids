include(embed)

function(st_build_luajitbind_module MODULE_NAME MODULE_TYPE_VAR)
    if (${${MODULE_TYPE_VAR}} STREQUAL "no")
        return()
    endif()

    set(ST_MODULE_TYPE ${${MODULE_TYPE_VAR}})
    set(ST_MODULE_SUBSYSTEM "luajitbind")
    set(ST_MODULE_NAME "${MODULE_NAME}")
    set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

    # Generate capitalized module name for display
    string(SUBSTRING "${MODULE_NAME}" 0 1 FIRST_LETTER)
    string(TOUPPER "${FIRST_LETTER}" FIRST_LETTER_UPPER)
    string(SUBSTRING "${MODULE_NAME}" 1 -1 REST_OF_NAME)
    set(MODULE_NAME_CAPITALIZED "${FIRST_LETTER_UPPER}${REST_OF_NAME}")
    set(MODULE_NAME_CAPITALIZED_STR "\"${MODULE_NAME_CAPITALIZED}\"")
    set(MODULE_NAME_STR "\"${MODULE_NAME}\"")

    # Configure module config.h from parent directory template
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    )

    # Generate <module>.c from parent directory template
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../luajitbind.c.in"
        "${CMAKE_CURRENT_BINARY_DIR}/luajitbind.c"
    )

    bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
    bb_add_more_warnings(
        LANG C
        CATEGORIES basic alloc array asciiz enum preprocessor
        OPTIONS C_COMPILE_OPTIONS
    )

    st_add_module(${ST_MODULE_TARGET} ${${MODULE_TYPE_VAR}})
    st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

    find_package(luajit REQUIRED)
    get_target_property(LUAJIT_INCLUDE_DIRS luajit::luajit 
        INTERFACE_INCLUDE_DIRECTORIES)

    target_compile_options(${ST_MODULE_TARGET} PRIVATE 
        ${C_COMPILE_OPTIONS} -fms-extensions)
    target_sources(${ST_MODULE_TARGET} PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/luajitbind.c"
    )

    embed(${ST_MODULE_TARGET} "embedded.luajit" "embedded_luajit.h" 
        "EMBEDDED_LUAJIT")

    bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

    target_include_directories(${ST_MODULE_TARGET} PRIVATE
        ${LUAJIT_INCLUDE_DIRS}
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()
