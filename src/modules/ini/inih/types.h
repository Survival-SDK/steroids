#pragma once

#include "steroids/types/modules/fnv1a.h"
#include "steroids/types/modules/htable.h"
#include "steroids/types/modules/logger.h"

typedef struct {
    st_fnv1a_get_u32hashstr_func_t get_u32hashstr_func;
} st_ini_inih_fnv1a_t;

typedef struct {
    st_modctx_t       *ctx;
    st_htable_init_t   init;
    st_htable_quit_t   quit;
    st_htable_create_t create;
} st_ini_inih_htable_t;

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_ini_inih_logger_t;

typedef struct {
    st_ini_inih_fnv1a_t  fnv1a;
    st_ini_inih_htable_t htable;
    st_ini_inih_logger_t logger;
} st_ini_inih_t;

typedef struct {
    st_ini_inih_t *module;
    st_htable_t   *data;
} st_inisection_t;

ST_CLASS (
    st_htable_t *sections;
) st_ini_t;

#define ST_INI_T_DEFINED
