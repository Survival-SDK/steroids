#ifndef ST_MODULES_LUA_LUAJIT_TYPES_H
#define ST_MODULES_LUA_LUAJIT_TYPES_H

#include <lua.h>

#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/luabind.h"

#include "slist.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_lua_luajit_logger_t;

typedef struct {
    st_modctx_t *ctx;
} st_lua_luajit_opts_t;

typedef struct {
    st_modctx_t      *ctx;
    st_luabind_quit_t quit;
} st_lua_luajit_binding_t;

typedef struct {
    st_lua_luajit_logger_t logger;
    st_lua_luajit_opts_t   opts;
    lua_State             *state;
    st_slist_t            *bindings;
} st_lua_luajit_t;

#endif /* ST_MODULES_LUA_LUAJIT_TYPES_H */
