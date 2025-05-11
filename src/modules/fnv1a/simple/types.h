#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    struct st_loggerctx_s *logger_ctx;
} st_fnv1actx_t;

#define ST_FNV1ACTX_T_DEFINED
