#pragma once

#include <stdbool.h>
#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#include <lwrb.h>
#pragma GCC diagnostic pop

#include "steroids/types/modules/logger.h"
#include "steroids/types/object.h"

ST_MODCTX (
    struct st_loggerctx_s *logger_ctx;
) st_rbufctx_t;

ST_CLASS (
    lwrb_t   handle;
    uint8_t  data[];
) st_rbuf_t;

#define ST_RBUFCTX_T_DEFINED
#define ST_RBUF_T_DEFINED
