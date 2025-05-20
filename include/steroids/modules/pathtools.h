#pragma once

#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_PATHTOOLSCTX_T_DEFINED
    typedef st_modctx_t st_pathtoolsctx_t;
#endif

typedef bool (*st_pathtools_resolve_t)(st_pathtoolsctx_t *pathtools_ctx,
 char *dst, size_t dstsize, const char *path);
typedef bool (*st_pathtools_get_parent_dir_t)(st_pathtoolsctx_t *pathtools_ctx,
 char *dst, size_t dstsize, const char *path);
typedef bool (*st_pathtools_concat_t)(st_pathtoolsctx_t *pathtools_ctx,
 char *dst, size_t dstsize, const char *path, const char *append);

typedef struct {
    st_modctx_funcs_t;
    st_pathtools_resolve_t        resolve;
    st_pathtools_get_parent_dir_t get_parent_dir;
    st_pathtools_concat_t         concat;
} st_pathtoolsctx_funcs_t;

#define ST_PATHTOOLSCTX_CALL(object, func, ...) \
    ((st_pathtoolsctx_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
