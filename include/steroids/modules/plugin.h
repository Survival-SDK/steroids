#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/modules/fs.h"
#include "steroids/modules/pathtools.h"
#include "steroids/modules/so.h"
#include "steroids/modules/spcpaths.h"
#include "steroids/modules/zip.h"
#include "steroids/object.h"

#ifndef ST_PLUGINCTX_T_DEFINED
    typedef struct st_pluginctx_s st_pluginctx_t;
#endif

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
