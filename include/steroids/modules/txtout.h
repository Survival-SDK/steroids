#pragma once

#include <sys/types.h>

#include "steroids/object.h"
#include "steroids/modctx.h"
#include "steroids/modules/font.h"

#ifndef ST_TXTOUTCTX_T_DEFINED
    typedef st_modctx_t st_txtoutctx_t;
#endif

typedef struct {
    const st_sprite_t *sprite;
    float              x;
    float              y;
    float              hscale;
    float              vscale;
} st_txtoutentry_t;

typedef unsigned (*st_txtout_get_text_width_t)(const st_txtoutctx_t *txtout_ctx, 
 const st_font_t *font, const char *text);
typedef const char *(*st_txtout_get_subtext_after_wrap_t)(
 const st_txtoutctx_t *txtout_ctx, size_t *codepoints_before_wrap,
 const st_font_t *font, const char *text, unsigned max_width);
typedef ssize_t (*st_txtout_get_output_data_t)(const st_txtoutctx_t *txtout_ctx, 
 st_txtoutentry_t *dst, const st_font_t *font, const char *text, 
 size_t codepoints, float x, float y, float hscale, float vscale, float pivot_x, 
 float pivot_y);

typedef struct {
    st_modctx_funcs_t;
    st_txtout_get_text_width_t         get_text_width;
    st_txtout_get_subtext_after_wrap_t get_subtext_after_wrap;
    st_txtout_get_output_data_t        get_output_data;
} st_txtoutctx_funcs_t;

#define ST_TXTOUTCTX_CALL(ctx, func, ...) \
    ((st_txtoutctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
