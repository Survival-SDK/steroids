#pragma once

#include <stddef.h>
#include <stdint.h>

#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/rbuf.h"
#include "steroids/types/object.h"

#define EVENT_TYPE_NAME_SIZE 32
#define EVENT_TYPES_MAX      32
#define SUBSCRIBERS_MAX      8

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_events_simple_logger_t;

typedef struct {
    st_modctx_t     *ctx;
    st_rbuf_init_t   init;
    st_rbuf_quit_t   quit;
    st_rbuf_create_t create;
} st_events_simple_rbuf_t;

ST_CLASS(
    st_rbuf_t *handle;
    bool       active;
) st_evq_t;

typedef struct {
    char      name[EVENT_TYPE_NAME_SIZE];
    size_t    data_size;
    st_evq_t *subscribers[SUBSCRIBERS_MAX];
    size_t    subscribers_count;
} st_evtype_t;

typedef struct {
    st_events_simple_logger_t logger;
    st_events_simple_rbuf_t   rbuf;
    st_evtype_t               types[EVENT_TYPES_MAX];
    size_t                    types_count;
} st_events_simple_t;

#define ST_EVQ_T_DEFINED
