#pragma once

#include "steroids/module.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/sprite.h"

typedef st_modctx_t *(*st_render_init_t)(st_modctx_t *angle_ctx,
 st_modctx_t *drawq_ctx, st_modctx_t *dynarr_ctx, st_modctx_t *logger_ctx,
 st_modctx_t *matrix3x3_ctx, st_modctx_t *sprite_ctx, st_modctx_t *texture_ctx,
 st_modctx_t *vec2_ctx, st_gfxctx_t *gfxctx);
typedef void (*st_render_quit_t)(st_modctx_t *render_ctx);

typedef void (*st_render_put_sprite_t)(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_rdangled_t)(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_dgangled_t)(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_rhsheared_t)(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_dhsheared_t)(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_rvsheared_t)(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
typedef void (*st_render_put_sprite_dvsheared_t)(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
typedef void (*st_render_process_t)(st_modctx_t *render_ctx);

typedef struct {
    st_render_init_t                 render_init;
    st_render_quit_t                 render_quit;
    st_render_put_sprite_t           render_put_sprite;
    st_render_put_sprite_rdangled_t  render_put_sprite_rdangled;
    st_render_put_sprite_dgangled_t  render_put_sprite_dgangled;
    st_render_put_sprite_rhsheared_t render_put_sprite_rhsheared;
    st_render_put_sprite_dhsheared_t render_put_sprite_dhsheared;
    st_render_put_sprite_rvsheared_t render_put_sprite_rvsheared;
    st_render_put_sprite_dvsheared_t render_put_sprite_dvsheared;
    st_render_process_t              render_process;
} st_render_funcs_t;
