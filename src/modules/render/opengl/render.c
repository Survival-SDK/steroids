#include "render.h"

#include <assert.h>
#include <stdio.h>

#include <GL/gl.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/params.h"

#include "batcher.inl"
#include "shader.inl"
#include "shdprog.inl"
#include "vao.inl"
#include "vertices.inl" // NOLINT(llvm-include-order)
#include "vbo.inl"
#include "vertattr.inl"

#define DEPTH_RANGE_NEAR_VAL           0.0
#define DEPTH_RANGE_FAR_VAL            1.0
#define VBO_COMPONENTS_PER_VERTEX      5

#define ATTR_POS_NAME                  "pos"
#define ATTR_POS_COMPONENTS_COUNT      3
#define ATTR_POS_OFFSET                0
#define ATTR_TEXCOORD_NAME             "vert_tex_coord"
#define ATTR_TEXCOORD_COMPONENTS_COUNT 2
#define ATTR_TEXCOORD_OFFSET           \
 (sizeof(float) * ATTR_POS_COMPONENTS_COUNT)

#ifdef _WIN32
    #define MINIMAL_OPENGL "1.1"
#elif __linux__
    #define MINIMAL_OPENGL "1.2"
#else
    #error Unknown target OS
#endif

static st_renderctx_t *st_render_init(const st_param_t params[]);
static void st_render_quit(st_renderctx_t *render_ctx);

static void st_render_put_sprite(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float pivot_x, float pivot_y);
static void st_render_put_sprite_rdangled(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
static void st_render_put_sprite_dgangled(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
static void st_render_put_sprite_rhsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
static void st_render_put_sprite_dhsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
static void st_render_put_sprite_rvsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y);
static void st_render_put_sprite_dvsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y);
static void st_render_process(st_renderctx_t *render_ctx);

static st_renderctx_funcs_t st_render_opengl_funcs = {
    st_modctx_funcs,
    .put_sprite           = st_render_put_sprite,
    .put_sprite_rdangled  = st_render_put_sprite_rdangled,
    .put_sprite_dgangled  = st_render_put_sprite_dgangled,
    .put_sprite_rhsheared = st_render_put_sprite_rhsheared,
    .put_sprite_dhsheared = st_render_put_sprite_dhsheared,
    .put_sprite_rvsheared = st_render_put_sprite_rvsheared,
    .put_sprite_dvsheared = st_render_put_sprite_dvsheared,
    .process              = st_render_process,
};

static const st_modprerq_t mod_prereqs[] = {
    { "angle", NULL, },
    { "drawq", NULL, },
    { "dynarr", NULL, },
    { "gldebug", NULL, },
    { "glloader", NULL, },
    { "logger", NULL, },
    { "matrix3x3", NULL, },
    { "sprite", NULL, },
    { "vec2", NULL, },
    {0},
};

st_moddata_t *st_module_render_opengl_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("render", "opengl", ST_MODULE_TYPE, mod_prereqs,
     st_render_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_render_opengl_init(modsmgr);
}
#endif

// static bool glapi_least(st_gapi_t current_api, st_gapi_t req_api) {
//     return req_api >= ST_GAPI_GL1
//         && req_api <= ST_GAPI_GL46
//         && current_api >= req_api;
// }

static st_renderctx_t *st_render_init(const st_param_t params[]) {
    st_renderctx_t  *render_ctx;
    st_modsmgr_t    *modsmgr;
    st_shader_t      shd_vert = {0};
    st_shader_t      shd_frag = {0};
    st_gfxctx_t     *gfxctx;
    // st_windowctx_t  *window_ctx;

    modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    if (!modsmgr)
        return NULL;

    render_ctx = (st_renderctx_t *)st_modctx_new("render", "opengl", 
     sizeof(st_renderctx_t), NULL, &st_render_opengl_funcs, 
     (st_object_dtor_t)st_render_quit);
    if (!render_ctx)
        return NULL;

    render_ctx->logger_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "logger", NULL);
    if (!render_ctx->logger_ctx) {
        fprintf(stderr,
         "render_opengl: Unable to get logger context\n");
        goto logger_fail;
    }

    render_ctx->angle_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "angle", NULL);
    if (!render_ctx->angle_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get angle context");
        goto angle_fail;
    }

    render_ctx->drawq_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "drawq", NULL);
    if (!render_ctx->drawq_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get drawq context");
        goto drawq_ctx_fail;
    }

    render_ctx->dynarr_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "dynarr", NULL);
    if (!render_ctx->dynarr_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get dynarr context");
        goto dynarr_ctx_fail;
    }

    render_ctx->matrix3x3_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "matrix3x3",
     NULL);
    if (!render_ctx->matrix3x3_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get matrix3x3 context");
        goto matrix3x3_fail;
    }

    render_ctx->sprite_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "sprite", NULL);
    if (!render_ctx->sprite_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get sprite context");
        goto sprite_fail;
    }

    render_ctx->texture_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "texture",
     NULL);
    if (!render_ctx->texture_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get texture context");
        goto texture_fail;
    }

    render_ctx->vec2_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "vec2", NULL);
    if (!render_ctx->vec2_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get vec2 context");
        goto vec2_fail;
    }

    gfxctx = st_modctx_get_param_as_ptr(params, "gfxctx");
    if (!gfxctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to get gfxctx parameter");
        goto gfxctx_fail;
    }
    render_ctx->gfxctx = gfxctx;
    render_ctx->gapi = ST_GFXCTX_CALL(gfxctx, get_api);
    render_ctx->window = ST_GFXCTX_CALL(gfxctx, get_window);
    render_ctx->dpsrvconn_ctx = (st_dpsrvconnctx_t *)ST_WINDOW_CALL(
     render_ctx->window, get_owner);

    render_ctx->glloader_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "glloader",
     NULL);
    if (!render_ctx->glloader_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, warning,
         "render_opengl: Unable to get glloader context. Unable to use OpenGL "
         "functions above OpenGL %s and extensions", MINIMAL_OPENGL);
    }

    render_ctx->gldebug_ctx = ST_MODSMGR_CALL(modsmgr, get_singleton, "gldebug",
     NULL);
    if (!render_ctx->gldebug_ctx) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, warning,
         "render_opengl: Unable to get gldebug context");
    }

    render_ctx->gl = (st_glfuncs_t){0};
    render_ctx->glsupported = (st_glsupported_t){0};

    if (render_ctx->glloader_ctx) {
        if (!glfuncs_load_all(&render_ctx->gl, &render_ctx->glsupported,
         render_ctx->logger_ctx, render_ctx->glloader_ctx, render_ctx->gapi)) {
            ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
             "render_opengl: Unable to load OpenGL functions");
            goto glfuncs_fail;
        }
    }

    if (glapi_least(render_ctx->gapi, ST_GAPI_GL3)
     && (!render_ctx->glsupported.shader_main
      || !render_ctx->glsupported.buf_main)) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Shaders and Buffer objects required for OpenGL >=3.0");
        goto version_check_fail;
    }

    if (glapi_least(render_ctx->gapi, ST_GAPI_GL32)
     && !render_ctx->glsupported.vao_main) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: VAO required for OpenGL >=3.2");
        goto version_check_fail;
    }

    render_ctx->queue = ST_DRAWQCTX_CALL(render_ctx->drawq_ctx, create);
    if (!render_ctx->queue) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to create draw queue");
        goto queue_fail;
    }

    if (!vertices_init(&render_ctx->vertices, render_ctx->logger_ctx,
     render_ctx->dynarr_ctx)) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to init vertices array");
        goto vertices_fail;
    }

    if (!batcher_init(&render_ctx->batcher, render_ctx->logger_ctx,
     render_ctx->dynarr_ctx)) {
        ST_LOGGERCTX_CALL(render_ctx->logger_ctx, error,
         "render_opengl: Unable to init batcher");
        goto batcher_fail;
    }

    ST_GFXCTX_CALL(render_ctx->gfxctx, make_current);

    if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
        vao_init(&render_ctx->vao, &render_ctx->gl);

    if (glapi_least(render_ctx->gapi, ST_GAPI_GL2)) {
        vbo_init(&render_ctx->vbo, &render_ctx->gl, VBO_COMPONENTS_PER_VERTEX);
        /* We have shader sources for only OpenGL 3.3 */
        assert(render_ctx->gapi == ST_GAPI_GL33);

        if (!shader_init(&shd_vert, render_ctx->logger_ctx,
         render_ctx->gldebug_ctx, &render_ctx->gl, SHD_VERTEX,
         VERTEX_SHADER_SOURCE_GL33)) {
            if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
                goto vert_fail;
        }

        if (shd_vert.handle && !shader_init(&shd_frag, render_ctx->logger_ctx,
         render_ctx->gldebug_ctx, &render_ctx->gl, SHD_FRAGMENT,
         FRAGMENT_SHADER_SOURCE_GL33)) {
            if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
                goto frag_fail;
        }

        if (shd_vert.handle && shd_frag.handle && !shdprog_init(
         &render_ctx->shdprog, render_ctx->logger_ctx, render_ctx->gldebug_ctx,
         &render_ctx->gl, &shd_vert, &shd_frag)) {
            if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
                goto prog_fail;
        }

        if (render_ctx->shdprog.handle && !vertattr_init(&render_ctx->posattr,
         render_ctx->logger_ctx, render_ctx->gldebug_ctx, &render_ctx->gl,
         &render_ctx->vbo, &render_ctx->shdprog, ATTR_POS_NAME,
         ATTR_POS_COMPONENTS_COUNT, ATTR_POS_OFFSET)) {
            if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
                goto posattr_fail;
        }

        if (render_ctx->shdprog.handle && (render_ctx->posattr.handle != -1)
         && !vertattr_init(&render_ctx->texcrdattr, render_ctx->logger_ctx,
         render_ctx->gldebug_ctx, &render_ctx->gl, &render_ctx->vbo,
         &render_ctx->shdprog, ATTR_TEXCOORD_NAME,
         ATTR_TEXCOORD_COMPONENTS_COUNT, ATTR_TEXCOORD_OFFSET)) {
            if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
                goto texcrdattr_fail;
        }

        shader_free(&shd_frag);
        shader_free(&shd_vert);

        if (glapi_least(render_ctx->gapi, ST_GAPI_GL3)) {
            vao_bind(&render_ctx->vao);
            vbo_bind(&render_ctx->vbo);
            vertattr_enable(&render_ctx->posattr);
            vertattr_enable(&render_ctx->texcrdattr);
            vao_unbind(&render_ctx->vao);
        }
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDepthRange(DEPTH_RANGE_NEAR_VAL, DEPTH_RANGE_FAR_VAL);
    glClearDepth(DEPTH_RANGE_FAR_VAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    ST_LOGGERCTX_CALL(render_ctx->logger_ctx, info,
     "render_opengl: Render subsystem initialized");

    return render_ctx;

texcrdattr_fail:
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL2))
        vertattr_free(&render_ctx->posattr);
posattr_fail:
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL2))
        shdprog_free(&render_ctx->shdprog);
prog_fail:
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL2))
        shader_free(&shd_frag);
frag_fail:
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL2))
        shader_free(&shd_vert);
vert_fail:
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL2))
        vbo_free(&render_ctx->vbo);
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
        vao_free(&render_ctx->vao);
    batcher_free(&render_ctx->batcher);
batcher_fail:
    vertices_free(render_ctx->vertices);
vertices_fail:
    ST_DRAWQ_CALL(render_ctx->queue, destroy);
queue_fail:
version_check_fail:
glfuncs_fail:
vec2_fail:
texture_fail:
sprite_fail:
matrix3x3_fail:
dynarr_ctx_fail:
drawq_ctx_fail:
angle_fail:
gfxctx_fail:
logger_fail:
    free(render_ctx);

    return NULL;
}

static void st_render_quit(st_renderctx_t *render_ctx) {
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL2)) {
        vertattr_free(&render_ctx->texcrdattr);
        vertattr_free(&render_ctx->posattr);
        shdprog_free(&render_ctx->shdprog);
        vbo_free(&render_ctx->vbo);
    }
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL3))
        vao_free(&render_ctx->vao);
    batcher_free(&render_ctx->batcher);
    vertices_free(render_ctx->vertices);
    ST_DRAWQ_CALL(render_ctx->queue, destroy);

    ST_LOGGERCTX_CALL(render_ctx->logger_ctx, info,
     "render_opengl: Render subsystem destroyed");
    free(render_ctx);
}

static void st_render_put_sprite(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float pivot_x, float pivot_y) {
    ST_DRAWQ_CALL(render_ctx->queue, add, sprite, x, y, z, hscale, vscale,
     0.0f, 0.0f, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_rdangled(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y) {
    ST_DRAWQ_CALL(render_ctx->queue, add, sprite, x, y, z, hscale, vscale,
     radians, 0.0f, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_dgangled(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y) {
    float radians = ST_ANGLECTX_CALL(render_ctx->angle_ctx, dtor, degrees);

    ST_DRAWQ_CALL(render_ctx->queue, add, sprite, x, y, z, hscale, vscale,
     radians, 0.0f, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_rhsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y) {
    ST_DRAWQ_CALL(render_ctx->queue, add, sprite, x, y, z, hscale, vscale,
     0.0f, radians, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_dhsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y) {
    float radians = ST_ANGLECTX_CALL(render_ctx->angle_ctx, dtor, degrees);

    ST_DRAWQ_CALL(render_ctx->queue, add, sprite, x, y, z, hscale, vscale,
     0.0f, radians, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_rvsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y) {
    ST_DRAWQ_CALL(render_ctx->queue, add, sprite, x, y, z, hscale, vscale,
     0.0f, 0.0f, radians, pivot_x, pivot_y);
}

static void st_render_put_sprite_dvsheared(const st_renderctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y) {
    float radians = ST_ANGLECTX_CALL(render_ctx->angle_ctx, dtor, degrees);

    ST_DRAWQ_CALL(render_ctx->queue, add, sprite, x, y, z, hscale, vscale,
     0.0f, 0.0f, radians, pivot_x, pivot_y);
}

typedef struct {
    float x;
    float y;
} pos_t;

typedef struct {
    pos_t upper_left;
    pos_t upper_right;
    pos_t lower_left;
    pos_t lower_right;
} tetragon_t;

static void screen_to_clip(float *x, float *y, unsigned window_width,
 unsigned window_height) {
    *x = *x / (float)window_width * 2 - 1.0f;
    *y = 1.0f - *y / (float)window_height * 2;
}

static void st_render_process_queue(st_renderctx_t *render_ctx) {
    unsigned            window_width = ST_WINDOW_CALL(render_ctx->window,
     get_width);
    unsigned            window_height = ST_WINDOW_CALL(render_ctx->window, 
     get_height);
    const st_drawrec_t *draw_entries = ST_DRAWQ_CALL(render_ctx->queue,
     get_all);
    size_t              draw_entries_count = ST_DRAWQ_CALL(render_ctx->queue,
     len);

    vertices_clear(render_ctx->vertices);
    batcher_clear(&render_ctx->batcher);

    if (draw_entries_count == 0)
        return;

    ST_DRAWQ_CALL(render_ctx->queue, sort);
    for (size_t i = 0; i < draw_entries_count; i++) {
        const st_sprite_t  *sprite = draw_entries[i].sprite;
        const st_texture_t *texture = ST_SPRITE_CALL(sprite, get_texture);
        unsigned            sprite_width = ST_SPRITE_CALL(sprite, get_width);
        unsigned            sprite_height = ST_SPRITE_CALL(sprite, get_height);
        float               pos_z = draw_entries[i].z / (float)UINT16_MAX +
         0.5f; // NOLINT(readability-magic-numbers)
        st_uv_t             uv;

        tetragon_t tetragon = {
            .upper_left  = {
                .x = -draw_entries[i].pivot_x,
                .y = -draw_entries[i].pivot_y,
            },
            .upper_right = {
                .x = (float)sprite_width - draw_entries[i].pivot_x,
                .y = -draw_entries[i].pivot_y,
            },
            .lower_left  = {
                .x = -draw_entries[i].pivot_x,
                .y = (float)sprite_height - draw_entries[i].pivot_y,
            },
            .lower_right = {
                .x = (float)sprite_width - draw_entries[i].pivot_x,
                .y = (float)sprite_height - draw_entries[i].pivot_y,
            },
        };

        st_matrix3x3_t matrix;
        bool           do_scaling = draw_entries[i].hscale != 1.0f
         || draw_entries[i].vscale != 1.0f;
        bool           do_hshearing = draw_entries[i].hshear != 0;
        bool           do_vshearing = draw_entries[i].vshear != 0;
        bool           do_rotation = draw_entries[i].angle != 0;

        ST_MATRIX3X3CTX_CALL(render_ctx->matrix3x3_ctx, identity, &matrix);

        ST_MATRIX3X3CTX_CALL(render_ctx->matrix3x3_ctx, translate, &matrix,
         draw_entries[i].x, draw_entries[i].y);

        if (do_scaling)
            ST_MATRIX3X3CTX_CALL(render_ctx->matrix3x3_ctx, scale, &matrix,
             draw_entries[i].hscale, draw_entries[i].vscale);
        if (do_hshearing)
            ST_MATRIX3X3CTX_CALL(render_ctx->matrix3x3_ctx, rhshear, &matrix,
             draw_entries[i].hshear);
        if (do_vshearing)
            ST_MATRIX3X3CTX_CALL(render_ctx->matrix3x3_ctx, rvshear, &matrix,
             draw_entries[i].vshear);
        if (do_rotation)
            ST_MATRIX3X3CTX_CALL(render_ctx->matrix3x3_ctx, rrotate, &matrix,
             draw_entries[i].angle);

        ST_VEC2CTX_CALL(render_ctx->vec2_ctx, apply_matrix3x3,
         &tetragon.upper_left.x, &tetragon.upper_left.y, &matrix);
        ST_VEC2CTX_CALL(render_ctx->vec2_ctx, apply_matrix3x3,
         &tetragon.upper_right.x, &tetragon.upper_right.y, &matrix);
        ST_VEC2CTX_CALL(render_ctx->vec2_ctx, apply_matrix3x3,
         &tetragon.lower_left.x, &tetragon.lower_left.y, &matrix);
        ST_VEC2CTX_CALL(render_ctx->vec2_ctx, apply_matrix3x3,
         &tetragon.lower_right.x, &tetragon.lower_right.y, &matrix);

        screen_to_clip(&tetragon.upper_left.x, &tetragon.upper_left.y,
         window_width, window_height);
        screen_to_clip(&tetragon.upper_right.x, &tetragon.upper_right.y,
         window_width, window_height);
        screen_to_clip(&tetragon.lower_left.x, &tetragon.lower_left.y,
         window_width, window_height);
        screen_to_clip(&tetragon.lower_right.x, &tetragon.lower_right.y,
         window_width, window_height);

        batcher_process_texture(&render_ctx->batcher, texture);

        ST_SPRITE_CALL(sprite, export_uv, &uv);

        vertices_add(render_ctx->vertices, tetragon.upper_left.x,
         tetragon.upper_left.y, pos_z, uv.upper_left.u, uv.upper_left.v);
        vertices_add(render_ctx->vertices, tetragon.upper_right.x,
         tetragon.upper_right.y, pos_z, uv.upper_right.u, uv.upper_right.v);
        vertices_add(render_ctx->vertices, tetragon.lower_left.x,
         tetragon.lower_left.y, pos_z, uv.lower_left.u, uv.lower_left.v);
        vertices_add(render_ctx->vertices, tetragon.upper_right.x,
         tetragon.upper_right.y, pos_z, uv.upper_right.u, uv.upper_right.v);
        vertices_add(render_ctx->vertices, tetragon.lower_left.x,
         tetragon.lower_left.y, pos_z, uv.lower_left.u, uv.lower_left.v);
        vertices_add(render_ctx->vertices, tetragon.lower_right.x,
         tetragon.lower_right.y, pos_z, uv.lower_right.u, uv.lower_right.v);
    }

    batcher_finalize(&render_ctx->batcher);
}

static GLenum check_and_print_opengl_error(const st_loggerctx_t *logger_ctx, 
 const st_gldebugctx_t *gldebug_ctx, bool force_err_msg, const char *message) {
    GLenum error;

    error = glGetError();
    if (error != GL_NO_ERROR) {
        if (gldebug_ctx)
            ST_LOGGERCTX_CALL(logger_ctx, error,
                "render_opengl: %s: %s", message,
                ST_GLDEBUGCTX_CALL(gldebug_ctx, get_error_msg, error));
        else
            ST_LOGGERCTX_CALL(logger_ctx, error,
                "render_opengl: %s", message);
    } else if (force_err_msg) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: %s", message);
    }

    return error;
}

static void st_render_process(st_renderctx_t *render_ctx) {
    st_render_process_queue(render_ctx);

    if (!ST_GFXCTX_CALL(render_ctx->gfxctx, make_current)) {
        check_and_print_opengl_error(render_ctx->logger_ctx,
         render_ctx->gldebug_ctx, true, 
         "Failed to make OpenGL context current, skipping frame");
        ST_DRAWQ_CALL(render_ctx->queue, clear);
        return;
    }
    glClear((GLbitfield)GL_COLOR_BUFFER_BIT | (GLbitfield)GL_DEPTH_BUFFER_BIT);

    shdprog_use(&render_ctx->shdprog);
    if (glapi_least(render_ctx->gapi, ST_GAPI_GL3)) {
        vao_bind(&render_ctx->vao);
    } else {
        vbo_bind(&render_ctx->vbo);
        vertattr_enable(&render_ctx->posattr);
        vertattr_enable(&render_ctx->texcrdattr);
    }

    vbo_set_vertices(&render_ctx->vbo, render_ctx->vertices);

    for (size_t i = 0; i < batcher_get_entries_count(&render_ctx->batcher); i++) {
        if (!batcher_bind_texture(&render_ctx->batcher, i, 0))
            break;

        glDrawArrays(GL_TRIANGLES,
         batcher_get_first_vertex_index(&render_ctx->batcher, i),
         batcher_get_vertices_count(&render_ctx->batcher, i));

        if (check_and_print_opengl_error(render_ctx->logger_ctx,
         render_ctx->gldebug_ctx, false, "Unable to draw array") != GL_NO_ERROR)
            break;
    }

    if (glapi_least(render_ctx->gapi, ST_GAPI_GL3)) {
        vao_unbind(&render_ctx->vao);
    } else {
        vertattr_disable(&render_ctx->texcrdattr);
        vertattr_disable(&render_ctx->posattr);
        vbo_unbind(&render_ctx->vbo);
    }
    shdprog_unuse(&render_ctx->shdprog);

    if (!ST_GFXCTX_CALL(render_ctx->gfxctx, swap_buffers)) {
        check_and_print_opengl_error(render_ctx->logger_ctx,
         render_ctx->gldebug_ctx, true, "Failed to swap buffers");
    }

    ST_DRAWQ_CALL(render_ctx->queue, clear);
}
