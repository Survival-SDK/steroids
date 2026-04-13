#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/sprite.h"
#include "steroids/modules/texture.h"
#include "steroids/object.h"

#define ST_FONT_LOADERS_MAX 8

typedef struct {
    st_modctx_t;
    st_modsmgr_t    *modsmgr;
    st_htablectx_t  *htable_ctx;
    st_loggerctx_t  *logger_ctx;
    st_spritectx_t  *sprite_ctx;
    st_texturectx_t *texture_ctx;
    st_modctx_t     *font_loaders[ST_FONT_LOADERS_MAX];
    size_t           font_loaders_count;
} st_fontctx_t;

typedef st_texture_t st_fontpage_t;

typedef struct {
    st_sprite_t *handle;
    int          xoffset;
    int          yoffset;
    int          xadvance;
} st_fontchar_t;

typedef struct {
    st_object_t;
    unsigned       line_height;
    unsigned       base;
    unsigned       texture_width;
    unsigned       texture_height;
    unsigned       pages_count;
    st_htable_t   *chars;
    st_fontpage_t *pages[];
} st_font_t;

#define ST_FONTCTX_T_DEFINED
#define ST_FONT_T_DEFINED
