#pragma once

#include <stdbool.h>

#include "steroids/module.h"

typedef st_modctx_t *(*st_luabind_init_t)(st_modctx_t *logger_ctx,
 st_modctx_t *lua_ctx);
typedef void (*st_luabind_quit_t)(st_modctx_t *luabind_ctx);

typedef struct {
    st_luabind_init_t luabind_init;
    st_luabind_quit_t luabind_quit;
} st_luabind_funcs_t;
