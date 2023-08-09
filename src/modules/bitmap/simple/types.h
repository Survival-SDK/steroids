#ifndef ST_MODULES_BITMAP_SIMPLE_TYPES_H
#define ST_MODULES_BITMAP_SIMPLE_TYPES_H

#include "steroids/types/modules/logger.h"

#define CODECS_MAX 32

struct st_bitmap_s;

typedef struct st_bitmap_s *(*st_bmcodec_load_t)(st_modctx_t *codec_ctx,
 const char *filename);
typedef struct st_bitmap_s *(*st_bmcodec_memload_t)(st_modctx_t *codec_ctx,
 const void *data, size_t size);
typedef bool (*st_bmcodec_save_t)(st_modctx_t *codec_ctx,
 const struct st_bitmap_s *bitmap, const char *filename);
typedef const char *(*st_bmcodec_memsave_t)(st_modctx_t *codec_ctx, void *dst,
 size_t *size, const struct st_bitmap_s *bitmap);

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_bitmap_simple_logger_t;

typedef struct {
    st_modctx_t         *ctx;
    st_bmcodec_load_t    load;
    st_bmcodec_memload_t memload;
    st_bmcodec_save_t    save;
    st_bmcodec_memsave_t memsave;
} st_bitmap_simple_codec_t;

typedef struct {
    st_bitmap_simple_logger_t logger;
    st_bitmap_simple_codec_t  codecs[CODECS_MAX];
    size_t                    codecs_count;
} st_bitmap_simple_t;

typedef struct st_bitmap_s {
    st_bitmap_simple_t *module;
    unsigned            width;
    unsigned            height;
    int                 pixel_format;
    char                data[];
} st_bitmap_t;

#define ST_BITMAP_CODEC_FUNCS_DEFINED
#define ST_BITMAP_T_DEFINED

#endif /* ST_MODULES_BITMAP_SIMPLE_TYPES_H */
