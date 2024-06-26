#pragma once

#include "config.h" // IWYU pragma: keep
#include "types.h"  // IWYU pragma: keep
#include "steroids/atlas.h"

static st_atlasctx_funcs_t st_atlas_simple_funcs = {
    .quit   = st_atlas_quit,
    .create = st_atlas_create,
};

static st_modfuncentry_t st_module_atlas_simple_funcs[] = {
    {"init",            st_atlas_init},
    {"quit",            st_atlas_quit},
    {"create",          st_atlas_create},
    {"destroy",         st_atlas_destroy},
    {"set_clip",        st_atlas_set_clip},
    {"get_texture",     st_atlas_get_texture},
    {"get_clips_count", st_atlas_get_clips_count},
    {"get_clip_x",      st_atlas_get_clip_x},
    {"get_clip_y",      st_atlas_get_clip_y},
    {"get_clip_width",  st_atlas_get_clip_width},
    {"get_clip_height", st_atlas_get_clip_height},
    {NULL, NULL},
};
