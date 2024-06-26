#pragma once

#include "config.h" // IWYU pragma: keep
#include "steroids/pathtools.h"
#include "steroids/types/modules/logger.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_pathtools_cwalk_logger_t;

typedef struct {
    st_pathtools_cwalk_logger_t logger;
} st_pathtools_cwalk_t;

static st_pathtools_funcs_t st_pathtools_cwalk_funcs = {
    .pathtools_init           = st_pathtools_init,
    .pathtools_quit           = st_pathtools_quit,
    .pathtools_resolve        = st_pathtools_resolve,
    .pathtools_get_parent_dir = st_pathtools_get_parent_dir,
    .pathtools_concat         = st_pathtools_concat,
};

static st_modfuncentry_t st_module_pathtools_cwalk_funcs[] = {
    {"init"          , st_pathtools_init},
    {"quit"          , st_pathtools_quit},
    {"resolve"       , st_pathtools_resolve},
    {"get_parent_dir", st_pathtools_get_parent_dir},
    {"concat"        , st_pathtools_concat},
    {NULL, NULL},
};
