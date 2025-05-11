#pragma once

#include "steroids/module.h"
#include "steroids/modules/bitmap.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/object.h"

#ifndef ST_TEXTURE_T_DEFINED
    typedef struct st_texture_s st_texture_t;
#endif

typedef st_modctx_t *(*st_texture_init_t)(st_modctx_t *bitmap_ctx,
 st_modctx_t *logger_ctx, st_gfxctx_t *gfxctx);
typedef void (*st_texture_quit_t)(st_modctx_t *texture_ctx);

typedef st_texture_t *(*st_texture_load_t)(st_modctx_t *texture_ctx,
 const char *filename);
typedef st_texture_t *(*st_texture_memload_t)(st_modctx_t *texture_ctx,
 const void *data, size_t size);
typedef void (*st_texture_destroy_t)(st_texture_t *texture);
typedef bool (*st_texture_bind_t)(const st_texture_t *texture, unsigned unit);
typedef unsigned (*st_texture_get_width_t)(const st_texture_t *texture);
typedef unsigned (*st_texture_get_height_t)(const st_texture_t *texture);

typedef struct {
    st_texture_init_t    texture_init;
    st_texture_quit_t    texture_quit;
    st_texture_load_t    texture_load;
    st_texture_memload_t texture_memload;
} st_texturectx_funcs_t;

typedef struct {
    st_texture_destroy_t    destroy;
    st_texture_bind_t       bind;
    st_texture_get_width_t  get_width;
    st_texture_get_height_t get_height;
} st_texture_funcs_t;

#define ST_TEXTURE_CALL(object, func, ...) \
    ((st_texture_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
