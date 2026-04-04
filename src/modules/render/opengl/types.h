#pragma once

#include <GL/gl.h>

#include "steroids/modules/angle.h"
#include "steroids/modules/dpsrvconn.h"
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

#include "glfuncs.h"

typedef struct {
    const st_texture_t *texture;
    size_t              first_vertex_index;
    size_t              vertices_count;
} st_batch_entry_t;

typedef struct {
    st_dynarr_t        *entries;
    const st_texture_t *current_texture;
    size_t              current_first_vertex_index;
    size_t              current_vertex_index;
} st_batcher_t;

typedef struct {
    const st_glfuncs_t *gl;
    GLuint        handle;
} st_vao_t;

typedef struct {
    const st_glfuncs_t *gl;
    GLuint        handle;
    unsigned      components_per_vertex;
    size_t        vertices_size;
} st_vbo_t;

typedef struct {
    const st_glfuncs_t *gl;
    GLuint        handle;
} st_shader_t;

typedef struct {
    const st_glfuncs_t *gl;
    GLuint        handle;
} st_shdprog_t;

typedef struct {
    const st_glfuncs_t *gl;
    GLint         handle;
} st_vertattr_t;

typedef struct st_renderctx {
    st_modctx_t;
    st_anglectx_t     *angle_ctx;
    st_drawqctx_t     *drawq_ctx;
    st_dynarrctx_t    *dynarr_ctx;
    st_gfxctx_t       *gfxctx;
    st_glloaderctx_t  *glloader_ctx;
    st_gldebugctx_t   *gldebug_ctx;
    st_loggerctx_t    *logger_ctx;
    st_matrix3x3ctx_t *matrix3x3_ctx;
    st_spritectx_t    *sprite_ctx;
    st_texturectx_t   *texture_ctx;
    st_vec2ctx_t      *vec2_ctx;
    st_dpsrvconnctx_t *dpsrvconn_ctx;
    st_window_t       *window;
    
    st_gapi_t          gapi;

    st_glfuncs_t       gl;
    st_glsupported_t   glsupported;

    st_drawq_t        *queue;
    st_dynarr_t       *vertices;
    st_batcher_t       batcher;
    st_vao_t           vao;
    st_vbo_t           vbo;
    st_shdprog_t       shdprog;
    st_vertattr_t      posattr;
    st_vertattr_t      texcrdattr;
} st_renderctx_t;

#define ST_RENDERCTX_T_DEFINED
