#pragma once

#include <stdbool.h>

#include "steroids/module.h"
#include "steroids/types/modules/logger.h"

static st_loggerctx_t *st_logger_init(struct st_eventsctx_s *events_ctx);
