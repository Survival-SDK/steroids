#include "simple.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modules/atlas.h"
#include "steroids/modules/dynarr.h"
#include "steroids/modules/sprite.h"
#include "steroids/modules/texture.h"
#include "steroids/modsmgr.h"

static st_atilesctx_t *st_atiles_init(const st_param_t params[]);
static void st_atiles_quit(st_atilesctx_t *atilesctx);

static const st_atileset_t *st_atiles_tileset_load(st_atilesctx_t *atilesctx, 
 const char *filename);
static const st_atileset_t *st_atiles_tileset_from_texture(
 st_atilesctx_t *atilesctx, st_texture_t *texture);
static const st_atileset_t *st_atiles_tileset_from_atlas(
 st_atilesctx_t *atilesctx, const st_atlas_t *atlas);
static int st_atiles_add_layer(st_atilesctx_t *atilesctx, unsigned rows, 
 unsigned cols, unsigned tile_width, unsigned tile_height, 
 st_atnghbrmode_t neighbor_mode);
static void st_atiles_update_tile(st_atilesctx_t *atilesctx, 
 unsigned layer_index, unsigned row, unsigned col, st_atile_t *tile, 
 bool update_subtiles);
static const st_atile_t *st_atiles_get_tile(const st_atilesctx_t *atilesctx, 
 unsigned layer_index, unsigned row, unsigned col);
static void st_atiles_get_subtile(const st_atilesctx_t *atilesctx, 
 st_atsubtile_t dst[ST_ATSPP_LEN], unsigned layer_index, unsigned row, 
 unsigned col);
static void st_atiles_get_subtile_sprite(const st_atilesctx_t *atilesctx, 
 const st_sprite_t *dst[ST_ATSPP_LEN], unsigned layer_index, unsigned row, 
 unsigned col);
static void st_atiles_update(st_atilesctx_t *atilesctx);

static st_atilesctx_funcs_t atilesctx_funcs = {
    ST_MODCTX_FUNCS,
    .tileset_load         = st_atiles_tileset_load,
    .tileset_from_texture = st_atiles_tileset_from_texture,
    .tileset_from_atlas   = st_atiles_tileset_from_atlas,
    .add_layer            = st_atiles_add_layer,
    .update_tile          = st_atiles_update_tile,
    .get_tile             = st_atiles_get_tile,
    .get_subtile          = st_atiles_get_subtile,
    .get_subtile_sprite   = st_atiles_get_subtile_sprite,
    .update               = st_atiles_update,
};

static const st_modprerq_t mod_prereqs[] = {
    { "atlas", NULL, },
    { "dynarr", NULL, },
    { "logger", NULL, },
    { "sprite", NULL, },
    { "texture", NULL, },
    {0},
};

st_moddata_t *st_module_atiles_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("atiles", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_atiles_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_atiles_simple_init(modsmgr);
}
#endif

static st_atilesctx_t *st_atiles_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_dynarrctx_t *dynarr_ctx = (st_dynarrctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "dynarr", NULL);
    st_texturectx_t *texture_ctx = (st_texturectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "texture", NULL);
    st_spritectx_t *sprite_ctx = (st_spritectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "sprite", NULL);
    st_atilesctx_t *atiles_ctx;
    
    if (!logger_ctx || !dynarr_ctx || !texture_ctx || !sprite_ctx)
        return NULL;

    atiles_ctx = (st_atilesctx_t *)st_modctx_new("atiles", "simple", 
     sizeof(st_atilesctx_t), NULL, &atilesctx_funcs, 
     (st_object_dtor_t)st_atiles_quit);
    if (!atiles_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "atiles_simple: Unable to create new atiles ctx object");

        return NULL;
    }

    atiles_ctx->logger_ctx = logger_ctx;
    atiles_ctx->dynarr_ctx = dynarr_ctx;
    atiles_ctx->texture_ctx = texture_ctx;
    atiles_ctx->sprite_ctx = sprite_ctx;
    atiles_ctx->layers = ST_DYNARRCTX_CALL(dynarr_ctx, create, 
     sizeof(st_atilelayer_t *), 8);
    if (!atiles_ctx->layers) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "atiles_simple: Unable to create dynamic array for layers");

        goto layers_create_fail;
    }

    atiles_ctx->tilesets = ST_DYNARRCTX_CALL(dynarr_ctx, create, 
     sizeof(st_atileset_t), 8);
    if (!atiles_ctx->tilesets) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "atiles_simple: Unable to create dynamic array for tilesets");

        goto tilesets_create_fail;
    }

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "atiles_simple: Atiles context initialized");

    return atiles_ctx;

tilesets_create_fail:
    ST_DYNARR_CALL(atiles_ctx->layers, destroy);
layers_create_fail:
    free(atiles_ctx);

    return NULL;
}

static void destroy_tileset(st_atileset_t *tileset) {
    for (size_t i = 0; i < ST_ATS_LEN; i++)
        if (tileset->sprites[i])
            ST_SPRITE_CALL(tileset->sprites[i], destroy);
    if (tileset->texture_owned)
        ST_TEXTURE_CALL(tileset->texture, destroy);
    /* not needed to free tileset because memory owned by dynarr */
}

static void st_atiles_quit(st_atilesctx_t *atilesctx) {
    ST_LOGGERCTX_CALL(atilesctx->logger_ctx, info,
     "atiles_simple: Atiles context destroyed");
    for (size_t i = 0; 
     i < ST_DYNARR_CALL(atilesctx->layers, get_elements_count); i++)
        free(*(st_atilelayer_t **)ST_DYNARR_CALL(atilesctx->layers, get, i));
    ST_DYNARR_CALL(atilesctx->layers, destroy);
    for (size_t i = 0; 
     i < ST_DYNARR_CALL(atilesctx->tilesets, get_elements_count); i++) {
        st_atileset_t tileset;
        ST_DYNARR_CALL(atilesctx->tilesets, extract, &tileset, i);
        destroy_tileset(&tileset);
     }
    ST_DYNARR_CALL(atilesctx->tilesets, destroy);
    free(atilesctx);
}

static const st_atileset_t *st_atiles_tileset_from_texture_impl(
 st_atilesctx_t *atilesctx, st_texture_t *texture, bool owned) {
    st_atileset_t tileset;
    unsigned      texture_width;
    unsigned      texture_height;
    unsigned      subtile_width;
    unsigned      subtile_height;

    if (!texture) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to create tileset from NULL texture");

        return NULL;
    }

    texture_width = ST_TEXTURE_CALL(texture, get_width);
    texture_height = ST_TEXTURE_CALL(texture, get_height);
    if (!texture_width || !texture_height) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to create tileset from texture with zero width "
         "or height");

        return NULL;
    }

    subtile_width = texture_width / 5;
    subtile_height = texture_height / 3;

    tileset.texture = texture;
    tileset.texture_owned = owned;

    tileset.sprites[ST_ATS_NORTH_WALL] = ST_SPRITECTX_CALL(atilesctx->sprite_ctx, 
     from_texture_region, texture, subtile_width, 0, subtile_width, 
     subtile_height);
    tileset.sprites[ST_ATS_SOUTH_WALL] = ST_SPRITECTX_CALL(atilesctx->sprite_ctx, 
     from_texture_region, texture, subtile_width, subtile_height * 2, 
     subtile_width, subtile_height);
    tileset.sprites[ST_ATS_WEST_WALL] = ST_SPRITECTX_CALL(atilesctx->sprite_ctx, 
     from_texture_region, texture, 0, subtile_height, subtile_width, 
     subtile_height);
    tileset.sprites[ST_ATS_EAST_WALL] = ST_SPRITECTX_CALL(atilesctx->sprite_ctx, 
     from_texture_region, texture, subtile_width * 2, subtile_height, 
     subtile_width, subtile_height);
    tileset.sprites[ST_ATS_NORTH_WEST_EXTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, 0, 0, subtile_width, 
     subtile_height);
    tileset.sprites[ST_ATS_NORTH_EAST_EXTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, subtile_width * 2, 0, 
     subtile_width, subtile_height);
    tileset.sprites[ST_ATS_SOUTH_WEST_EXTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, 0, subtile_height * 2, 
     subtile_width, subtile_height);
    tileset.sprites[ST_ATS_SOUTH_EAST_EXTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, subtile_width * 2, 
     subtile_height * 2, subtile_width, subtile_height);
    tileset.sprites[ST_ATS_NORTH_WEST_INTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, subtile_width * 4, 
     subtile_height, subtile_width, subtile_height);
    tileset.sprites[ST_ATS_NORTH_EAST_INTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, subtile_width * 3, 
     subtile_height, subtile_width, subtile_height);
    tileset.sprites[ST_ATS_SOUTH_WEST_INTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, subtile_width * 4, 0, 
     subtile_width, subtile_height);
    tileset.sprites[ST_ATS_SOUTH_EAST_INTERNAL_CORNER] = ST_SPRITECTX_CALL(
     atilesctx->sprite_ctx, from_texture_region, texture, subtile_width * 3, 0, 
     subtile_width, subtile_height);
    tileset.sprites[ST_ATS_ENTIRE] = ST_SPRITECTX_CALL(atilesctx->sprite_ctx, 
     from_texture_region, texture, subtile_width, subtile_height, subtile_width, 
     subtile_height);
    tileset.sprites[ST_ATS_EMPTY] = NULL;

    for (size_t i = 0; i < ST_ATS_NONEMPTY_LEN; i++) {
        if (!tileset.sprites[i]) {
            ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
             "atiles_simple: Unable to create tileset from texture with "
             "subimage");

            goto sprite_create_fail;
        }
    }

    ST_DYNARR_CALL(atilesctx->tilesets, append, &tileset);

    return ST_DYNARR_CALL(atilesctx->tilesets, get_last);

sprite_create_fail:
invalid_subimage_name:
    for (size_t i = 0; i < ST_ATS_NONEMPTY_LEN; i++)
        if (tileset.sprites[i])
            ST_SPRITE_CALL(tileset.sprites[i], destroy);
    ST_TEXTURE_CALL(tileset.texture, destroy);

    return NULL;
}

static const st_atileset_t *st_atiles_tileset_load(st_atilesctx_t *atilesctx, 
 const char *filename) {
    const st_atileset_t *tileset;
    st_texture_t        *texture = ST_TEXTURECTX_CALL(atilesctx->texture_ctx, 
     load, filename);

    if (!texture) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to load texture for tileset: %s", filename);

        return NULL;
    }

    tileset = st_atiles_tileset_from_texture_impl(atilesctx, texture, true);
    if (!tileset) {
        ST_TEXTURE_CALL(texture, destroy);

        return NULL;
    }

    return tileset;
}

/* This function adds texture_owned = false to the tileset */
static const st_atileset_t *st_atiles_tileset_from_texture(
 st_atilesctx_t *atilesctx, st_texture_t *texture) {
    return st_atiles_tileset_from_texture_impl(atilesctx, texture, false);
}

static const st_atileset_t *st_atiles_tileset_from_atlas(
 st_atilesctx_t *atilesctx, const st_atlas_t *atlas) {
    st_atileset_t tileset;
    const char   *texture_filename;

    if (!atlas) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to create tileset from NULL atlas");

        return NULL;
    }

    if (ST_ATLAS_CALL(atlas, get_subimages_count) != ST_ATS_LEN) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to create tileset from atlas with invalid "
         "subimages count");

        return NULL;
    }

    texture_filename = ST_ATLAS_CALL(atlas, get_filename);
    tileset.texture = ST_TEXTURECTX_CALL(atilesctx->texture_ctx, load, 
     texture_filename);
    if (!tileset.texture) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to load texture for tileset: %s", 
         texture_filename);

        return NULL;
    }
    tileset.texture_owned = true;

    memset(tileset.sprites, 0, sizeof(st_sprite_t *) * ST_ATS_LEN);

    for (size_t i = 0; i < ST_ATS_LEN; i++) {
        const char   *subimage_name = ST_ATLAS_CALL(atlas, get_subimage_name, i);
        st_sprite_t **sprite;

        if (strcmp(subimage_name, "n") == 0) {
            sprite = &tileset.sprites[ST_ATS_NORTH_WALL];
        } else if (strcmp(subimage_name, "s") == 0) {
            sprite = &tileset.sprites[ST_ATS_SOUTH_WALL];
        } else if (strcmp(subimage_name, "w") == 0) {
            sprite = &tileset.sprites[ST_ATS_WEST_WALL];
        } else if (strcmp(subimage_name, "e") == 0) {
            sprite = &tileset.sprites[ST_ATS_EAST_WALL];
        } else if (strcmp(subimage_name, "nwe") == 0) {
            sprite = &tileset.sprites[ST_ATS_NORTH_WEST_EXTERNAL_CORNER];
        } else if (strcmp(subimage_name, "nee") == 0) {
            sprite = &tileset.sprites[ST_ATS_NORTH_EAST_EXTERNAL_CORNER];
        } else if (strcmp(subimage_name, "swe") == 0) {
            sprite = &tileset.sprites[ST_ATS_SOUTH_WEST_EXTERNAL_CORNER];
        } else if (strcmp(subimage_name, "see") == 0) {
            sprite = &tileset.sprites[ST_ATS_SOUTH_EAST_EXTERNAL_CORNER];
        } else if (strcmp(subimage_name, "nwi") == 0) {
            sprite = &tileset.sprites[ST_ATS_NORTH_WEST_INTERNAL_CORNER];
        } else if (strcmp(subimage_name, "nei") == 0) {
            sprite = &tileset.sprites[ST_ATS_NORTH_EAST_INTERNAL_CORNER];
        } else if (strcmp(subimage_name, "swi") == 0) {
            sprite = &tileset.sprites[ST_ATS_SOUTH_WEST_INTERNAL_CORNER];
        } else if (strcmp(subimage_name, "sei") == 0) {
            sprite = &tileset.sprites[ST_ATS_SOUTH_EAST_INTERNAL_CORNER];
        } else if (strcmp(subimage_name, "entire") == 0) {
            sprite = &tileset.sprites[ST_ATS_ENTIRE];
        } else {
            ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
             "atiles_simple: Unable to create tileset from atlas with invalid "
             "subimage name: %s", subimage_name);

            goto invalid_subimage_name;
        }

        *sprite = ST_SPRITECTX_CALL(atilesctx->sprite_ctx, from_texture_region, 
         tileset.texture, ST_ATLAS_CALL(atlas, get_subimage_x, i), 
         ST_ATLAS_CALL(atlas, get_subimage_y, i), 
         ST_ATLAS_CALL(atlas, get_subimage_width, i), 
         ST_ATLAS_CALL(atlas, get_subimage_height, i));
        if (!*sprite) {
            ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
             "atiles_simple: Unable to create tileset from atlas with invalid "
             "subimage: %s", subimage_name);

            goto sprite_create_fail;
        }
    }

    ST_DYNARR_CALL(atilesctx->tilesets, append, &tileset);

    return ST_DYNARR_CALL(atilesctx->tilesets, get_last); 

sprite_create_fail:
invalid_subimage_name:
    for (size_t i = 0; i < ST_ATS_LEN; i++)
        if (tileset.sprites[i])
            ST_SPRITE_CALL(tileset.sprites[i], destroy);
    ST_TEXTURE_CALL(tileset.texture, destroy);

    return NULL;
}

static int st_atiles_add_layer(st_atilesctx_t *atilesctx, unsigned rows, 
 unsigned cols, unsigned tile_width, unsigned tile_height, 
 st_atnghbrmode_t neighbor_mode) {
    st_atilelayer_t *layer;
    
    if (rows == 0 || cols == 0 || tile_width == 0 || tile_height == 0) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to add layer with zero rows, columns, tile "
         "width or tile height");

        return -1;
    }

    layer = malloc(
     sizeof(st_atilelayer_t) + sizeof(st_atilecell_t) * rows * cols);
    if (!layer) {
        ST_LOGGERCTX_CALL(atilesctx->logger_ctx, error,
         "atiles_simple: Unable to allocate memory for layer");

        return -1;
    }

    layer->rows = rows;
    layer->cols = cols;
    layer->tile_width = tile_width;
    layer->tile_height = tile_height;
    layer->neighbor_mode = neighbor_mode;

    for (unsigned i = 0; i < rows * cols; i++) {
        layer->cells[i].tile = NULL;
        layer->cells[i].subtiles[ST_ATSPP_NW] = ST_ATS_EMPTY;
        layer->cells[i].subtiles[ST_ATSPP_NE] = ST_ATS_EMPTY;
        layer->cells[i].subtiles[ST_ATSPP_SE] = ST_ATS_EMPTY;
        layer->cells[i].subtiles[ST_ATSPP_SW] = ST_ATS_EMPTY;
    }

    if (!ST_DYNARR_CALL(atilesctx->layers, append, &layer)) {
        free(layer);

        return -1;
    }

    return ST_DYNARR_CALL(atilesctx->layers, get_elements_count) - 1;
}

static bool is_connected_to_tile(const st_atilelayer_t *layer, unsigned row, 
 unsigned col, const st_atile_t *tile) {
    const st_atile_t *other_tile;

    if (!layer || !tile || row >= layer->rows || col >= layer->cols)
        return false;

    other_tile = layer->cells[row * layer->cols + col].tile;
    if (!other_tile)
        return false;

    if (layer->neighbor_mode == ST_ATNM_ANY)
        return true;

    return other_tile == tile;
}

static void update_subtiles(st_atilelayer_t *layer, unsigned row, unsigned col) {
    st_atilecell_t *cell;
    st_atile_t     *tile;
    bool            north;
    bool            south;
    bool            east;
    bool            west;
    bool            north_east;
    bool            south_east;
    bool            south_west;
    bool            north_west;

    if (!layer || row >= layer->rows || col >= layer->cols)
        return;

    cell = &layer->cells[row * layer->cols + col];
    tile = cell->tile;
    if (!tile) {
        cell->subtiles[ST_ATSPP_NW] = ST_ATS_EMPTY;
        cell->subtiles[ST_ATSPP_NE] = ST_ATS_EMPTY;
        cell->subtiles[ST_ATSPP_SE] = ST_ATS_EMPTY;
        cell->subtiles[ST_ATSPP_SW] = ST_ATS_EMPTY;

        return;
    }

    north = row > 0 ? is_connected_to_tile(layer, row - 1, col, tile) : false;
    south = row + 1 < layer->rows
        ? is_connected_to_tile(layer, row + 1, col, tile)
        : false;
    east = col + 1 < layer->cols
        ? is_connected_to_tile(layer, row, col + 1, tile)
        : false;
    west = col > 0 ? is_connected_to_tile(layer, row, col - 1, tile) : false;

    north_east = row > 0 && col + 1 < layer->cols
        ? is_connected_to_tile(layer, row - 1, col + 1, tile)
        : false;
    south_east = row + 1 < layer->rows && col + 1 < layer->cols
        ? is_connected_to_tile(layer, row + 1, col + 1, tile)
        : false;
    south_west = row + 1 < layer->rows && col > 0
        ? is_connected_to_tile(layer, row + 1, col - 1, tile)
        : false;
    north_west = row > 0 && col > 0
        ? is_connected_to_tile(layer, row - 1, col - 1, tile)
        : false;

    if (north) {
        if (west) {
            cell->subtiles[ST_ATSPP_NW] = north_west
                ? ST_ATS_ENTIRE
                : ST_ATS_NORTH_WEST_INTERNAL_CORNER;
        } else {
            cell->subtiles[ST_ATSPP_NW] = ST_ATS_WEST_WALL;
        }
    } else {
        if (west) {
            cell->subtiles[ST_ATSPP_NW] = ST_ATS_NORTH_WALL;
        } else {
            cell->subtiles[ST_ATSPP_NW] = ST_ATS_NORTH_WEST_EXTERNAL_CORNER;
        }
    }

    if (north) {
        if (east) {
            cell->subtiles[ST_ATSPP_NE] = north_east
                ? ST_ATS_ENTIRE
                : ST_ATS_NORTH_EAST_INTERNAL_CORNER;
        } else {
            cell->subtiles[ST_ATSPP_NE] = ST_ATS_EAST_WALL;
        }
    } else {
        if (east) {
            cell->subtiles[ST_ATSPP_NE] = ST_ATS_NORTH_WALL;
        } else {
            cell->subtiles[ST_ATSPP_NE] = ST_ATS_NORTH_EAST_EXTERNAL_CORNER;
        }
    }

    if (south) {
        if (east) {
            cell->subtiles[ST_ATSPP_SE] = south_east
                ? ST_ATS_ENTIRE
                : ST_ATS_SOUTH_EAST_INTERNAL_CORNER;
        } else {
            cell->subtiles[ST_ATSPP_SE] = ST_ATS_EAST_WALL;
        }
    } else {
        if (east) {
            cell->subtiles[ST_ATSPP_SE] = ST_ATS_SOUTH_WALL;
        } else {
            cell->subtiles[ST_ATSPP_SE] = ST_ATS_SOUTH_EAST_EXTERNAL_CORNER;
        }
    }

    if (south) {
        if (west) {
            cell->subtiles[ST_ATSPP_SW] = south_west
                ? ST_ATS_ENTIRE
                : ST_ATS_SOUTH_WEST_INTERNAL_CORNER;
        } else {
            cell->subtiles[ST_ATSPP_SW] = ST_ATS_WEST_WALL;
        }
    } else {
        if (west) {
            cell->subtiles[ST_ATSPP_SW] = ST_ATS_SOUTH_WALL;
        } else {
            cell->subtiles[ST_ATSPP_SW] = ST_ATS_SOUTH_WEST_EXTERNAL_CORNER;
        }
    }
}

static void st_atiles_update_tile(st_atilesctx_t *atilesctx, 
 unsigned layer_index, unsigned row, unsigned col, st_atile_t *tile, 
 bool do_update_subtiles) {
    st_atilelayer_t *layer = NULL;
    bool             extracted = ST_DYNARR_CALL(atilesctx->layers, extract, 
     &layer, layer_index);

    if (!extracted)
        return;

    if (col >= layer->cols || row >= layer->rows)
        return;

    layer->cells[row * layer->cols + col].tile = tile;

    if (do_update_subtiles) {
        update_subtiles(layer, row, col);
        if (row > 0) {
            update_subtiles(layer, row - 1, col);
            if (col > 0)
                update_subtiles(layer, row - 1, col - 1);
            if (col < layer->cols - 1)
                update_subtiles(layer, row - 1, col + 1);
        }
        if (row < layer->rows - 1) {
            update_subtiles(layer, row + 1, col);
            if (col > 0)
                update_subtiles(layer, row + 1, col - 1);
            if (col < layer->cols - 1)
                update_subtiles(layer, row + 1, col + 1);
        }
        if (col > 0)
            update_subtiles(layer, row, col - 1);
        if (col < layer->cols - 1)
            update_subtiles(layer, row, col + 1);
    }
}

static const st_atile_t *st_atiles_get_tile(const st_atilesctx_t *atilesctx, 
 unsigned layer_index, unsigned row, unsigned col) {
    st_atilelayer_t * const *layer = ST_DYNARR_CALL(atilesctx->layers, 
     get, layer_index);
    if (!layer)
        return NULL;

    return (layer && col < (*layer)->cols && row < (*layer)->rows)
        ? (*layer)->cells[row * (*layer)->cols + col].tile 
        : NULL;
}

static void st_atiles_get_subtile(const st_atilesctx_t *atilesctx, 
 st_atsubtile_t dst[ST_ATSPP_LEN], unsigned layer_index, unsigned row, 
 unsigned col) {
    st_atilelayer_t * const *layer = ST_DYNARR_CALL(atilesctx->layers, get, 
     layer_index);

    for (size_t i = 0; i < ST_ATSPP_LEN; i++)
        dst[i] = ST_ATS_EMPTY;

    if (!layer)
        return;

    if (col >= (*layer)->cols || row >= (*layer)->rows)
        return;

    dst[ST_ATSPP_NW] = (*layer)->cells[row * (*layer)->cols + col].subtiles[
     ST_ATSPP_NW];
    dst[ST_ATSPP_NE] = (*layer)->cells[row * (*layer)->cols + col].subtiles[
     ST_ATSPP_NE];
    dst[ST_ATSPP_SE] = (*layer)->cells[row * (*layer)->cols + col].subtiles[
     ST_ATSPP_SE];
    dst[ST_ATSPP_SW] = (*layer)->cells[row * (*layer)->cols + col].subtiles[
     ST_ATSPP_SW];
}

static void st_atiles_get_subtile_sprite(const st_atilesctx_t *atilesctx, 
 const st_sprite_t *dst[ST_ATSPP_LEN], unsigned layer_index, unsigned row, 
 unsigned col) {
    st_atsubtile_t    subtiles[ST_ATSPP_LEN];
    st_atileset_t    *tileset;
    st_atilelayer_t * const *layer = ST_DYNARR_CALL(atilesctx->layers, get, 
     layer_index);
    
    memset(dst, 0, sizeof(st_sprite_t *) * ST_ATSPP_LEN);
    
    if (!layer)
        return;

    if (col >= (*layer)->cols || row >= (*layer)->rows)
        return;

    tileset = (*layer)->cells[row * (*layer)->cols + col].tile;
    if (!tileset)
        return;

    subtiles[ST_ATSPP_NW] = (*layer)->cells[
     row * (*layer)->cols + col].subtiles[ST_ATSPP_NW];
    subtiles[ST_ATSPP_NE] = (*layer)->cells[
     row * (*layer)->cols + col].subtiles[ST_ATSPP_NE];
    subtiles[ST_ATSPP_SE] = (*layer)->cells[
     row * (*layer)->cols + col].subtiles[ST_ATSPP_SE];
    subtiles[ST_ATSPP_SW] = (*layer)->cells[
     row * (*layer)->cols + col].subtiles[ST_ATSPP_SW];

    for (size_t i = 0; i < ST_ATSPP_LEN; i++)
        dst[i] = tileset->sprites[subtiles[i]];
}

static void st_atiles_update(st_atilesctx_t *atilesctx) {
    st_atilelayer_t * const *layers = ST_DYNARR_CALL(atilesctx->layers, 
     get_all);
    if (!layers)
        return;

    for (size_t i = 0; 
     i < ST_DYNARR_CALL(atilesctx->layers, get_elements_count); i++) {
        st_atilelayer_t *layer = layers[i];
        if (!layer)
            continue;

        for (size_t j = 0; j < layer->rows * layer->cols; j++) 
            update_subtiles(layer, j / layer->cols, j % layer->cols);
    }
}
