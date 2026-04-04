#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_spcpathsctx_t;

#define ST_SPCPATHSCTX_T_DEFINED
