#pragma once

#include <stdbool.h>

#include "steroids/modules/dynarr.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/sprite.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t       *ctx;
    st_dynarr_create_t create;
} st_drawq_simple_dynarr_t;

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_drawq_simple_logger_t;

typedef struct {
    st_modctx_t *ctx;
} st_drawq_simple_sprite_t;

typedef struct {
    st_drawq_simple_dynarr_t dynarr;
    st_drawq_simple_logger_t logger;
    st_drawq_simple_sprite_t sprite;
} st_drawq_simple_t;

ST_CLASS(
    st_dynarr_t *entries;
) st_drawq_t;

#define ST_DRAWQ_T_DEFINED
