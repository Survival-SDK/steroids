#pragma once

#include "steroids/types/modctx.h"
#include "steroids/types/modules/logger.h"

typedef struct {
    st_modctx_t;
    struct st_loggerctx_s *logger_ctx;
} st_pathtoolsctx_t;

#define ST_PATHTOOLSCTX_T_DEFINED
