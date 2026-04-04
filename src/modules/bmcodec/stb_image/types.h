#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/bitmap.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
    st_bitmapctx_t *bitmap_ctx;
} st_bmcodecctx_t;

#define ST_BMCODECCTX_T_DEFINED
