#pragma once

#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/object.h"

#ifndef ST_SPCPATHSCTX_T_DEFINED
    typedef struct st_spcpathsctx_s st_spcpathsctx_t;
#endif

typedef st_spcpathsctx_t *(*st_spcpaths_init_t)(
 struct st_loggerctx_s *logger_ctx);
typedef void (*st_spcpaths_get_config_path_t)(st_spcpathsctx_t *spcpaths_ctx,
 char *dst, size_t dstlen, const char *appname);
typedef void (*st_spcpaths_get_data_path_t)(st_spcpathsctx_t *spcpaths_ctx,
 char *dst, size_t dstlen, const char *appname);
typedef void (*st_spcpaths_get_cache_path_t)(st_spcpathsctx_t *spcpaths_ctx,
 char *dst, size_t dstlen, const char *appname);

typedef struct {
    st_modctx_funcs_t;
    st_spcpaths_get_config_path_t get_config_path;
    st_spcpaths_get_data_path_t   get_data_path;
    st_spcpaths_get_cache_path_t  get_cache_path;
} st_spcpathsctx_funcs_t;

#define ST_SPCPATHSCTX_CALL(object, func, ...) \
    ((st_spcpathsctx_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
