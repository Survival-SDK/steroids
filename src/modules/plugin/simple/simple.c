#include "simple.h"

#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_pluginctx_t *st_plugin_init(const st_param_t params[]);
static void st_plugin_quit(st_pluginctx_t *plugin_ctx);

static bool st_plugin_load(st_pluginctx_t *plugin_ctx, const char *filename,
 bool force);
static bool st_plugin_memload(st_pluginctx_t *plugin_ctx, const void *data,
 size_t size, bool force);

static st_pluginctx_funcs_t pluginctx_funcs = {
    st_modctx_funcs,
    .load    = st_plugin_load,
    .memload = st_plugin_memload,
};

static const st_modprerq_t mod_prereqs[] = {
    { "fs", NULL, },
    { "logger", NULL, },
    { "pathtools", NULL, },
    { "so", NULL, },
    { "spcpaths", NULL, },
    { "zip", NULL, },
    {0},
};

st_moddata_t *st_module_plugin_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("plugin", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_plugin_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_plugin_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "plugin";
static const char *st_module_name = "simple";

static st_pluginctx_t *st_plugin_init(const st_param_t params[]) {
    st_modsmgr_t          *modsmgr = st_modctx_get_param_as_ptr(params,
     "modsmgr");
    struct st_loggerctx_s *logger_ctx = (
     struct st_loggerctx_s *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    st_fsctx_t            *fs_ctx = (st_fsctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "fs", NULL);
    st_pathtoolsctx_t     *pathtools_ctx = (st_pathtoolsctx_t *)ST_MODSMGR_CALL(
     modsmgr, get_singleton, "pathtools", NULL);
    st_soctx_t            *so_ctx = (st_soctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "so", NULL);
    st_spcpathsctx_t      *spcpaths_ctx = (st_spcpathsctx_t *)ST_MODSMGR_CALL(
     modsmgr, get_singleton, "spcpaths", NULL);
    st_zipctx_t           *zip_ctx = (st_zipctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "zip", NULL);
    st_pluginctx_t        *plugin_ctx = (st_pluginctx_t *)st_modctx_new(
     "plugin", "simple", sizeof(st_pluginctx_t), NULL, &pluginctx_funcs,
     (st_object_dtor_t)st_plugin_quit);

    if (!plugin_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "plugin_simple: Unable to create new plugin ctx object");

        return NULL;
    }

    plugin_ctx->modsmgr       = modsmgr;
    plugin_ctx->fs_ctx        = fs_ctx;
    plugin_ctx->logger_ctx    = logger_ctx;
    plugin_ctx->pathtools_ctx = pathtools_ctx;
    plugin_ctx->so_ctx        = so_ctx;
    plugin_ctx->spcpaths_ctx  = spcpaths_ctx;
    plugin_ctx->zip_ctx       = zip_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "plugin_simple: Plugin loader initialized");

    return plugin_ctx;
}

static void st_plugin_quit(st_pluginctx_t *plugin_ctx) {
    ST_LOGGERCTX_CALL(plugin_ctx->logger_ctx, info,
     "plugin_simple: Plugin loader destroyed");
    free(plugin_ctx);
}

static bool st_plugin_load_impl(st_pluginctx_t *plugin_ctx, st_zip_t *zip,
 const char *filename, bool force) {
    ssize_t zip_entries_count;
    char    tmp_path[PATH_MAX];
    char    triplet_path[PATH_MAX];

    zip_entries_count = ST_ZIP_CALL(zip, get_entries_count);
    if (zip_entries_count == -1) {
        ST_LOGGERCTX_CALL(plugin_ctx->logger_ctx, error,
         "plugin_simple: Unable to get entries count in plugin \"%s\"",
         filename);

        goto fail;
    }

    ST_SPCPATHSCTX_CALL(plugin_ctx->spcpaths_ctx, get_cache_path, tmp_path,
     PATH_MAX, "steroids");

    if (!ST_PATHTOOLSCTX_CALL(plugin_ctx->pathtools_ctx, concat, triplet_path,
     PATH_MAX, tmp_path, ST_TRIPLET)) {
        ST_LOGGERCTX_CALL(plugin_ctx->logger_ctx, error,
         "plugin_simple: Unable to get triplet path for plugin \"%s\"",
         filename);

        return false;
    }

    for (size_t i = 0; i < (size_t)zip_entries_count; i++) {
        char             entry_name[PATH_MAX];
        char             entry_parent_dir[PATH_MAX];
        char             so_filename[PATH_MAX];
        st_modinitfunc_t modinit_func;
        st_so_t         *so;

        if (ST_ZIP_CALL(zip, get_entry_type, i) == ST_ZET_DIR ||
         !ST_ZIP_CALL(zip, get_entry_name, entry_name, PATH_MAX, i) ||
         !ST_PATHTOOLSCTX_CALL(plugin_ctx->pathtools_ctx, get_parent_dir,
          entry_parent_dir, PATH_MAX, entry_name) ||
         strcmp(entry_parent_dir, ST_TRIPLET) != 0)
            continue;

        if (!ST_PATHTOOLSCTX_CALL(plugin_ctx->pathtools_ctx, concat, so_filename,
          PATH_MAX, triplet_path, basename(entry_name))) {
            ST_LOGGERCTX_CALL(plugin_ctx->logger_ctx, error,
             "plugin_simple: Unable to get output filename for plugin entry "
             "\"%s\"", entry_name);

            goto fail;
        }

        if (!ST_FSCTX_CALL(plugin_ctx->fs_ctx, mkdir, triplet_path) ||
         !ST_ZIP_CALL(zip, extract_entry, i, so_filename))
            goto fail;

        so = ST_SOCTX_CALL(plugin_ctx->so_ctx, open, so_filename);
        if (!so)
            goto fail;

        modinit_func = ST_SO_CALL(so, load_symbol, "st_module_init");
        if (!modinit_func) {
            ST_LOGGERCTX_CALL(plugin_ctx->logger_ctx, error,
             "plugin_simple: Module %s has not function \"st_module_init\"");

            ST_SO_CALL(so, destroy);

            goto fail;
        }

        ST_ZIP_CALL(zip, destroy);

        return ST_MODSMGR_CALL(plugin_ctx->modsmgr, load_module, modinit_func,
         force);
    }

fail:
    ST_LOGGERCTX_CALL(plugin_ctx->logger_ctx, error,
     "plugin_simple: Plugin \"%s\" cannot be loaded on this platform",
     filename);
    ST_ZIP_CALL(zip, destroy);

    return false;
}

static bool st_plugin_load(st_pluginctx_t *plugin_ctx, const char *filename,
 bool force) {
    st_zip_t *zip = ST_ZIPCTX_CALL(plugin_ctx->zip_ctx, open, filename);

    return zip
        ? st_plugin_load_impl(plugin_ctx, zip, filename, force)
        : false;
}

static bool st_plugin_memload(st_pluginctx_t *plugin_ctx, const void *data,
 size_t size, bool force) {
    st_zip_t *zip = ST_ZIPCTX_CALL(plugin_ctx->zip_ctx, memopen, data, size);

    return zip
        ? st_plugin_load_impl(plugin_ctx, zip, "", force)
        : false;
}
