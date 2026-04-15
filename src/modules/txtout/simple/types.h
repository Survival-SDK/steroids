#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/font.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/utf8.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_fontctx_t   *font_ctx;
    st_utf8ctx_t   *utf8_ctx;
} st_txtoutctx_t;

#define ST_TXTOUTCTX_T_DEFINED
