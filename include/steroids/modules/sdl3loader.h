#pragma once

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_SDL3LOADERCTX_T_DEFINED
    typedef st_modctx_t st_sdl3loaderctx_t;
#endif

typedef void *(*st_sdl3loader_get_proc_address_t)(
 const st_sdl3loaderctx_t *sdl3loader_ctx, const char *funcname);

typedef struct {
    st_modctx_funcs_t;
    st_sdl3loader_get_proc_address_t get_proc_address;
} st_sdl3loaderctx_funcs_t;

#define ST_SDL3LOADERCTX_CALL(ctx, func, ...) \
    ((st_sdl3loaderctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
