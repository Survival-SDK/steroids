#pragma once

#include "steroids/module.h"
#include "steroids/modules/bitmap.h"

typedef st_modctx_t *(*st_bmcodec_init_t)(st_modctx_t *bitmap_ctx,
 st_modctx_t *logger_ctx);
typedef void (*st_bmcodec_quit_t)(st_modctx_t *bmcodec_ctx);

typedef int (*st_bmcodec_get_priority_t)(st_modctx_t *bmcodec_ctx);
typedef st_bitmap_t *(*st_bmcodec_load_t)(st_modctx_t *bmcodec_ctx,
 const char *filename);
typedef st_bitmap_t *(*st_bmcodec_memload_t)(st_modctx_t *bmcodec_ctx,
 const void *data, size_t size);
typedef bool (*st_bmcodec_save_t)(st_modctx_t *bmcodec_ctx,
 const st_bitmap_t *bitmap, const char *filename, const char *format);
typedef bool (*st_bmcodec_memsave_t)(st_modctx_t *bmcodec_ctx, void *dst,
 size_t *size, const st_bitmap_t *bitmap, const char *format);

typedef struct {
    st_bmcodec_init_t         bmcodec_init;
    st_bmcodec_quit_t         bmcodec_quit;
    st_bmcodec_get_priority_t bmcodec_get_priority;
    st_bmcodec_load_t         bmcodec_load;
    st_bmcodec_memload_t      bmcodec_memload;
    st_bmcodec_save_t         bmcodec_save;
    st_bmcodec_memsave_t      bmcodec_memsave;
} st_bmcodec_funcs_t;
