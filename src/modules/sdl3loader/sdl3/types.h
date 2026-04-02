#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
} st_sdl3loaderctx_t;

#define ST_SDL3LOADERCTX_T_DEFINED

