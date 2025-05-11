#pragma once

#include "steroids/types/modules/zip.h"

static st_zipctx_t *st_zip_init(st_fsctx_t *fs_ctx,
 struct st_loggerctx_s *logger_ctx, st_pathtoolsctx_t *pathtools_ctx);
