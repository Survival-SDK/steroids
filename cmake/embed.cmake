# Universal function to embed any file as a C string constant

function(embed TARGET SRC_FILE DST_FILE CONST_NAME)
    if(NOT IS_ABSOLUTE "${SRC_FILE}")
        set(SRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}")
    endif()
    
    if(NOT IS_ABSOLUTE "${DST_FILE}")
        set(DST_FILE "${CMAKE_CURRENT_BINARY_DIR}/${DST_FILE}")
    endif()
    
    add_custom_command(
        OUTPUT ${DST_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DSRC_FILE=${SRC_FILE}
            -DDST_FILE=${DST_FILE}
            -DCONST_NAME=${CONST_NAME}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"
        DEPENDS "${SRC_FILE}"
        COMMENT "Embedding ${SRC_FILE} as ${CONST_NAME}"
        VERBATIM
    )
    
    target_sources(${TARGET} PRIVATE ${DST_FILE})
    target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()

# Script mode: execute when called with -P
if(CMAKE_SCRIPT_MODE_FILE)
    file(READ "${SRC_FILE}" CONTENT)
    file(WRITE "${DST_FILE}"
        "#pragma once\n\n"
        "#define ${CONST_NAME} R\"\"\"\"(${CONTENT})\"\"\"\"\n"
    )
endif()
