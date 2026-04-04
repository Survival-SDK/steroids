#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/fnv1a.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_fnv1actx_t  *fnv1a_ctx;
    st_htablectx_t *htable_ctx;
    st_loggerctx_t *logger_ctx;
} st_inictx_t;

typedef st_htable_t st_inisection_t;

typedef struct {
    st_object_t;
    st_inisection_t *sections;
} st_ini_t;

#define ST_INICTX_T_DEFINED
#define ST_INI_T_DEFINED
