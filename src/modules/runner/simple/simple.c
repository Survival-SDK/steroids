#include "simple.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_CONFIG_FILENAME   "steroids.ini"
#define DEFAULT_DIRECTORY_NAME    "."
#define ERRMSGBUF_SIZE            128
#define RUNNABLE_MODULE_NAME_SIZE 256

static st_runnerctx_t *st_runner_init(const st_param_t params[]);
static void st_runner_quit(st_runnerctx_t *runner_ctx);

static bool st_runner_run(st_runnablectx_t *runner_ctx,
 const st_param_t params[]);

static st_runnerctx_funcs_t runnerctx_funcs = {
    st_modctx_funcs,
    .run = st_runner_run,
};

static const st_modprerq_t mod_prereqs[] = {
    { "ini",       NULL, },
    { "logger",    NULL, },
    { "opts",      NULL, },
    { "pathtools", NULL, },
    { "plugin",    NULL, },
    {0},
};

st_moddata_t *st_module_runner_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("runner", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_runner_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_runner_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static st_runnerctx_t *st_runner_init(const st_param_t params[]) {
    st_modsmgr_t      *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    const char        *default_configfile = st_modctx_get_param_as_ptr(
     params, "default-configfile");
    const char        *default_directory = st_modctx_get_param_as_ptr(
     params, "default-directory");
    st_loggerctx_t    *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_inictx_t       *ini_ctx = (st_inictx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "ini", NULL);
    st_optsctx_t      *opts_ctx = (st_optsctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "opts", NULL);
    st_pathtoolsctx_t *pathtools_ctx = (st_pathtoolsctx_t *)ST_MODSMGR_CALL(
     modsmgr, get_singleton, "pathtools", NULL);
    st_pluginctx_t    *plugin_ctx = (st_pluginctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "plugin", NULL);
    st_runnerctx_t    *runner_ctx = (st_runnerctx_t *)st_modctx_new("runner",
     "simple", sizeof(st_runnerctx_t), NULL, &runnerctx_funcs,
     (st_object_dtor_t)st_runner_quit);

    if (!runner_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "runner_simple: Unable to create new runner ctx object");

        return NULL;
    }

    runner_ctx->modsmgr            = modsmgr;
    runner_ctx->ini_ctx            = ini_ctx;
    runner_ctx->logger_ctx         = logger_ctx;
    runner_ctx->opts_ctx           = opts_ctx;
    runner_ctx->pathtools_ctx      = pathtools_ctx;
    runner_ctx->plugin_ctx         = plugin_ctx;
    runner_ctx->default_configfile = default_configfile
     ?: DEFAULT_CONFIG_FILENAME;
    runner_ctx->default_directory  = default_directory
     ?: DEFAULT_DIRECTORY_NAME;

    ST_LOGGERCTX_CALL(logger_ctx, info, "runner_simple: Runner initialized");

    return runner_ctx;
}

static void st_runner_quit(st_runnerctx_t *runner_ctx) {
    ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, info,
     "runner_simple: Runner destroyed");
    free(runner_ctx);
}

static bool get_config_filename(st_runnerctx_t *runner_ctx,
 char filename[PATH_MAX]) {
    int ret;

    if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, add_option, 'c', "cfg",
     ST_OA_REQUIRED, "filename", "Config file")) {
        if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, get_str, "cfg", filename,
         PATH_MAX))
            return true;

        ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, warning,
         "runner_simple: Unable to get cmdline option for config filename. "
         "Using default config file \"%s\"", runner_ctx->default_configfile);
    } else {
        ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, warning,
         "runner_simple: Unable to set cmdline option for config filename. "
         "Using default config file \"%s\"", runner_ctx->default_configfile);
    }

    ret = snprintf(filename, PATH_MAX, "%s", runner_ctx->default_configfile);
    if (ret < 0 || ret == PATH_MAX) {
        ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, error,
         "ini_inih: Unable to copy default config filename");

        return false;
    }

    return true;
}

static bool get_directory_name(st_runnerctx_t *runner_ctx,
 char dirname[PATH_MAX], const st_ini_t *ini) {
    if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, add_option, 'p', "plugin-path",
     ST_OA_REQUIRED, "path", "Path where plugins stored")) {
        if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, get_str, "plugin-path",
         dirname, PATH_MAX))
            return true;
    }

    if (ini && !ST_INI_CALL(ini, fill_str, dirname, PATH_MAX, "steroids.runner",
     "plugin_path")) {
        int ret;

        ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, warning,
         "runner_simple: Unable to get plugin directory name. Using default "
         "directory \"%s\"", runner_ctx->default_directory);

        ret = snprintf(dirname, PATH_MAX, "%s", runner_ctx->default_directory);
        if (ret < 0 || ret == PATH_MAX) {
            ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, error,
             "runner_simple: Unable to copy default plugin directory name "
             "\"%s\"", runner_ctx->default_directory);

            return false;
        }
    }

    return !!ini;
}

static bool get_runnable_module_name(st_runnerctx_t *runner_ctx,
 char runnable[RUNNABLE_MODULE_NAME_SIZE], const st_ini_t *ini) {
    if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, add_option, 'r', "run",
     ST_OA_REQUIRED, "module_name",
     "Name of the mudule that must be launched")) {
        if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, get_str, "run", runnable,
         RUNNABLE_MODULE_NAME_SIZE))
            return true;
    }

    if (ini && !ST_INI_CALL(ini, fill_str, runnable, RUNNABLE_MODULE_NAME_SIZE,
     "steroids.runner", "run_module")) {
        ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, error,
         "runner_simple: Unable to get runnable module name");

        return false;
    }

    return !!ini;
}

static bool get_script_name(st_runnerctx_t *runner_ctx,
 char script_name[PATH_MAX], const st_ini_t *ini) {
    if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, add_option, 's', "script",
     ST_OA_REQUIRED, "filename", "Name of the script that must be launched")) {
        if (ST_OPTSCTX_CALL(runner_ctx->opts_ctx, get_str, "script",
         script_name, PATH_MAX))
            return true;
    }

    if (ini && !ST_INI_CALL(ini, fill_str, script_name, PATH_MAX,
     "steroids.runner", "script"))
        return false;

    return !!ini;
}

static bool load_plugins(st_runnerctx_t *runner_ctx,
 const char dirname[PATH_MAX]) {
    struct dirent *entry;
    DIR           *dir = opendir(dirname);

    if (!dir) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, error,
             "runner_simple: Unable to open directory \"%s\": %s", dirname,
             errbuf);

        return false;
    }

    entry = readdir(dir);
    while (entry) {
        if (entry->d_type == DT_REG) {
            char filename[PATH_MAX];

            if (ST_PATHTOOLSCTX_CALL(runner_ctx->pathtools_ctx, concat,
             filename, PATH_MAX, dirname, entry->d_name)) {
                if (!ST_PLUGINCTX_CALL(runner_ctx->plugin_ctx, load, filename,
                 true))
                    ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, error,
                     "runner_simple: Unable to load plugin \"%s\"", filename);
            }
        }

        entry = readdir(dir);
    }

    ST_MODSMGR_CALL(runner_ctx->modsmgr, process_deps);

    closedir(dir);

    return true;
}

static bool run_runnable(st_runnerctx_t *runner_ctx,
 const char *module_subsystem, const char *module_name,
 const char script_name[PATH_MAX]) {
    st_ctx_ctor_t     runnable_ctor;
    st_runnablectx_t *runnable_ctx;
    bool              result;

    runnable_ctor = ST_MODSMGR_CALL(runner_ctx->modsmgr, get_ctor,
     module_subsystem, module_name);
    if (!runnable_ctor) {
        ST_LOGGERCTX_CALL(runner_ctx->logger_ctx, error,
         "runner_simple: Unable to load ctor from module \"%s_%s\"",
         module_subsystem, module_name);

        return false;
    }

    runnable_ctx = runnable_ctor(
     (st_param_t[]){{"modsmgr", (uintptr_t)runner_ctx->modsmgr}, {0}});
    if (!runnable_ctx)
        return false;

    result = ST_RUNNABLECTX_CALL(runnable_ctx, run,
     (st_param_t[]){{"script", (uintptr_t)script_name}, {0}});
    ST_RUNNABLECTX_CALL(runnable_ctx, destroy);

    return result;
}

static bool st_runner_run(st_runnablectx_t *runner_ctx,
 __attribute__((unused)) const st_param_t params[]) {
    char      cfg_filename[PATH_MAX];
    st_ini_t *ini;
    char      dirname[PATH_MAX];
    char      runnable[RUNNABLE_MODULE_NAME_SIZE];
    char      script_name[PATH_MAX] = "";
    char     *runnable_subsystem = runnable;
    char     *runnable_name;

    if (!get_config_filename((st_runnerctx_t *)runner_ctx, cfg_filename))
        return false;

    ini = ST_INICTX_CALL(((st_runnerctx_t *)runner_ctx)->ini_ctx, load,
     cfg_filename);

    if (!get_directory_name((st_runnerctx_t *)runner_ctx, dirname, ini) ||
     !get_runnable_module_name((st_runnerctx_t *)runner_ctx, runnable, ini))
        goto fail;

    runnable_name = strchr(runnable, ':');
    if (!runnable_name) {
        ST_LOGGERCTX_CALL(((st_runnerctx_t *)runner_ctx)->logger_ctx, error,
         "runner_simple: Missing name of runnable module");

        goto fail;
    }

    *runnable_name++ = '\0';

    get_script_name((st_runnerctx_t *)runner_ctx, script_name, ini);

    if (!load_plugins((st_runnerctx_t *)runner_ctx, dirname))
        goto fail;

    return run_runnable((st_runnerctx_t *)runner_ctx, runnable_subsystem,
     runnable_name, script_name);

fail:
    if (ini)
        ST_INI_CALL(ini, destroy);

    return false;
}
