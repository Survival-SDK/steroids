#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/font.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/xml.h"

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
    st_xmlctx_t    *xml_ctx;
    st_fontctx_t   *font_ctx;
} st_fontloaderctx_t;

#define ST_FONTLOADERCTX_T_DEFINED
