#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_dynarrctx_t;

typedef struct {
    st_object_t;
    struct scv_vector *handle;
} st_dynarr_t;

#define ST_DYNARRCTX_T_DEFINED
#define ST_DYNARR_T_DEFINED
