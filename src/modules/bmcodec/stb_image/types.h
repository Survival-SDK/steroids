#pragma once

#include "steroids/modules/bitmap.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t       *ctx;
    st_bitmap_import_t import;
} st_bmcodec_stb_image_bitmap_t;

typedef struct {
    st_modctx_t        *ctx;
    st_logger_debug_t   debug;
    st_logger_info_t    info;
    st_logger_error_t   error;
} st_bmcodec_stb_image_logger_t;

typedef struct {
    st_bmcodec_stb_image_bitmap_t bitmap;
    st_bmcodec_stb_image_logger_t logger;
} st_bmcodec_stb_image_t;
