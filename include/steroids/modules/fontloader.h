#pragma once

#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/modules/font.h"

#ifndef ST_FONTLOADERCTX_T_DEFINED
    typedef st_modctx_t st_fontloaderctx_t;
#endif

typedef st_font_t *(*st_fontloader_load_t)(st_fontloaderctx_t *fontloader_ctx,
 const char *filename);
typedef st_font_t *(*st_fontloader_memload_t)(
 st_fontloaderctx_t *fontloader_ctx, const void *data, size_t size);

typedef struct {
    st_modctx_funcs_t;
    st_fontloader_load_t    load;
    st_fontloader_memload_t memload;
} st_fontloaderctx_funcs_t;

#define ST_FONTLOADERCTX_CALL(ctx, func, ...) \
    ((st_fontloaderctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
