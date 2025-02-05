cmake_minimum_required(VERSION 3.9)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${BB_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(barebones/barebones)
include(options)
include(build-module)

project(steroids LANGUAGES C)

set(CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/cmake"
    "${CMAKE_SOURCE_DIR}/cmake/modules"
)

add_subdirectory("${CMAKE_SOURCE_DIR}/src/common/slist"
 "${CMAKE_BINARY_DIR}/src/common/slist")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/common/dlist"
 "${CMAKE_BINARY_DIR}/src/common/dlist")

add_custom_target(copy_plugins
    ALL
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins
    # "${CMAKE_COMMAND} -E copy" not working with *-wildcard if no suitable
    # files found
    COMMAND
        # ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/*.stp ${CMAKE_BINARY_DIR}/plugins
        ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/copy-plugins.cmake
)

set(ST_INTERNAL_MODULES_COUNT 0)
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/angle/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/angle/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/bitmap/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/bitmap/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/bmcodec/libpng"
#  "${CMAKE_BINARY_DIR}/src/modules/bmcodec/libpng")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/bmcodec/stb_image"
#  "${CMAKE_BINARY_DIR}/src/modules/bmcodec/stb_image")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/drawq/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/drawq/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/dynarr/scv"
#  "${CMAKE_BINARY_DIR}/src/modules/dynarr/scv")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/events/simple"
 "${CMAKE_BINARY_DIR}/src/modules/events/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/fnv1a/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/fnv1a/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/fs/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/fs/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/gfxctx/egl"
#  "${CMAKE_BINARY_DIR}/src/modules/gfxctx/egl")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/gldebug/opengl"
#  "${CMAKE_BINARY_DIR}/src/modules/gldebug/opengl")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/glloader/egl"
#  "${CMAKE_BINARY_DIR}/src/modules/glloader/egl")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/htable/hash_table"
#  "${CMAKE_BINARY_DIR}/src/modules/htable/hash_table")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/ini/inih"
#  "${CMAKE_BINARY_DIR}/src/modules/ini/inih")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/keyboard/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/keyboard/simple")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/logger/libsir"
 "${CMAKE_BINARY_DIR}/src/modules/logger/libsir")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/lua/luajit"
#  "${CMAKE_BINARY_DIR}/src/modules/lua/luajit")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/angle"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/angle")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/bitmap"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/bitmap")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/drawq"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/drawq")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/dynarr"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/dynarr")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/events"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/events")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/fs"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/fs")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/gfxctx"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/gfxctx")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/keyboard"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/keyboard")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/logger"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/logger")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/matrix3x3"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/matrix3x3")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/monitor"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/monitor")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/mouse"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/mouse")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/opts"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/opts")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/pathtools"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/pathtools")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/render"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/render")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/sprite"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/sprite")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/terminal"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/terminal")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/texture"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/texture")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/timer"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/timer")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/vec2"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/vec2")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/luabind/window"
#  "${CMAKE_BINARY_DIR}/src/modules/luabind/window")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/matrix3x3/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/matrix3x3/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/monitor/xlib"
#  "${CMAKE_BINARY_DIR}/src/modules/monitor/xlib")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/mouse/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/mouse/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/opts/ketopt"
#  "${CMAKE_BINARY_DIR}/src/modules/opts/ketopt")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/pathtools/cwalk"
#  "${CMAKE_BINARY_DIR}/src/modules/pathtools/cwalk")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/plugin/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/plugin/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/render/opengl"
#  "${CMAKE_BINARY_DIR}/src/modules/render/opengl")
add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/rbuf/lwrb"
 "${CMAKE_BINARY_DIR}/src/modules/rbuf/lwrb")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/runner/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/runner/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/so/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/so/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/spcpaths/cfgpath"
#  "${CMAKE_BINARY_DIR}/src/modules/spcpaths/cfgpath")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/sprite/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/sprite/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/terminal/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/terminal/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/texture/opengl"
#  "${CMAKE_BINARY_DIR}/src/modules/texture/opengl")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/timer/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/timer/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/vec2/simple"
#  "${CMAKE_BINARY_DIR}/src/modules/vec2/simple")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/window/xlib"
#  "${CMAKE_BINARY_DIR}/src/modules/window/xlib")
# add_subdirectory("${CMAKE_SOURCE_DIR}/src/modules/zip/zip"
#  "${CMAKE_BINARY_DIR}/src/modules/zip/zip")

foreach(ST_INTERNAL_MODULE ${ST_INTERNAL_MODULES})
    set(INTERNAL_MODULES_INIT_FUNCS_PROTOTYPES
     "${INTERNAL_MODULES_INIT_FUNCS_PROTOTYPES}\
     extern st_moddata_t *st_module_${ST_INTERNAL_MODULE}_init(\
      st_modsmgr_t *, st_modsmgr_funcs_t *);\n"
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
