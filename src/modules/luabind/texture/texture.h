#ifndef ST_MODULES_LUABIND_TEXTURE_TEXTURE_H
#define ST_MODULES_LUABIND_TEXTURE_TEXTURE_H

#include "config.h" // IWYU pragma: keep
#include "steroids/luabind.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/lua.h"
#include "steroids/types/modules/texture.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_luabind_texture_logger_t;

typedef struct {
    st_modctx_t *ctx;
} st_luabind_texture_lua_t;

typedef struct {
    st_luabind_texture_logger_t logger;
    st_luabind_texture_lua_t    lua;
} st_luabind_texture_t;

st_luabind_funcs_t st_luabind_texture_funcs = {
    .luabind_init = st_luabind_init,
    .luabind_quit = st_luabind_quit,
};

st_modfuncentry_t st_module_luabind_texture_funcs[] = {
    {"init", st_luabind_init},
    {"quit", st_luabind_quit},
    {NULL, NULL},
};

#endif /* ST_MODULES_LUABIND_TEXTURE_TEXTURE_H */