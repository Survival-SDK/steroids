#pragma once

#include <GL/gl.h>

#include "steroids/modctx.h"
#include "steroids/modules/bitmap.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/gldebug.h"
#include "steroids/modules/glloader.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_bitmapctx_t   *bitmap_ctx;
    st_glloaderctx_t *glloader_ctx;
    st_gldebugctx_t  *gldebug_ctx;
    st_loggerctx_t   *logger_ctx;
    st_gfxctx_t      *gfxctx;
    st_gapi_t         gfxctx_api;
} st_texturectx_t;

typedef struct {
    st_object_t;
    GLuint   id;
    unsigned width;
    unsigned height;
} st_texture_t;

#define ST_TEXTURECTX_T_DEFINED
#define ST_TEXTURE_T_DEFINED
