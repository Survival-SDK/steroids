#pragma once

#define ST_RUNNABLECTX_CALL(object, func, ...) \
    ((const st_runnablectx_funcs_t *)object->funcs)->func(object, ## __VA_ARGS__)

typedef st_modctx_t st_runnablectx_t;

typedef bool (*st_runnablectx_run_t)(st_runnablectx_t *runnablectx,
 const st_param_t params[]);

typedef struct {
    st_modctx_funcs_t;
    st_runnablectx_run_t run;
} st_runnablectx_funcs_t;
