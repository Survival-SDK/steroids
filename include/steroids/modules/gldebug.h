#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/gfxctx.h"

#ifndef ST_GLDEBUGCTX_T_DEFINED
    typedef st_modctx_t st_gldebugctx_t;
#endif

typedef void (*st_gldebug_label_buffer_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_shader_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_shdprog_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_vao_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_pipeline_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_texture_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_label_framebuffer_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
typedef void (*st_gldebug_unlabel_buffer_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_shader_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_shdprog_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_vao_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_pipeline_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_texture_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
typedef void (*st_gldebug_unlabel_framebuffer_t)(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
typedef const char *(*st_gldebug_get_error_msg_t)(
 const st_gldebugctx_t *gldebug_ctx, unsigned err);

typedef struct {
    st_modctx_funcs_t;
    st_gldebug_label_buffer_t        label_buffer;
    st_gldebug_label_shader_t        label_shader;
    st_gldebug_label_shdprog_t       label_shdprog;
    st_gldebug_label_vao_t           label_vao;
    st_gldebug_label_pipeline_t      label_pipeline;
    st_gldebug_label_texture_t       label_texture;
    st_gldebug_label_framebuffer_t   label_framebuffer;
    st_gldebug_unlabel_buffer_t      unlabel_buffer;
    st_gldebug_unlabel_shader_t      unlabel_shader;
    st_gldebug_unlabel_shdprog_t     unlabel_shdprog;
    st_gldebug_unlabel_vao_t         unlabel_vao;
    st_gldebug_unlabel_pipeline_t    unlabel_pipeline;
    st_gldebug_unlabel_texture_t     unlabel_texture;
    st_gldebug_unlabel_framebuffer_t unlabel_framebuffer;
    st_gldebug_get_error_msg_t       get_error_msg;
} st_gldebugctx_funcs_t;

#define ST_GLDEBUGCTX_CALL(ctx, func, ...) \
    ((st_gldebugctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
