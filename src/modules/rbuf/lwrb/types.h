#pragma once

#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#include <lwrb.h>
#pragma GCC diagnostic pop

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_rbufctx_t;

typedef struct {
    st_object_t;
    lwrb_t  handle;
    uint8_t data[];
} st_rbuf_t;

#define ST_RBUFCTX_T_DEFINED
#define ST_RBUF_T_DEFINED
