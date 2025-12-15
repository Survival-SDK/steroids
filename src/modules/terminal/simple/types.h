#pragma once

#include "steroids/modctx.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/logger.h"

typedef struct st_terminalctx_s {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
} st_terminalctx_t;

#define ST_TERMINALCTX_T_DEFINED
