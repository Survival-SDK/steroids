#pragma once

#include "steroids/modules/angle.h"
#include "steroids/modules/drawq.h"
#include "steroids/modules/dynarr.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/gldebug.h"
#include "steroids/modules/glloader.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/matrix3x3.h"
#include "steroids/modules/sprite.h"
#include "steroids/modules/texture.h"
#include "steroids/modules/vec2.h"
#include "steroids/modules/window.h"

typedef struct {
    st_modctx_t    *ctx;
    st_angle_dtor_t dtor;
} st_render_opengl_angle_t;

typedef struct {
    st_modctx_t      *ctx;
    st_drawq_create_t create;
    st_drawq_t       *handle;
} st_render_opengl_drawq_t;

typedef struct {
    st_modctx_t                   *ctx;
    st_dynarr_create_t             create;
    st_dynarr_destroy_t            destroy;
    st_dynarr_append_t             append;
    st_dynarr_clear_t              clear;
    st_dynarr_get_t                get;
    st_dynarr_get_all_t            get_all;
    st_dynarr_get_elements_count_t get_elements_count;
} st_render_opengl_dynarr_t;

typedef struct {
    st_gapi_t    gapi;
    st_gfxctx_t *handle;
} st_render_opengl_gfxctx_t;

typedef struct {
    st_modctx_t                 *ctx;
    st_gldebug_init_t            init;
    st_gldebug_quit_t            quit;
    st_gldebug_label_buffer_t    label_buffer;
    st_gldebug_label_shader_t    label_shader;
    st_gldebug_label_shdprog_t   label_shdprog;
    st_gldebug_label_vao_t       label_vao;
    st_gldebug_unlabel_buffer_t  unlabel_buffer;
    st_gldebug_unlabel_shader_t  unlabel_shader;
    st_gldebug_unlabel_shdprog_t unlabel_shdprog;
    st_gldebug_unlabel_vao_t     unlabel_vao;
    st_gldebug_get_error_msg_t   get_error_msg;
} st_render_opengl_gldebug_t;

typedef struct {
    st_modctx_t        *ctx;
    st_logger_debug_t   debug;
    st_logger_info_t    info;
    st_logger_warning_t warning;
    st_logger_error_t   error;
} st_render_opengl_logger_t;

typedef struct {
    st_modctx_t            *ctx;
    st_matrix3x3_identity_t identity;
} st_render_opengl_matrix3x3_t;

typedef struct {
    st_sprite_get_texture_t get_texture;
    st_sprite_get_width_t   get_width;
    st_sprite_get_height_t  get_height;
    st_sprite_export_uv_t   export_uv;
} st_render_opengl_sprite_t;

typedef struct {
    st_texture_bind_t bind;
} st_render_opengl_texture_t;

typedef struct {
    st_modctx_t              *ctx;
    st_vec2_apply_matrix3x3_t apply_matrix3x3;
} st_render_opengl_vec2_t;

typedef struct {
    st_window_t *handle;
} st_render_opengl_window_t;
