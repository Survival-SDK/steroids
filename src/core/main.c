#include "modules_manager.h"
#include "steroids/modctx.h"
#include "steroids/modules/runner.h"

#define DEFAULT_CONFIG_FILENAME   "steroids.ini"
#define DEFAULT_DIRECTORY_NAME    "."

int main(int argc, char **argv) {
    st_modsmgr_t *modsmgr = st_modsmgr_init();
    st_ctx_ctor_t runner_ctor;

    ST_MODSMGR_CALL(modsmgr, create_singleton, "logger",    NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "htable",    NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "fnv1a",     NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "ini",       NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "opts",      NULL, (
    st_param_t[]){
        {"modsmgr", (uintptr_t)modsmgr},
        {"argc", (uintptr_t)argc},
        {"argv", (uintptr_t)argv},
    {0}, });
    ST_MODSMGR_CALL(modsmgr, create_singleton, "pathtools", NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "fs",        NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "so",        NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "spcpaths",  NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "zip",       NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});
    ST_MODSMGR_CALL(modsmgr, create_singleton, "plugin",    NULL,
     (st_param_t[]){{"modsmgr", (uintptr_t)modsmgr}, {0}});

    runner_ctor = ST_MODSMGR_CALL(modsmgr, get_ctor, "runner", NULL);
    if (runner_ctor) {
        st_runnerctx_t *runner_ctx;

        runner_ctx = runner_ctor((st_param_t[]){
            {"modsmgr",            (uintptr_t)modsmgr},
            {"default-configfile", (uintptr_t)DEFAULT_CONFIG_FILENAME},
            {"default-directory",  (uintptr_t)DEFAULT_DIRECTORY_NAME},
            {0}
        });

        if (runner_ctx)
            ST_RUNNERCTX_CALL(runner_ctx, run, NULL);
    }

    ST_MODSMGR_CALL(modsmgr, destroy);

    return 0;
}
