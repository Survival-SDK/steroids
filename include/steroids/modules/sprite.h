#pragma once

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_SPRITECTX_T_DEFINED
    typedef st_modctx_t st_spritectx_t;
#endif
#ifndef ST_SPRITE_T_DEFINED
    typedef st_object_t st_sprite_t;
#endif

#ifndef ST_UV_T_DEFINED
    typedef struct {
        float u;
        float v;
    } st_uvcoord_t;

    typedef struct {
        st_uvcoord_t upper_left;
        st_uvcoord_t upper_right;
        st_uvcoord_t lower_left;
        st_uvcoord_t lower_right;
    } st_uv_t;
#endif

typedef st_sprite_t *(*st_sprite_from_texture_t)(st_spritectx_t *sprite_ctx,
 const st_texture_t *texture);
typedef const st_texture_t *(*st_sprite_get_texture_t)(
 const st_sprite_t *sprite);
typedef unsigned (*st_sprite_get_width_t)(const st_sprite_t *sprite);
typedef unsigned (*st_sprite_get_height_t)(const st_sprite_t *sprite);
typedef void (*st_sprite_export_uv_t)(const st_sprite_t *sprite, st_uv_t *dstuv);

typedef struct {
    st_modctx_funcs_t;
    st_sprite_from_texture_t from_texture;
} st_spritectx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_sprite_get_texture_t get_texture;
    st_sprite_get_width_t   get_width;
    st_sprite_get_height_t  get_height;
    st_sprite_export_uv_t   export_uv;
} st_sprite_funcs_t;

#define ST_SPRITECTX_CALL(ctx, func, ...) \
    ((st_spritectx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_SPRITE_CALL(object, func, ...) \
    ((st_sprite_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
