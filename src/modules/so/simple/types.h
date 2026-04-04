#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

#include "dlist.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_dlist_t     *opened_handles;
} st_soctx_t;

typedef struct {
    st_object_t;
    void *handle;
} st_so_t;

#define ST_SOCTX_T_DEFINED
#define ST_SO_T_DEFINED
