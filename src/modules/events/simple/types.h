#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/rbuf.h"
#include "steroids/object.h"

#define EVENT_TYPE_NAME_SIZE 32
#define EVENT_TYPES_MAX      32
#define SUBSCRIBERS_MAX      8

typedef struct {
    st_object_t;
    st_rbuf_t *handle;
    bool       active;
} st_evq_t;

typedef struct {
    char      name[EVENT_TYPE_NAME_SIZE];
    size_t    data_size;
    st_evq_t *subscribers[SUBSCRIBERS_MAX];
    size_t    subscribers_count;
} st_evtype_t;

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_rbufctx_t   *rbuf_ctx;
    st_evtype_t     types[EVENT_TYPES_MAX];
    size_t          types_count;
} st_eventsctx_t;

#define ST_EVENTSCTX_T_DEFINED
#define ST_EVQ_T_DEFINED
