#pragma once

#include <stdbool.h>

#include "steroids/modctx.h"
#include "steroids/modules/atlas.h"
#include "steroids/modules/sprite.h"
#include "steroids/modules/texture.h"
#include "steroids/object.h"

#ifndef ST_ATILESCTX_T_DEFINED
    typedef st_modctx_t st_atilesctx_t;
#endif

#ifndef ST_ATILESET_T_DEFINED
    typedef struct st_atileset st_atileset_t;
#endif

// #ifndef ST_ATILE_T_DEFINED
    // typedef st_atileset_t st_atile_t;
// #endif

typedef enum {
    ST_ATS_NORTH_WALL = 0,
    ST_ATS_SOUTH_WALL = 1,
    ST_ATS_EAST_WALL = 2,
    ST_ATS_WEST_WALL = 3,
    ST_ATS_NORTH_EAST_EXTERNAL_CORNER = 4,
    ST_ATS_SOUTH_EAST_EXTERNAL_CORNER = 5,
    ST_ATS_SOUTH_WEST_EXTERNAL_CORNER = 6,
    ST_ATS_NORTH_WEST_EXTERNAL_CORNER = 7,
    ST_ATS_NORTH_EAST_INTERNAL_CORNER = 8,
    ST_ATS_SOUTH_EAST_INTERNAL_CORNER = 9,
    ST_ATS_SOUTH_WEST_INTERNAL_CORNER = 10,
    ST_ATS_NORTH_WEST_INTERNAL_CORNER = 11,
    ST_ATS_ENTIRE = 12,
    ST_ATS_EMPTY = 13,

    ST_ATS_NONEMPTY_LEN = 13,
    ST_ATS_LEN = 14,
} st_atsubtile_t;

typedef enum {
    ST_ATSPP_NW = 0,
    ST_ATSPP_NE,
    ST_ATSPP_SE,
    ST_ATSPP_SW,

    ST_ATSPP_LEN = 4,
} st_atsubtilepos_t;

typedef enum {
    ST_ATNM_SAME,
    ST_ATNM_ANY,
} st_atnghbrmode_t;

typedef const st_atileset_t *(*st_atiles_tileset_load_t)(
 st_atilesctx_t *atilesctx, const char *filename);
typedef const st_atileset_t *(*st_atiles_tileset_from_texture_t)(
 st_atilesctx_t *atilesctx, st_texture_t *texture);
typedef const st_atileset_t *(*st_atiles_tileset_from_atlas_t)(
 st_atilesctx_t *atilesctx, const st_atlas_t *atlas);
typedef int (*st_atiles_add_layer_t)(st_atilesctx_t *atilesctx, 
 unsigned rows, unsigned cols, unsigned tile_width, unsigned tile_height, 
 st_atnghbrmode_t neighbor_mode);
typedef void (*st_atiles_update_tile_t)(st_atilesctx_t *atilesctx, 
 unsigned layer_index, unsigned row, unsigned col, st_atile_t *tile, 
 bool update_subtiles);
typedef const st_atile_t *(*st_atiles_get_tile_t)(
 const st_atilesctx_t *atilesctx, unsigned layer_index, unsigned row, 
 unsigned col);
typedef void (*st_atiles_get_subtile_t)(const st_atilesctx_t *atilesctx, 
 st_atsubtile_t dst[ST_ATSPP_LEN], unsigned layer_index, unsigned row, 
 unsigned col);
typedef void (*st_atiles_get_subtile_sprite_t)(
 const st_atilesctx_t *atilesctx, const st_sprite_t *dst[ST_ATSPP_LEN], 
 unsigned layer_index, unsigned row, unsigned col);
typedef void (*st_atiles_update_t)(st_atilesctx_t *atilesctx);

typedef struct {
    st_modctx_funcs_t;
    st_atiles_tileset_load_t         tileset_load;
    st_atiles_tileset_from_texture_t tileset_from_texture;
    st_atiles_tileset_from_atlas_t   tileset_from_atlas;
    st_atiles_add_layer_t            add_layer;
    st_atiles_update_tile_t          update_tile;
    st_atiles_get_tile_t             get_tile;
    st_atiles_get_subtile_t          get_subtile;
    st_atiles_get_subtile_sprite_t   get_subtile_sprite;
    st_atiles_update_t               update;
} st_atilesctx_funcs_t;

#define ST_ATILESCTX_CALL(ctx, func, ...) \
    ((st_atilesctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
