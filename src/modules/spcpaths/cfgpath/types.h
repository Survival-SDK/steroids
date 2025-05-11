#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    struct st_loggerctx_s *logger_ctx;
} st_spcpathsctx_t;

#define ST_SPCPATHSCTX_T_DEFINED
