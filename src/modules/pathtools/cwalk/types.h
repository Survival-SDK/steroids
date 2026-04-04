#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_pathtoolsctx_t;

#define ST_PATHTOOLSCTX_T_DEFINED
