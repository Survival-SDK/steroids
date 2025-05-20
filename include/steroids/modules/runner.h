#pragma once

#include "steroids/runnablectx.h"

#ifndef ST_RUNNERCTX_T_DEFINED
    typedef st_runnablectx_t st_runnerctx_t;
#endif

typedef bool (*st_runner_run_t)(st_runnablectx_t *runner_ctx,
 const st_param_t params[]);

typedef st_runnablectx_funcs_t st_runnerctx_funcs_t;

#define ST_RUNNERCTX_CALL(object, func, ...) \
    ((st_runnerctx_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
