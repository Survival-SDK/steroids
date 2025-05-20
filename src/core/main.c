#include "modules_manager.h"
#include "steroids/modctx.h"

int main(int argc, char **argv) {
    st_modsmgr_t   *modsmgr = st_modsmgr_init();

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


    ST_MODSMGR_CALL(modsmgr, destroy);

    return 0;
}
