#pragma once

#include "steroids/module.h"

typedef st_modctx_t *(*st_runner_init_t)(st_modctx_t *ini_ctx,
 st_modctx_t *logger_ctx, st_modctx_t *opts_ctx, st_modctx_t *pathtools_ctx,
 st_modctx_t *plugin_ctx);
typedef void (*st_runner_quit_t)(st_modctx_t *runner_ctx);

typedef void (*st_runner_run_t)(st_modctx_t *runner_ctx, const void *params);

typedef struct {
    st_runner_init_t runner_init;
    st_runner_quit_t runner_quit;
    st_runner_run_t  runner_run;
} st_runner_funcs_t;
