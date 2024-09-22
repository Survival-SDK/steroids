#pragma once

#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/object.h"

#ifndef ST_PATHTOOLSCTX_T_DEFINED
    typedef struct st_pathtoolsctx_s st_pathtoolsctx_t;
#endif

typedef st_pathtoolsctx_t *(*st_pathtools_init_t)(
 struct st_loggerctx_s *logger_ctx);
typedef void (*st_pathtools_quit_t)(st_pathtoolsctx_t *pathtools_ctx);

typedef bool (*st_pathtools_resolve_t)(st_pathtoolsctx_t *pathtools_ctx,
 char *dst, size_t dstsize, const char *path);
typedef bool (*st_pathtools_get_parent_dir_t)(st_pathtoolsctx_t *pathtools_ctx,
 char *dst, size_t dstsize, const char *path);
typedef bool (*st_pathtools_concat_t)(st_pathtoolsctx_t *pathtools_ctx,
 char *dst, size_t dstsize, const char *path, const char *append);

typedef struct {
    st_pathtools_quit_t           quit;
    st_pathtools_resolve_t        resolve;
    st_pathtools_get_parent_dir_t get_parent_dir;
    st_pathtools_concat_t         concat;
} st_pathtoolsctx_funcs_t;

#define ST_PATHTOOLSCTX_CALL(object, func, ...) \
    ((st_pathtoolsctx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
