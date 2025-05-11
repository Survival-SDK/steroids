#pragma once

#include "config.h" // IWYU pragma: keep
#include "steroids/terminal.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_terminal_simple_logger_t;

typedef struct {
    st_terminal_simple_logger_t logger;
} st_terminal_simple_t;

static st_terminal_funcs_t st_terminal_simple_funcs = {
    .terminal_init           = st_terminal_init,
    .terminal_quit           = st_terminal_quit,
    .terminal_get_rows_count = st_terminal_get_rows_count,
    .terminal_get_cols_count = st_terminal_get_cols_count,
};

static st_modfuncentry_t st_module_terminal_simple_funcs[] = {
    {"init"          , st_terminal_init},
    {"quit"          , st_terminal_quit},
    {"get_rows_count", st_terminal_get_rows_count},
    {"get_cols_count", st_terminal_get_cols_count},
    {NULL, NULL},
};
