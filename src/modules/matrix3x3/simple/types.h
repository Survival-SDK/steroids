#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/angle.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_anglectx_t  *angle_ctx;
} st_matrix3x3ctx_t;

#define ST_MATRIX3X3CTX_T_DEFINED
