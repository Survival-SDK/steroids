#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
    st_gfxctx_t    *gfxctx;
} st_glloaderctx_t;

#define ST_GLLOADERCTX_T_DEFINED

