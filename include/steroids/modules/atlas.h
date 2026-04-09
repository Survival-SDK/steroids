#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_ATLASCTX_T_DEFINED
    typedef st_modctx_t st_atlasctx_t;
#endif
#ifndef ST_ATLAS_T_DEFINED
    typedef st_object_t st_atlas_t;
#endif

typedef st_atlas_t *(*st_atlas_load_t)(st_atlasctx_t *atlas_ctx,
 const char *filename);
typedef st_atlas_t *(*st_atlas_memload_t)(st_atlasctx_t *atlas_ctx,
 const void *data, size_t size);

/* Callbacks for loaders */
typedef st_atlas_t *(*st_atlas_create_empty_t)(st_atlasctx_t *atlas_ctx, 
 const char *filename);
typedef bool (*st_atlas_add_subimage_t)(st_atlas_t *atlas,
 const char *name, unsigned x, unsigned y, unsigned width, unsigned height);

typedef const char *(*st_atlas_get_filename_t)(const st_atlas_t *atlas);
typedef int (*st_atlas_get_subimages_count_t)(const st_atlas_t *atlas);
typedef const char *(*st_atlas_get_subimage_name_t)(const st_atlas_t *atlas,
 unsigned index);
typedef unsigned (*st_atlas_get_subimage_x_t)(const st_atlas_t *atlas, 
 unsigned index);
typedef unsigned (*st_atlas_get_subimage_y_t)(const st_atlas_t *atlas, 
 unsigned index);
typedef unsigned (*st_atlas_get_subimage_width_t)(const st_atlas_t *atlas, 
 unsigned index);
typedef unsigned (*st_atlas_get_subimage_height_t)(const st_atlas_t *atlas, 
 unsigned index);

typedef struct {
    st_modctx_funcs_t;
    st_atlas_load_t         load;
    st_atlas_memload_t      memload;
    st_atlas_create_empty_t create_empty;
} st_atlasctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_atlas_add_subimage_t        add_subimage;
    st_atlas_get_filename_t        get_filename;
    st_atlas_get_subimages_count_t get_subimages_count;
    st_atlas_get_subimage_name_t   get_subimage_name;
    st_atlas_get_subimage_x_t      get_subimage_x;
    st_atlas_get_subimage_y_t      get_subimage_y;
    st_atlas_get_subimage_width_t  get_subimage_width;
    st_atlas_get_subimage_height_t get_subimage_height;
} st_atlas_funcs_t;

#define ST_ATLASCTX_CALL(ctx, func, ...) \
    ((st_atlasctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_ATLAS_CALL(object, func, ...) \
    ((st_atlas_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
