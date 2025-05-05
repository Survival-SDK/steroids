#pragma once

#include "steroids/types/modctx.h"
#include "steroids/types/modules/fnv1a.h"
#include "steroids/types/modules/htable.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/object.h"

typedef struct {
    st_modctx_t;
    st_fnv1actx_t         *fnv1a_ctx;
    st_htablectx_t        *htable_ctx;
    struct st_loggerctx_s *logger_ctx;
} st_inictx_t;

typedef st_htable_t st_inisection_t;

typedef struct {
    st_object_t;
    st_htable_t *sections;
} st_ini_t;

#define ST_INICTX_T_DEFINED
#define ST_INI_T_DEFINED
