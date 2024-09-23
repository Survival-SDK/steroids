#pragma once

#include <zip/zip.h>

#include "steroids/types/modules/fs.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/pathtools.h"
#include "steroids/types/modctx.h"
#include "steroids/types/object.h"

typedef enum {
    ST_ZT_FILE,
    ST_ZT_MEM,
} st_ziptype_t;

ST_MODCTX (
    st_fsctx_t            *fs_ctx;
    struct st_loggerctx_s *logger_ctx;
    st_pathtoolsctx_t     *pathtools_ctx;
) st_zipctx_t;

ST_CLASS (
    struct zip_t *handle;
    st_ziptype_t  type;
) st_zip_t;

#define ST_ZIPCTX_T_DEFINED
#define ST_ZIP_T_DEFINED
