#include <stdbool.h>
#include <stdlib.h>

#include "modules_manager.h"

// #include "steroids/types/modules/fs.h"
// #include "steroids/types/modules/ini.h"
#include "steroids/types/modules/logger.h"
// #include "steroids/types/modules/opts.h"
// #include "steroids/types/modules/pathtools.h"
// #include "steroids/types/modules/plugin.h"
// #include "steroids/types/modules/runner.h"
// #include "steroids/types/modules/so.h"
// #include "steroids/types/modules/spcpaths.h"
// #include "steroids/types/modules/zip.h"

// static st_fs_init_t     st_fs_init;
// static st_ini_init_t    st_ini_init;
static st_logger_init_t st_logger_init;
// static st_opts_init_t   st_opts_init;

// static st_runner_init_t st_runner_init;
// static st_runner_quit_t st_runner_quit;
// static st_runner_run_t  st_runner_run;

// static st_pathtools_init_t st_pathtools_init;
// static st_plugin_init_t    st_plugin_init;
// static st_so_init_t        st_so_init;
// static st_spcpaths_init_t  st_spcpaths_init;
// static st_zip_init_t       st_zip_init;

#define LOAD_FUNCTION(module, function)                                        \
    st_##module##_##function = st_modsmgr_get_function(modsmgr, #module, NULL, \
     #function);                                                               \
    if (!st_##module##_##function) {                                           \
        ST_LOGGERCTX_CALL(logger_ctx, error,                                   \
         "steroids: Unable to load function \"%s\"", #function);               \
        return false;                                                          \
    }

static bool init_funcs(st_modsmgr_t *modsmgr,
 struct st_loggerctx_s *logger_ctx) {
    // LOAD_FUNCTION(fs, init);
    // LOAD_FUNCTION(ini, init);
    // LOAD_FUNCTION(opts, init);

//     LOAD_FUNCTION(runner, init);
//     LOAD_FUNCTION(runner, quit);
//     LOAD_FUNCTION(runner, run);

    // LOAD_FUNCTION(pathtools, init);
    // LOAD_FUNCTION(plugin,    init);
    // LOAD_FUNCTION(so,        init);
    // LOAD_FUNCTION(spcpaths,  init);
    // LOAD_FUNCTION(zip,       init);

    return true;
}

int main(int argc, char **argv) {
    st_modsmgr_t          *modsmgr = st_modsmgr_init();
    // st_fsctx_t            *fs_ctx;
    // st_inictx_t           *ini_ctx;
    struct st_loggerctx_s *logger_ctx;
    // st_optsctx_t          *opts_ctx;
    // st_modctx_t  *runner;
    // st_pathtoolsctx_t *pathtools_ctx;
    // st_pluginctx_t    *plugin_ctx;
    // st_soctx_t        *so_ctx;
    // st_spcpathsctx_t  *spcpaths_ctx;
    // st_zipctx_t       *zip_ctx;
    int                exitcode = EXIT_SUCCESS;

    st_logger_init = st_modsmgr_get_ctor(modsmgr, "logger", NULL);
    logger_ctx = st_logger_init(NULL);

    if (!init_funcs(modsmgr, logger_ctx)) {
        exitcode = EXIT_FAILURE;

        goto init_funcs_fail;
    }

    // ini_ctx = st_ini_init(logger_ctx);
    // opts_ctx = st_opts_init(argc, argv, logger_ctx);
    // pathtools_ctx = st_pathtools_init(logger_ctx);
    // fs_ctx = st_fs_init(logger_ctx, pathtools_ctx);
    // so_ctx = st_so_init(logger_ctx);
    // spcpaths_ctx = st_spcpaths_init(logger_ctx);
    // zip_ctx = st_zip_init(fs_ctx, logger_ctx, pathtools_ctx);
    // plugin_ctx = st_plugin_init(fs_ctx, logger_ctx, pathtools_ctx, so_ctx,
    //  spcpaths_ctx, zip_ctx);
//     runner = st_runner_init(ini, logger, opts, pathtools, plugin);

//     st_runner_run(runner, NULL);

//     st_runner_quit(runner);
    // ST_PLUGINCTX_CALL(plugin_ctx, quit);
    // ST_ZIPCTX_CALL(zip_ctx, quit);
    // ST_SPCPATHSCTX_CALL(spcpaths_ctx, quit);
    // ST_SOCTX_CALL(so_ctx, quit);
    // ST_FSCTX_CALL(fs_ctx, quit);
    // ST_PATHTOOLSCTX_CALL(pathtools_ctx, quit);
    // ST_OPTSCTX_CALL(opts_ctx, quit);
    // ST_INICTX_CALL(ini_ctx, quit);

init_funcs_fail:
    ST_LOGGERCTX_CALL(logger_ctx, release);
    st_modsmgr_destroy(modsmgr);

    return exitcode;
}
