#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/font.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/sprite.h"

#ifndef ST_RENDERCTX_T_DEFINED
    typedef st_modctx_t st_renderctx_t;
#endif

typedef void (*st_render_put_sprite_t)(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_rdangled_t)(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_dgangled_t)(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_rhsheared_t)(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_dhsheared_t)(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_rvsheared_t)(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_dvsheared_t)(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
typedef void (*st_render_put_text_t)(const st_renderctx_t *render_ctx,
 const st_font_t *font, const char *text, size_t codepoints, float x, float y, 
 float z, float hscale, float vscale, float pivot_x, float pivot_y);
typedef void (*st_render_process_t)(st_renderctx_t *render_ctx);

typedef struct {
    st_modctx_funcs_t;
    st_render_put_sprite_t           put_sprite;
    st_render_put_sprite_rdangled_t  put_sprite_rdangled;
    st_render_put_sprite_dgangled_t  put_sprite_dgangled;
    st_render_put_sprite_rhsheared_t put_sprite_rhsheared;
    st_render_put_sprite_dhsheared_t put_sprite_dhsheared;
    st_render_put_sprite_rvsheared_t put_sprite_rvsheared;
    st_render_put_sprite_dvsheared_t put_sprite_dvsheared;
    st_render_put_text_t             put_text;
    st_render_process_t              process;
} st_renderctx_funcs_t;

#define ST_RENDERCTX_CALL(ctx, func, ...) \
    ((st_renderctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
