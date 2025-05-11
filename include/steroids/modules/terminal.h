#pragma once

#include "steroids/module.h"

typedef st_modctx_t *(*st_terminal_init_t)(st_modctx_t *logger_ctx);
typedef void (*st_terminal_quit_t)(st_modctx_t *terminal_ctx);

typedef int (*st_terminal_get_rows_count_t)(st_modctx_t *terminal_ctx);
typedef int (*st_terminal_get_cols_count_t)(st_modctx_t *terminal_ctx);

typedef struct {
    st_terminal_init_t           terminal_init;
    st_terminal_quit_t           terminal_quit;
    st_terminal_get_rows_count_t terminal_get_rows_count;
    st_terminal_get_cols_count_t terminal_get_cols_count;
} st_terminal_funcs_t;
