#pragma once

#include "steroids/types/modules/fs.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/pathtools.h"
#include "steroids/types/modules/so.h"
#include "steroids/types/modules/spcpaths.h"
#include "steroids/types/modules/zip.h"
#include "steroids/types/modctx.h"

ST_MODCTX (
    st_fsctx_t            *fs_ctx;
    struct st_loggerctx_s *logger_ctx;
    st_pathtoolsctx_t     *pathtools_ctx;
    st_soctx_t            *so_ctx;
    st_spcpathsctx_t      *spcpaths_ctx;
    st_zipctx_t           *zip_ctx;
) st_pluginctx_t;

#define ST_PLUGINCTX_T_DEFINED
