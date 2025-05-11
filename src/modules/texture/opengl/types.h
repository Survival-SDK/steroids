#pragma once

#include <GL/gl.h>

#include "steroids/modules/bitmap.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/gldebug.h"
#include "steroids/modules/glloader.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t           *ctx;
    st_bitmap_load_t       load;
    st_bitmap_memload_t    memload;
    st_bitmap_destroy_t    destroy;
    st_bitmap_get_data_t   get_data;
    st_bitmap_get_width_t  get_width;
    st_bitmap_get_height_t get_height;
} st_texture_opengl_bitmap_t;

typedef struct {
    st_modctx_t                 *ctx;
    st_gldebug_init_t            init;
    st_gldebug_quit_t            quit;
    st_gldebug_label_texture_t   label_texture;
    st_gldebug_unlabel_texture_t unlabel_texture;
    st_gldebug_get_error_msg_t   get_error_msg;
} st_texture_opengl_gldebug_t;

typedef struct {
    st_modctx_t        *ctx;
    st_logger_debug_t   debug;
    st_logger_info_t    info;
    st_logger_warning_t warning;
    st_logger_error_t   error;
} st_texture_opengl_logger_t;

typedef struct {
    st_gfxctx_t *handle;
    st_gapi_t    api;
} st_texture_opengl_gfxctx_t;

typedef struct {
    st_texture_opengl_bitmap_t  bitmap;
    st_texture_opengl_gldebug_t gldebug;
    st_texture_opengl_logger_t  logger;
    st_texture_opengl_gfxctx_t  gfxctx;
} st_texture_opengl_t;

ST_CLASS (
    GLuint   id;
    unsigned width;
    unsigned height;
) st_texture_t;

#define ST_TEXTURE_T_DEFINED
