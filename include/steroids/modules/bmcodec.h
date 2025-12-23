#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/modules/bitmap.h"

#ifndef ST_BMCODECCTX_T_DEFINED
    typedef st_modctx_t st_bmcodecctx_t;
#endif

typedef int (*st_bmcodec_get_priority_t)(const st_bmcodecctx_t *bmcodec_ctx);
typedef st_bitmap_t *(*st_bmcodec_load_t)(st_bmcodecctx_t *bmcodec_ctx,
 const char *filename);
typedef st_bitmap_t *(*st_bmcodec_memload_t)(st_bmcodecctx_t *bmcodec_ctx,
 const void *data, size_t size);
typedef bool (*st_bmcodec_save_t)(st_bmcodecctx_t *bmcodec_ctx,
 const st_bitmap_t *bitmap, const char *filename, const char *format);
typedef bool (*st_bmcodec_memsave_t)(st_bmcodecctx_t *bmcodec_ctx, void *dst,
 size_t *size, const st_bitmap_t *bitmap, const char *format);

typedef struct {
    st_modctx_funcs_t;
    st_bmcodec_get_priority_t get_priority;
    st_bmcodec_load_t         load;
    st_bmcodec_memload_t      memload;
    st_bmcodec_save_t         save;
    st_bmcodec_memsave_t      memsave;
} st_bmcodecctx_funcs_t;

#define ST_BMCODECCTX_CALL(ctx, func, ...) \
    ((st_bmcodecctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
