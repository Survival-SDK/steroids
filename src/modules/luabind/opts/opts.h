#pragma once

#include "config.h" // IWYU pragma: keep
#include "steroids/luabind.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/lua.h"
#include "steroids/types/modules/opts.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_luabind_opts_logger_t;

typedef struct {
    st_modctx_t *ctx;
} st_luabind_opts_lua_t;

typedef struct {
    st_luabind_opts_logger_t logger;
    st_luabind_opts_lua_t    lua;
} st_luabind_opts_t;

static st_luabind_funcs_t st_luabind_opts_funcs = {
    .luabind_init = st_luabind_init,
    .luabind_quit = st_luabind_quit,
};

static st_modfuncentry_t st_module_luabind_opts_funcs[] = {
    {"init", st_luabind_init},
    {"quit", st_luabind_quit},
    {NULL, NULL},
};
