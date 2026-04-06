cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH ${BB_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(steroids LANGUAGES C)

include(barebones/barebones)
include(options)
include(build-module)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/common/slist"
 "${CMAKE_BINARY_DIR}/src/common/slist")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/common/dlist"
 "${CMAKE_BINARY_DIR}/src/common/dlist")

add_custom_target(copy_plugins
    ALL
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins
    COMMAND
        ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/copy-plugins.cmake
)

set(ST_INTERNAL_MODULES_COUNT 0)
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/angle/simple"
 "${CMAKE_BINARY_DIR}/src/modules/angle/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/bitmap/simple"
 "${CMAKE_BINARY_DIR}/src/modules/bitmap/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/bmcodec/libpng"
 "${CMAKE_BINARY_DIR}/src/modules/bmcodec/libpng")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/bmcodec/stb_image"
 "${CMAKE_BINARY_DIR}/src/modules/bmcodec/stb_image")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/dpsrvconn/sdl3"
 "${CMAKE_BINARY_DIR}/src/modules/dpsrvconn/sdl3")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/dpsrvconn/libxrandr"
 "${CMAKE_BINARY_DIR}/src/modules/dpsrvconn/libxrandr")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/drawq/simple"
 "${CMAKE_BINARY_DIR}/src/modules/drawq/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/dynarr/scv"
 "${CMAKE_BINARY_DIR}/src/modules/dynarr/scv")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/events/simple"
 "${CMAKE_BINARY_DIR}/src/modules/events/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/fnv1a/simple"
 "${CMAKE_BINARY_DIR}/src/modules/fnv1a/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/fs/simple"
 "${CMAKE_BINARY_DIR}/src/modules/fs/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/gfxctx/egl"
 "${CMAKE_BINARY_DIR}/src/modules/gfxctx/egl")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/gldebug/opengl"
 "${CMAKE_BINARY_DIR}/src/modules/gldebug/opengl")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/glloader/egl"
 "${CMAKE_BINARY_DIR}/src/modules/glloader/egl")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/htable/hash_table"
 "${CMAKE_BINARY_DIR}/src/modules/htable/hash_table")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/ini/inih"
 "${CMAKE_BINARY_DIR}/src/modules/ini/inih")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/keyboard/simple"
 "${CMAKE_BINARY_DIR}/src/modules/keyboard/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/logger/libsir"
 "${CMAKE_BINARY_DIR}/src/modules/logger/libsir")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajit/luajit"
 "${CMAKE_BINARY_DIR}/src/modules/luajit/luajit")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/angle"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/angle")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/dpsrvconn"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/dpsrvconn")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/gfxctx"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/gfxctx")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/keyboard"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/keyboard")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/logger"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/logger")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/mouse"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/mouse")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/opts"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/opts")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/render"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/render")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/sprite"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/sprite")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/terminal"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/terminal")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/texture"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/texture")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luajitbind/timer"
 "${CMAKE_BINARY_DIR}/src/modules/luajitbind/timer")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/matrix3x3/simple"
 "${CMAKE_BINARY_DIR}/src/modules/matrix3x3/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/mouse/simple"
 "${CMAKE_BINARY_DIR}/src/modules/mouse/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/mpool/simple"
 "${CMAKE_BINARY_DIR}/src/modules/mpool/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/opts/ketopt"
 "${CMAKE_BINARY_DIR}/src/modules/opts/ketopt")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/pathtools/cwalk"
 "${CMAKE_BINARY_DIR}/src/modules/pathtools/cwalk")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/plugin/simple"
 "${CMAKE_BINARY_DIR}/src/modules/plugin/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/render/opengl"
 "${CMAKE_BINARY_DIR}/src/modules/render/opengl")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/rbuf/lwrb"
 "${CMAKE_BINARY_DIR}/src/modules/rbuf/lwrb")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/runner/simple"
 "${CMAKE_BINARY_DIR}/src/modules/runner/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/sdl3loader/sdl3"
 "${CMAKE_BINARY_DIR}/src/modules/sdl3loader/sdl3")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/so/simple"
 "${CMAKE_BINARY_DIR}/src/modules/so/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/spcpaths/cfgpath"
 "${CMAKE_BINARY_DIR}/src/modules/spcpaths/cfgpath")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/sprite/simple"
 "${CMAKE_BINARY_DIR}/src/modules/sprite/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/terminal/simple"
 "${CMAKE_BINARY_DIR}/src/modules/terminal/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/texture/opengl"
 "${CMAKE_BINARY_DIR}/src/modules/texture/opengl")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/timer/simple"
 "${CMAKE_BINARY_DIR}/src/modules/timer/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/vec2/simple"
 "${CMAKE_BINARY_DIR}/src/modules/vec2/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/zip/zip"
 "${CMAKE_BINARY_DIR}/src/modules/zip/zip")

foreach(ST_INTERNAL_MODULE ${ST_INTERNAL_MODULES})
    set(INTERNAL_MODULES_INIT_FUNCS_PROTOTYPES
     "${INTERNAL_MODULES_INIT_FUNCS_PROTOTYPES}\
     extern st_moddata_t *st_module_${ST_INTERNAL_MODULE}_init(\
      st_modsmgr_t *);\n"
    )
    set(ST_INTERNAL_MODULES_INIT_FUNCS
     "${ST_INTERNAL_MODULES_INIT_FUNCS}\
     st_module_${ST_INTERNAL_MODULE}_init,\n        ")
endforeach(ST_INTERNAL_MODULE)
configure_file (
    "${CMAKE_SOURCE_DIR}/src/core/internal_modules.h.in"
    "${CMAKE_BINARY_DIR}/src/core/internal_modules.h"
)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/core" "${CMAKE_BINARY_DIR}/src/core")

add_custom_target(copy_core
    ALL
    COMMAND
        ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/core/steroids ${CMAKE_BINARY_DIR}
    DEPENDS
        steroids
)
