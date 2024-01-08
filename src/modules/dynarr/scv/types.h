#pragma once

#include <scv.h>

#include "steroids/types/modules/logger.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_dynarr_scv_logger_t;

typedef struct {
    st_dynarr_scv_logger_t logger;
} st_dynarr_scv_t;

typedef struct scv_vector st_dynarr_t;

#define ST_DYNARR_T_DEFINED