#pragma once

#include "steroids/modules/ini.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/opts.h"
#include "steroids/modules/pathtools.h"
#include "steroids/modules/plugin.h"

typedef struct {
    st_modctx_t  *ctx;
    st_ini_load_t load;
} st_runner_simple_ini_t;

typedef struct {
    st_modctx_t        *ctx;
    st_logger_debug_t   debug;
    st_logger_info_t    info;
    st_logger_warning_t warning;
    st_logger_error_t   error;
} st_runner_simple_logger_t;

typedef struct {
    st_modctx_t         *ctx;
    st_opts_add_option_t add_option;
    st_opts_get_str_t    get_str;
} st_runner_simple_opts_t;

typedef struct {
    st_modctx_t          *ctx;
    st_pathtools_concat_t concat;
} st_runner_simple_pathtools_t;

typedef struct {
    st_modctx_t         *ctx;
    st_plugin_load_t load;
} st_runner_simple_plugin_t;

typedef struct {
    st_runner_simple_ini_t       ini;
    st_runner_simple_logger_t    logger;
    st_runner_simple_opts_t      opts;
    st_runner_simple_pathtools_t pathtools;
    st_runner_simple_plugin_t    plugin;
} st_runner_simple_t;
