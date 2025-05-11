#pragma once

#include "steroids/module.h"
#include "steroids/types/modctx.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/pathtools.h"
#include "steroids/types/object.h"

#ifndef ST_FSCTX_T_DEFINED
    typedef struct st_fsctx_s st_fsctx_t;
#endif

typedef enum {
    ST_FT_UNKNOWN,
    ST_FT_REG,
    ST_FT_DIR,
    ST_FT_CHR,
    ST_FT_BLK,
    ST_FT_FIFO,
    ST_FT_LINK,
    ST_FT_SOCK,
} st_filetype_t;

typedef st_fsctx_t *(*st_fs_init_t)(struct st_loggerctx_s *logger_ctx,
 st_pathtoolsctx_t *pathtools_ctx);
typedef st_filetype_t (*st_fs_get_file_type_t)(st_fsctx_t *fs_ctx,
 const char *filename);
typedef bool (*st_fs_mkdir_t)(st_fsctx_t *fs_ctx, const char *dirname);

typedef struct {
    st_modctx_funcs_t;
    st_fs_get_file_type_t get_file_type;
    st_fs_mkdir_t         mkdir;
} st_fsctx_funcs_t;

#define ST_FSCTX_CALL(object, func, ...) \
    ((st_fsctx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
