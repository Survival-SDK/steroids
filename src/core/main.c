#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "modules_manager.h"

#include "steroids/modules/fs.h"
#include "steroids/modules/ini.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/opts.h"
#include "steroids/modules/pathtools.h"
#include "steroids/modules/plugin.h"
// #include "steroids/modules/runner.h"
#include "steroids/modules/so.h"
#include "steroids/modules/spcpaths.h"
#include "steroids/modules/zip.h"

static st_fs_init_t     st_fs_init;
static st_ini_init_t    st_ini_init;
static st_logger_init_t st_logger_init;
static st_opts_init_t   st_opts_init;

// static st_runner_init_t st_runner_init;
// static st_runner_quit_t st_runner_quit;
// static st_runner_run_t  st_runner_run;

static st_pathtools_init_t st_pathtools_init;
static st_plugin_init_t    st_plugin_init;
static st_so_init_t        st_so_init;
static st_spcpaths_init_t  st_spcpaths_init;
static st_zip_init_t       st_zip_init;

#define LOAD_CTOR(module)                                                   \
    st_##module##_init = ST_MODSMGR_CALL(modsmgr, get_ctor, #module, NULL); \
    if (!st_##module##_init) {                                              \
        ST_LOGGERCTX_CALL(logger_ctx, error,                                \
         "steroids: Unable to load constructor of %s_ctx", #module);        \
        return false;                                                       \
    }

static bool init_ctors(st_modsmgr_t *modsmgr,
 struct st_loggerctx_s *logger_ctx) {
    LOAD_CTOR(fs);
    LOAD_CTOR(ini);
    LOAD_CTOR(opts);
//     LOAD_CTOR(runner);
    LOAD_CTOR(pathtools);
    LOAD_CTOR(plugin);
    LOAD_CTOR(so);
    LOAD_CTOR(spcpaths);
    LOAD_CTOR(zip);

    return true;
}

int main(int argc, char **argv) {
    st_modsmgr_t          *modsmgr = st_modsmgr_init();
    st_fsctx_t            *fs_ctx;
    st_inictx_t           *ini_ctx;
    struct st_loggerctx_s *logger_ctx;
    st_optsctx_t          *opts_ctx;
    // st_modctx_t  *runner;
    st_pathtoolsctx_t *pathtools_ctx;
    st_pluginctx_t    *plugin_ctx;
    st_soctx_t        *so_ctx;
    st_spcpathsctx_t  *spcpaths_ctx;
    st_zipctx_t       *zip_ctx;
    int                exitcode = EXIT_SUCCESS;

    st_logger_init = ST_MODSMGR_CALL(modsmgr, get_ctor, "logger", NULL);
    if (!st_logger_init) {
        fprintf(stderr,
         "steroids: Unable to load function \"st_logger_init\"\n");
        exitcode = EXIT_FAILURE;

        goto get_logger_ctor_fail;
    }

    logger_ctx = st_logger_init(NULL);

    if (!init_ctors(modsmgr, logger_ctx)) {
        exitcode = EXIT_FAILURE;

        goto init_funcs_fail;
    }

    ini_ctx = st_ini_init(logger_ctx, modsmgr);
    opts_ctx = st_opts_init(argc, argv, logger_ctx);
    pathtools_ctx = st_pathtools_init(logger_ctx);
    fs_ctx = st_fs_init(logger_ctx, pathtools_ctx);
    so_ctx = st_so_init(logger_ctx);
    spcpaths_ctx = st_spcpaths_init(logger_ctx);
    zip_ctx = st_zip_init(fs_ctx, logger_ctx, pathtools_ctx);
    plugin_ctx = st_plugin_init(modsmgr, fs_ctx, logger_ctx, pathtools_ctx,
     so_ctx, spcpaths_ctx, zip_ctx);
//     runner = st_runner_init(ini, logger, opts, pathtools, plugin);

//     st_runner_run(runner, NULL);

//     st_runner_quit(runner);
    ST_PLUGINCTX_CALL(plugin_ctx, destroy);
    ST_ZIPCTX_CALL(zip_ctx, destroy);
    ST_SPCPATHSCTX_CALL(spcpaths_ctx, destroy);
    ST_SOCTX_CALL(so_ctx, destroy);
    ST_FSCTX_CALL(fs_ctx, destroy);
    ST_PATHTOOLSCTX_CALL(pathtools_ctx, destroy);
    ST_OPTSCTX_CALL(opts_ctx, destroy);
    ST_INICTX_CALL(ini_ctx, destroy);

init_funcs_fail:
    ST_LOGGERCTX_CALL(logger_ctx, destroy);
get_logger_ctor_fail:
    ST_MODSMGR_CALL(modsmgr, destroy);

    return exitcode;
}
