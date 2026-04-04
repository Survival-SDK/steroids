#pragma once

#include <stdbool.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_BITMAPCTX_T_DEFINED
    typedef st_modctx_t st_bitmapctx_t;
#endif
#ifndef ST_BITMAP_T_DEFINED
    typedef st_object_t st_bitmap_t;
#endif

typedef enum {
    PF_UNKNOWN,
    PF_RGBA,

    PF_MAX,
} st_pxfmt_t;

typedef st_bitmap_t *(*st_bitmap_load_t)(st_bitmapctx_t *bitmap_ctx,
 const char *filename);
typedef st_bitmap_t *(*st_bitmap_memload_t)(st_bitmapctx_t *bitmap_ctx,
 const void *data, size_t size);
typedef st_bitmap_t *(*st_bitmap_import_t)(st_bitmapctx_t *bitmap_ctx,
 const void *data, unsigned width, unsigned height, st_pxfmt_t pixel_format);
typedef bool (*st_bitmap_save_t)(const st_bitmap_t *bitmap,
 const char *filename, const char *format);
typedef bool (*st_bitmap_memsave_t)(void *dst, size_t *size,
 const st_bitmap_t *bitmap, const char *format);
typedef const void *(*st_bitmap_get_data_t)(const st_bitmap_t *bitmap);
typedef unsigned (*st_bitmap_get_width_t)(const st_bitmap_t *bitmap);
typedef unsigned (*st_bitmap_get_height_t)(const st_bitmap_t *bitmap);
typedef st_pxfmt_t (*st_bitmap_get_pixel_format_t)(const st_bitmap_t *bitmap);

typedef struct {
    st_modctx_funcs_t;
    st_bitmap_load_t    load;
    st_bitmap_memload_t memload;
    st_bitmap_import_t  import;
} st_bitmapctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_bitmap_save_t             save;
    st_bitmap_memsave_t          memsave;
    st_bitmap_get_data_t         get_data;
    st_bitmap_get_width_t        get_width;
    st_bitmap_get_height_t       get_height;
    st_bitmap_get_pixel_format_t get_pixel_format;
} st_bitmap_funcs_t;

#define ST_BITMAPCTX_CALL(ctx, func, ...) \
    ((st_bitmapctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_BITMAP_CALL(object, func, ...) \
    ((st_bitmap_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
