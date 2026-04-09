#pragma once

#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/modules/atlas.h"

#ifndef ST_ATLLOADERCTX_T_DEFINED
    typedef st_modctx_t st_atlloaderctx_t;
#endif

typedef st_atlas_t *(*st_atlloader_load_t)(st_atlloaderctx_t *atlloader_ctx,
 const char *filename);
typedef st_atlas_t *(*st_atlloader_memload_t)(st_atlloaderctx_t *atlloader_ctx,
 const void *data, size_t size);

typedef struct {
    st_modctx_funcs_t;
    st_atlloader_load_t    load;
    st_atlloader_memload_t memload;
} st_atlloaderctx_funcs_t;

#define ST_ATLLOADERCTX_CALL(ctx, func, ...) \
    ((st_atlloaderctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
