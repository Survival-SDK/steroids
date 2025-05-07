#pragma once

#include "steroids/module.h"
#include "steroids/types/modules/fs.h"

static st_fsctx_t *st_fs_init(struct st_loggerctx_s *logger_ctx,
 st_pathtoolsctx_t *pathtools_ctx);
