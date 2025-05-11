#pragma once

#include "steroids/module.h"
#include "steroids/modules/gfxctx.h"

typedef st_modctx_t *(*st_gldebug_init_t)(st_modctx_t *logger_ctx,
 st_gfxctx_t *gfxctx);
typedef void (*st_gldebug_quit_t)(st_modctx_t *gldebug_ctx);

typedef void (*st_gldebug_label_buffer_t)(const st_modctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_shader_t)(const st_modctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_shdprog_t)(const st_modctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_vao_t)(const st_modctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_pipeline_t)(const st_modctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_texture_t)(const st_modctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_framebuffer_t)(const st_modctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_unlabel_buffer_t)(const st_modctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_shader_t)(const st_modctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_shdprog_t)(const st_modctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_vao_t)(const st_modctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_pipeline_t)(const st_modctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_texture_t)(const st_modctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_framebuffer_t)(const st_modctx_t *gldebug_ctx,
 unsigned id);
typedef const char *(*st_gldebug_get_error_msg_t)(
 const st_modctx_t *gldebug_ctx, unsigned err);

typedef struct {
    st_gldebug_init_t                gldebug_init;
    st_gldebug_quit_t                gldebug_quit;
    st_gldebug_label_buffer_t        gldebug_label_buffer;
    st_gldebug_label_shader_t        gldebug_label_shader;
    st_gldebug_label_shdprog_t       gldebug_label_shdprog;
    st_gldebug_label_vao_t           gldebug_label_vao;
    st_gldebug_label_pipeline_t      gldebug_label_pipeline;
    st_gldebug_label_texture_t       gldebug_label_texture;
    st_gldebug_label_framebuffer_t   gldebug_label_framebuffer;
    st_gldebug_unlabel_buffer_t      gldebug_unlabel_buffer;
    st_gldebug_unlabel_shader_t      gldebug_unlabel_shader;
    st_gldebug_unlabel_shdprog_t     gldebug_unlabel_shdprog;
    st_gldebug_unlabel_vao_t         gldebug_unlabel_vao;
    st_gldebug_unlabel_pipeline_t    gldebug_unlabel_pipeline;
    st_gldebug_unlabel_texture_t     gldebug_unlabel_texture;
    st_gldebug_unlabel_framebuffer_t gldebug_unlabel_framebuffer;
    st_gldebug_get_error_msg_t       gldebug_get_error_msg;
} st_gldebug_funcs_t;
