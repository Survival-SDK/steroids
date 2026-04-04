#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef void (*st_freefunc_t)(void *ptr);

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_htablectx_t;

typedef struct {
    st_object_t;
    struct hash_table *handle;
    st_freefunc_t      keydelfunc;
    st_freefunc_t      valdelfunc;
} st_htable_t;

typedef st_object_t st_htiter_t;

#define ST_HTABLECTX_T_DEFINED
#define ST_HTABLE_T_DEFINED
#define ST_HTITER_T_DEFINED
#define ST_FREEFUNC_T_DEFINED
