#pragma once

#include "config.h" // IWYU pragma: keep
#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/luajit.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_luajitctx_t *luajit_ctx;
} st_luajitbindctx_t;

#define ST_LUAJITBINDCTX_T_DEFINED