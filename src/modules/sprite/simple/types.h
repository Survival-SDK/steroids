#pragma once

#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/texture.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_sprite_simple_logger_t;

typedef struct {
    st_texture_get_width_t  get_width;
    st_texture_get_height_t get_height;
} st_sprite_simple_texture_t;

typedef struct {
    st_sprite_simple_logger_t  logger;
    st_sprite_simple_texture_t texture;
} st_sprite_simple_t;

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
    st_sprite_simple_t *module;
    const st_texture_t *texture;
    unsigned            width;
    unsigned            height;
    st_uv_t             uv;
} st_sprite_t;

#define ST_UV_T_DEFINED
#define ST_SPRITE_T_DEFINED
