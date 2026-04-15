#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_utf8ctx_t;

#define ST_UTF8CTX_T_DEFINED
