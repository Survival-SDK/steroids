#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/atlas.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/xml.h"

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
    st_xmlctx_t    *xml_ctx;
    st_atlasctx_t  *atlas_ctx;
} st_atlloaderctx_t;

#define ST_ATLLOADERCTX_T_DEFINED
