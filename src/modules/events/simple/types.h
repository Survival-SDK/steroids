#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/types/modctx.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/rbuf.h"
#include "steroids/types/object.h"

#define EVENT_TYPE_NAME_SIZE 32
#define EVENT_TYPES_MAX      32
#define SUBSCRIBERS_MAX      8

typedef struct st_evq_s {
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

typedef struct st_eventsctx_s {
    st_modctx_t;
    struct st_loggerctx_s *logger_ctx;
    st_rbufctx_t          *rbuf_ctx;
    st_evtype_t            types[EVENT_TYPES_MAX];
    size_t                 types_count;
} st_eventsctx_t;
