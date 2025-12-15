#pragma once

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_TERMINALCTX_T_DEFINED
    typedef st_modctx_t st_terminalctx_t;
#endif

typedef int (*st_terminal_get_rows_count_t)(
 const st_terminalctx_t *terminal_ctx);
typedef int (*st_terminal_get_cols_count_t)(
 const st_terminalctx_t *terminal_ctx);

typedef struct {
    st_modctx_funcs_t;
    st_terminal_get_rows_count_t get_rows_count;
    st_terminal_get_cols_count_t get_cols_count;
} st_terminalctx_funcs_t;

#define ST_TERMINALCTX_CALL(ctx, func, ...) \
    ((st_terminalctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
