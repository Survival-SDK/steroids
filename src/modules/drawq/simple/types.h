#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/dynarr.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_dynarrctx_t *dynarr_ctx;
    st_loggerctx_t *logger_ctx;
} st_drawqctx_t;

typedef struct {
    st_object_t;
    st_dynarr_t *entries;
} st_drawq_t;

#define ST_DRAWQCTX_T_DEFINED
#define ST_DRAWQ_T_DEFINED
