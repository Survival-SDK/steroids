#pragma once

#include "steroids/module.h"
#include "steroids/types/modules/plugin.h"

static st_pluginctx_t *st_plugin_init(st_modsmgr_t *modsmgr, st_fsctx_t *fs_ctx,
 struct st_loggerctx_s *logger_ctx, st_pathtoolsctx_t *pathtools_ctx,
 st_soctx_t *so_ctx, st_spcpathsctx_t *spcpaths_ctx, st_zipctx_t *zip_ctx);
