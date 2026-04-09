#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/dynarr.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

#define ST_ATL_LOADERS_MAX 8

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
    st_dynarrctx_t *dynarr_ctx;
    st_modctx_t    *atl_loaders[ST_ATL_LOADERS_MAX];
    size_t          atl_loaders_count;
} st_atlasctx_t;

typedef struct {
    char    *name;
    unsigned x;
    unsigned y;
    unsigned width;
    unsigned height;
} st_atlsubimage_t;

typedef struct {
    st_object_t;
    char        *filename;
    st_dynarr_t *subimages;
} st_atlas_t;

#define ST_ATLASCTX_T_DEFINED
#define ST_ATLAS_T_DEFINED
