#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_TEXTURECTX_T_DEFINED
    typedef st_modctx_t st_texturectx_t;
#endif
#ifndef ST_TEXTURE_T_DEFINED
    typedef st_object_t st_texture_t;
#endif

typedef st_texture_t *(*st_texture_load_t)(st_texturectx_t *texture_ctx,
 const char *filename);
typedef st_texture_t *(*st_texture_memload_t)(st_texturectx_t *texture_ctx,
 const void *data, size_t size);
typedef bool (*st_texture_bind_t)(const st_texture_t *texture, unsigned unit);
typedef unsigned (*st_texture_get_width_t)(const st_texture_t *texture);
typedef unsigned (*st_texture_get_height_t)(const st_texture_t *texture);

typedef struct {
    st_modctx_funcs_t;
    st_texture_load_t    load;
    st_texture_memload_t memload;
} st_texturectx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_texture_bind_t       bind;
    st_texture_get_width_t  get_width;
    st_texture_get_height_t get_height;
} st_texture_funcs_t;

#define ST_TEXTURECTX_CALL(ctx, func, ...) \
    ((st_texturectx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_TEXTURE_CALL(object, func, ...) \
    ((st_texture_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
