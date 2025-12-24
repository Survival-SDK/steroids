#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/texture.h"
#include "steroids/object.h"

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

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_spritectx_t;

typedef struct {
    st_object_t;
    const st_texture_t *texture;
    unsigned            width;
    unsigned            height;
    st_uv_t             uv;
} st_sprite_t;

#define ST_UV_T_DEFINED
#define ST_SPRITECTX_T_DEFINED
#define ST_SPRITE_T_DEFINED
