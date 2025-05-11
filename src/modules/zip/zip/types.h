#pragma once

#include <zip/zip.h>

#include "steroids/modctx.h"
#include "steroids/modules/fs.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/pathtools.h"
#include "steroids/object.h"

typedef enum {
    ST_ZT_FILE,
    ST_ZT_MEM,
} st_ziptype_t;

typedef struct {
    st_modctx_t;
    st_fsctx_t            *fs_ctx;
    struct st_loggerctx_s *logger_ctx;
    st_pathtoolsctx_t     *pathtools_ctx;
} st_zipctx_t;

typedef struct {
    st_object_t;
    struct zip_t *handle;
    st_ziptype_t  type;
} st_zip_t;

#define ST_ZIPCTX_T_DEFINED
#define ST_ZIP_T_DEFINED
