#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/angle.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_anglectx_t  *angle_ctx;
} st_vec2ctx_t;

#define ST_VEC2CTX_T_DEFINED
