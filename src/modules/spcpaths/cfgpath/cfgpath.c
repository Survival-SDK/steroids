#include "cfgpath.h"

#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <cfgpath.h>
#pragma GCC diagnostic pop

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_spcpathsctx_funcs_t spcpathsctx_funcs = {
    .quit            = st_spcpaths_quit,
    .get_config_path = st_spcpaths_get_config_path,
    .get_data_path   = st_spcpaths_get_data_path,
    .get_cache_path  = st_spcpaths_get_cache_path,
};

ST_MODULE_DEF_GET_FUNC(spcpaths_cfgpath)
ST_MODULE_DEF_INIT_FUNC(spcpaths_cfgpath)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_spcpaths_cfgpath_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "spcpaths";
static const char *st_module_name = "cfgpath";

static st_spcpathsctx_t *st_spcpaths_init(struct st_loggerctx_s *logger_ctx) {
    st_spcpathsctx_t *spcpaths_ctx = st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_spcpathsctx_t), NULL, &spcpathsctx_funcs);

    if (!spcpaths_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "spcpaths_cfgpath: unable to create new spcpaths ctx object");

        return NULL;
    }

    spcpaths_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "spcpaths_cfgpath: Special paths manager initialized.");

    return spcpaths_ctx;
}

static void st_spcpaths_quit(st_spcpathsctx_t *spcpaths_ctx) {
    ST_LOGGERCTX_CALL(spcpaths_ctx->logger_ctx, info,
     "spcpaths_cfgpath: Special paths manager destroyed");
    free(spcpaths_ctx);
}

static void st_spcpaths_get_config_path(
 __attribute__((unused)) st_spcpathsctx_t *spcpaths_ctx, char *dst,
 size_t dstlen, const char *appname) {
    get_user_config_folder(dst, (unsigned)dstlen, appname);
}

static void st_spcpaths_get_data_path(
 __attribute__((unused)) st_spcpathsctx_t *spcpaths_ctx, char *dst,
 size_t dstlen, const char *appname) {
    get_user_data_folder(dst, (unsigned)dstlen, appname);
}

static void st_spcpaths_get_cache_path(
 __attribute__((unused)) st_spcpathsctx_t *spcpaths_ctx, char *dst,
 size_t dstlen, const char *appname) {
    get_user_cache_folder(dst, (unsigned)dstlen, appname);
}
