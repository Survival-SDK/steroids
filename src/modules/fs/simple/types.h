#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/pathtools.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t    *logger_ctx;
    st_pathtoolsctx_t *pathtools_ctx;
 } st_fsctx_t;

#define ST_FSCTX_T_DEFINED
