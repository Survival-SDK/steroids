#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/modules/fs.h"
#include "steroids/types/modules/pathtools.h"
#include "steroids/types/modules/so.h"
#include "steroids/types/modules/spcpaths.h"
#include "steroids/types/modules/zip.h"
#include "steroids/types/object.h"

#ifndef ST_PLUGINCTX_T_DEFINED
    typedef struct st_pluginctx_s st_pluginctx_t;
#endif

typedef st_pluginctx_t *(*st_plugin_init_t)(st_fsctx_t *fs_ctx,
 struct st_loggerctx_s *logger_ctx,
 st_pathtoolsctx_t *pathtools_ctx, st_soctx_t *so_ctx,
 st_spcpathsctx_t *spcpaths_ctx, st_zipctx_t *zip_ctx);

typedef bool (*st_plugin_load_t)(st_pluginctx_t *plugin_ctx,
 const char *filename, bool force);
typedef bool (*st_plugin_memload_t)(st_pluginctx_t *plugin_ctx,
 const void *data, size_t size, bool force);

typedef struct {
    st_modctx_funcs_t;
    st_plugin_load_t    load;
    st_plugin_memload_t memload;
} st_pluginctx_funcs_t;

#define ST_PLUGINCTX_CALL(object, func, ...) \
    ((st_pluginctx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
