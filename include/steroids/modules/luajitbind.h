#pragma once

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_LUAJITBINDCTX_T_DEFINED
    typedef st_modctx_t st_luajitbindctx_t;
#endif

typedef bool (*st_luajitbind_bind_t)(st_luajitbindctx_t *luajitbind_ctx,
 const char *state_name);

typedef struct {
    st_modctx_funcs_t;
    st_luajitbind_bind_t bind;
} st_luajitbindctx_funcs_t;

#define ST_LUAJITBINDCTX_CALL(ctx, func, ...) \
    ((st_luajitbindctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
