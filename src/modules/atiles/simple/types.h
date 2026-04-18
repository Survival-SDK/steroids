#pragma once

#include "steroids/modctx.h"

typedef struct st_atilesctx st_atilesctx_t;
typedef struct st_atileset st_atileset_t;
typedef st_atileset_t st_atile_t;
#define ST_ATILESCTX_T_DEFINED
#define ST_ATILESET_T_DEFINED
#define ST_ATILE_T_DEFINED

#include "steroids/modules/atiles.h"
#include "steroids/modules/dynarr.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/sprite.h"
#include "steroids/modules/texture.h"
#include "steroids/object.h"

struct st_atileset {
    st_texture_t *texture;
    bool          texture_owned;
    st_sprite_t  *sprites[ST_ATS_LEN];
};

// typedef st_atileset_t st_atile_t;

typedef struct {
    st_atile_t    *tile;
    st_atsubtile_t subtiles[ST_ATSPP_LEN];
} st_atilecell_t;

typedef struct {
    unsigned         rows;
    unsigned         cols;
    unsigned         tile_width;
    unsigned         tile_height;
    st_atnghbrmode_t neighbor_mode;
    st_atilecell_t   cells[];
} st_atilelayer_t;

struct st_atilesctx {
    st_modctx_t;
    st_dynarrctx_t  *dynarr_ctx;
    st_loggerctx_t  *logger_ctx;
    st_texturectx_t *texture_ctx;
    st_spritectx_t  *sprite_ctx;
    st_dynarr_t     *layers;
    st_dynarr_t     *tilesets;
};

// #define ST_ATILESCTX_T_DEFINED
// #define ST_ATILESET_T_DEFINED
// #define ST_ATILE_T_DEFINED
