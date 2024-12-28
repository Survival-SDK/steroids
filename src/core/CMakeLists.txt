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

bb_add_executable(steroids)

target_compile_options(steroids PRIVATE ${C_COMPILE_OPTIONS} -fms-extensions)
target_sources(steroids PRIVATE
    main.c
    modules_manager.c
    "${CMAKE_SOURCE_DIR}/src/utils.c"
)

bb_set_c_std(steroids STD 11 EXTENSIONS)

bb_add_iwyu_mapping_file(steroids ${CMAKE_CURRENT_SOURCE_DIR}/iwyu.imp)

target_include_directories(steroids PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(steroids PRIVATE
    dlist
    ${ST_INTERNAL_MODULES_LIBS}
)

file(WRITE ${CMAKE_BINARY_DIR}/steroids.ini
"[steroids.runner]
plugin_path=cmake_build/plugins
run_module=lua:luajit
script=main.lua"
)