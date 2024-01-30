#include "render.h"

#include <assert.h>

#include <GL/gl.h>
#include <GL/glu.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <safeclib/safe_mem_lib.h>
#include <safeclib/safe_str_lib.h>
#pragma GCC diagnostic pop
#include <safeclib/safe_types.h>

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
#define ERR_MSG_BUF_SIZE               1024

#ifdef _WIN32
    #define MINIMAL_OPENGL "1.1"
#elif __linux__
    #define MINIMAL_OPENGL "1.2"
#else
    #error Unknown target OS
#endif

static char               err_msg_buf[ERR_MSG_BUF_SIZE];
static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

ST_MODULE_DEF_GET_FUNC(render_opengl)
ST_MODULE_DEF_INIT_FUNC(render_opengl)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_render_opengl_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_render_import_functions(st_modctx_t *render_ctx,
 st_modctx_t *angle_ctx, st_modctx_t *drawq_ctx, st_modctx_t *dynarr_ctx,
 st_modctx_t *logger_ctx, st_modctx_t *matrix3x3_ctx, st_modctx_t *sprite_ctx,
 st_modctx_t *texture_ctx, st_modctx_t *vec2_ctx, st_gfxctx_t *gfxctx) {
    st_render_opengl_t            *module = render_ctx->data;
    st_gfxctx_get_api_t            st_gfxctx_get_api = NULL;
    st_gfxctx_get_ctx_t            st_gfxctx_get_ctx;
    st_gfxctx_get_window_t         st_gfxctx_get_window;
    st_glloader_init_t             st_glloader_init;
    st_glloader_quit_t             st_glloader_quit;
    st_glloader_get_proc_address_t st_glloader_get_proc_address;
    st_window_get_ctx_t            st_window_get_ctx;
    st_modctx_t                   *gfxctx_ctx = NULL;
    st_modctx_t                   *window_ctx;
    st_modctx_t                   *glloader_ctx = NULL;
    errno_t                        err;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "render_opengl: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    err = memset_s(&module->gl, sizeof(st_glfuncs_t), 0, sizeof(st_glfuncs_t));

    if (err) {
        strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to set initial states of OpenGL function "
         "pointers: %s",
         err_msg_buf);

        return false;
    }

    err = memset_s(&module->glsupported, sizeof(st_glsupported_t), 0,
     sizeof(st_glsupported_t));

    if (err) {
        strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to set initial states of supported features: "
         "%s",
         err_msg_buf);

        return false;
    }

    st_gfxctx_get_ctx = global_modsmgr_funcs.get_function(global_modsmgr,
     "gfxctx", NULL, "get_ctx");
    if (!st_gfxctx_get_ctx) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to load function \"get_ctx\" from module "
         "\"gfxctx\"\n");

        return false;
    }

    gfxctx_ctx = st_gfxctx_get_ctx(gfxctx);
    if (gfxctx_ctx)
        st_gfxctx_get_api = global_modsmgr_funcs.get_function_from_ctx(
         global_modsmgr, gfxctx_ctx, "get_api");

    if (!st_gfxctx_get_api) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to load function \"get_api\" from module "
         "\"gfxctx\"\n");

        return false;
    }

    module->gfxctx.handle = gfxctx;
    module->gfxctx.gapi = st_gfxctx_get_api(gfxctx);

    st_gfxctx_get_window = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, gfxctx_ctx, "get_window");
    if (!st_gfxctx_get_window) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to load function \"get_window\" from module "
         "\"gfxctx\"\n");

        return false;
    }

    module->window.handle = st_gfxctx_get_window(gfxctx);

    st_window_get_ctx = global_modsmgr_funcs.get_function(global_modsmgr,
     "window", NULL, "get_ctx");
    if (!st_window_get_ctx) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to load function \"get_ctx\" from module "
         "\"window\"\n");

        return false;
    }
    window_ctx = st_window_get_ctx(module->window.handle);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", angle, dtor);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", drawq, create);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", drawq, destroy);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", drawq, len);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", drawq, get_all);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", drawq, add);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", drawq, sort);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", drawq, clear);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", dynarr, create);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", dynarr, destroy);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", dynarr, append);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", dynarr, clear);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", dynarr, get);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", dynarr, get_all);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", dynarr, get_elements_count);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", gfxctx, make_current);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", gfxctx, swap_buffers);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", logger, info);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", logger, warning);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", matrix3x3, identity);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", matrix3x3, translate);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", matrix3x3, scale);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", matrix3x3, rrotate);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", matrix3x3, rhshear);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", matrix3x3, rvshear);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", sprite, get_texture);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", sprite, get_width);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", sprite, get_height);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", sprite, export_uv);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", texture, bind);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", vec2, apply_matrix3x3);

    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", window, get_width);
    ST_LOAD_FUNCTION_FROM_CTX("render_opengl", window, get_height);

    st_glloader_init = global_modsmgr_funcs.get_function(global_modsmgr,
     "glloader", gfxctx_ctx->name, "init");
    if (!st_glloader_init) {
        module->logger.warning(module->logger.ctx,
         "render_opengl: Unable to load function \"init\" from module "
         "\"glloader\". This is why unable to use OpenGL functions above "
         "OpenGL %s and extensions\n", MINIMAL_OPENGL);
    }

    if (st_glloader_init)
        glloader_ctx = st_glloader_init(logger_ctx, gfxctx);
    if (glloader_ctx) {
        st_glloader_quit = global_modsmgr_funcs.get_function_from_ctx(
         global_modsmgr, glloader_ctx, "quit");
        if (!st_glloader_quit) {
            module->logger.warning(module->logger.ctx,
             "render_opengl: Unable to load function \"quit\" from module "
             "\"glloader\"\n");
        }

        st_glloader_get_proc_address =
         global_modsmgr_funcs.get_function_from_ctx(global_modsmgr,
         glloader_ctx, "get_proc_address");
        if (!st_glloader_get_proc_address) {
            module->logger.error(module->logger.ctx,
             "render_opengl: Unable to load function \"get_proc_address\" "
             "from module \"gllogger\"\n");

            goto get_proc_addr_fail;
        }

        glfuncs_load_all(&module->gl, &module->glsupported,
         &module->logger, glloader_ctx, st_glloader_get_proc_address,
         module->gfxctx.gapi);

get_proc_addr_fail:
        if (st_glloader_quit)
            st_glloader_quit(glloader_ctx);
    }

    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3)
     && (!module->glsupported.shader_main || !module->glsupported.buf_main)) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Shaders and Buffer objects required for OpenGL >=3.0");

        return false;
    }
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL32) &&
     !module->glsupported.vao_main) {
        module->logger.error(module->logger.ctx,
         "render_opengl: VAO required for OpenGL >=3.2");

        return false;
    }

    return true;
}

static st_modctx_t *st_render_init(st_modctx_t *angle_ctx,
 st_modctx_t *drawq_ctx, st_modctx_t *dynarr_ctx, st_modctx_t *logger_ctx,
 st_modctx_t *matrix3x3_ctx, st_modctx_t *sprite_ctx, st_modctx_t *texture_ctx,
 st_modctx_t *vec2_ctx, st_gfxctx_t *gfxctx) {
    st_modctx_t        *render_ctx;
    st_render_opengl_t *module;
    st_shader_t         shd_vert = {0};
    st_shader_t         shd_frag = {0};

    render_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_render_opengl_data, sizeof(st_render_opengl_t));

    if (!render_ctx)
        return NULL;

    render_ctx->funcs = &st_render_opengl_funcs;

    module = render_ctx->data;
    module->angle.ctx = angle_ctx;
    module->drawq.ctx = drawq_ctx;
    module->dynarr.ctx = dynarr_ctx;
    module->logger.ctx = logger_ctx;
    module->matrix3x3.ctx = matrix3x3_ctx;
    module->vec2.ctx = vec2_ctx;

    if (!st_render_import_functions(render_ctx, angle_ctx, drawq_ctx,
     dynarr_ctx, logger_ctx, matrix3x3_ctx, sprite_ctx, texture_ctx, vec2_ctx,
     gfxctx))
        goto import_fail;

    module->drawq.handle = module->drawq.create(module->drawq.ctx);
    if (!module->drawq.handle) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to create draw queue");

        goto drawq_fail;
    }

    if (!vertices_init(render_ctx)) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to init vertices array");

        goto vertices_fail;
    }

    if (!batcher_init(render_ctx)) {
        module->logger.error(module->logger.ctx,
         "render_opengl: Unable to init batcher");

        goto batcher_fail;
    }

    module->gfxctx.make_current(module->gfxctx.handle);

    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
        vao_init(render_ctx, &module->vao);

    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL2)) {
        vbo_init(render_ctx, VBO_COMPONENTS_PER_VERTEX);
        /* We have shader sources for only OpenGL 3.3 */
        assert(module->gfxctx.gapi == ST_GAPI_GL33);

        if (!shader_init(render_ctx, &shd_vert, SHD_VERTEX,
         VERTEX_SHADER_SOURCE_GL33)) {
            if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
                goto vert_fail;
        }

        if (shd_vert.handle &&
         !shader_init(render_ctx, &shd_frag, SHD_FRAGMENT,
          FRAGMENT_SHADER_SOURCE_GL33)) {
            if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
                goto frag_fail;
        }

        if (shd_vert.handle && shd_frag.handle &&
         !shdprog_init(render_ctx, &shd_vert, &shd_frag)) {
            if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
                goto prog_fail;
        }

        if (module->shdprog.handle && !vertattr_init(render_ctx, &module->posattr,
         &module->vbo, &module->shdprog, ATTR_POS_NAME,
         ATTR_POS_COMPONENTS_COUNT, ATTR_POS_OFFSET)) {
            if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
                goto posattr_fail;
        }

        if (module->shdprog.handle && (module->posattr.handle != -1) &&
         !vertattr_init(render_ctx, &module->texcrdattr, &module->vbo,
         &module->shdprog, ATTR_TEXCOORD_NAME, ATTR_TEXCOORD_COMPONENTS_COUNT,
         ATTR_TEXCOORD_OFFSET)) {
            if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
                goto texcrdattr_fail;
        }

        shader_free(&shd_frag);
        shader_free(&shd_vert);

        if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3)) {
            vao_bind(&module->vao);
            vbo_bind(&module->vbo);
            vertattr_enable(&module->posattr);
            vertattr_enable(&module->texcrdattr);
            vao_unbind(&module->vao);
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

    module->logger.info(module->logger.ctx,
     "render_opengl: Render subsystem initialized");

    return render_ctx;

texcrdattr_fail:
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL2))
        vertattr_free(&module->texcrdattr);
posattr_fail:
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL2))
        shdprog_free(&module->shdprog);
prog_fail:
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL2))
        shader_free(&shd_frag);
frag_fail:
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL2))
        shader_free(&shd_vert);
vert_fail:
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL2))
        vbo_free(&module->vbo);
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
        vao_free(&module->vao);
    batcher_free(&module->batcher);
batcher_fail:
    vertices_free(&module->vertices);
vertices_fail:
    module->drawq.destroy(module->drawq.handle);
drawq_fail:
import_fail:
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, render_ctx);

    return NULL;
}

static void st_render_quit(st_modctx_t *render_ctx) {
    st_render_opengl_t *module = render_ctx->data;

    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL2)) {
        vertattr_free(&module->texcrdattr);
        vertattr_free(&module->posattr);
        shdprog_free(&module->shdprog);
        vbo_free(&module->vbo);
    }
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3))
        vao_free(&module->vao);
    batcher_free(&module->batcher);
    vertices_free(&module->vertices);
    module->drawq.destroy(module->drawq.handle);

    module->logger.info(module->logger.ctx,
     "render_opengl: Render subsystem destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, render_ctx);
}

static void st_render_put_sprite(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float pivot_x, float pivot_y) {
    st_render_opengl_t *module = render_ctx->data;

    module->drawq.add(module->drawq.handle, sprite, x, y, z, hscale, vscale,
     0.0f, 0.0f, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_rdangled(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y) {
    st_render_opengl_t *module = render_ctx->data;

    module->drawq.add(module->drawq.handle, sprite, x, y, z, hscale, vscale,
     radians, 0.0f, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_dgangled(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y) {
    st_render_opengl_t *module = render_ctx->data;

    module->drawq.add(module->drawq.handle, sprite, x, y, z, hscale, vscale,
     module->angle.dtor(module->angle.ctx, degrees), 0.0f, 0.0f, pivot_x,
     pivot_y);
}

static void st_render_put_sprite_rhsheared(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y) {
    st_render_opengl_t *module = render_ctx->data;

    module->drawq.add(module->drawq.handle, sprite, x, y, z, hscale, vscale,
     0.0f, radians, 0.0f, pivot_x, pivot_y);
}

static void st_render_put_sprite_dhsheared(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y) {
    st_render_opengl_t *module = render_ctx->data;

    module->drawq.add(module->drawq.handle, sprite, x, y, z, hscale, vscale,
     0.0f, module->angle.dtor(module->angle.ctx, degrees), 0.0f, pivot_x,
     pivot_y);
}

static void st_render_put_sprite_rvsheared(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float radians, float pivot_x, float pivot_y) {
    st_render_opengl_t *module = render_ctx->data;

    module->drawq.add(module->drawq.handle, sprite, x, y, z, hscale, vscale,
     0.0f, 0.0f, radians, pivot_x, pivot_y);
}

static void st_render_put_sprite_dvsheared(st_modctx_t *render_ctx,
 const st_sprite_t *sprite, float x, float y, float z, float hscale,
 float vscale, float degrees, float pivot_x, float pivot_y) {
    st_render_opengl_t *module = render_ctx->data;

    module->drawq.add(module->drawq.handle, sprite, x, y, z, hscale, vscale,
     0.0f, 0.0f, module->angle.dtor(module->angle.ctx, degrees), pivot_x,
     pivot_y);
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

static void st_render_process_queue(st_modctx_t *render_ctx) {
    st_render_opengl_t *module = render_ctx->data;
    unsigned            window_width = module->window.get_width(
     module->window.handle);
    unsigned            window_height = module->window.get_height(
     module->window.handle);
    const st_drawrec_t *draw_entries = module->drawq.get_all(
     module->drawq.handle);
    size_t              draw_entries_count = module->drawq.len(
     module->drawq.handle);

    vertices_clear(&module->vertices);
    batcher_clear(&module->batcher);

    if (draw_entries_count == 0)
        return;

    module->drawq.sort(module->drawq.handle);
    for (size_t i = 0; i < draw_entries_count; i++) {
        const st_sprite_t  *sprite = draw_entries[i].sprite;
        const st_texture_t *texture = module->sprite.get_texture(sprite);
        unsigned            sprite_width = module->sprite.get_width(sprite);
        unsigned            sprite_height = module->sprite.get_height(sprite);
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

        module->matrix3x3.identity(module->matrix3x3.ctx, &matrix);

        module->matrix3x3.translate(&matrix, draw_entries[i].x,
         draw_entries[i].y);

        if (do_scaling)
            module->matrix3x3.scale(&matrix, draw_entries[i].hscale,
             draw_entries[i].vscale);
        if (do_hshearing)
            module->matrix3x3.rhshear(&matrix, draw_entries[i].hshear);
        if (do_vshearing)
            module->matrix3x3.rvshear(&matrix, draw_entries[i].vshear);
        if (do_rotation)
            module->matrix3x3.rrotate(&matrix, draw_entries[i].angle);

        module->vec2.apply_matrix3x3(module->vec2.ctx, &tetragon.upper_left.x,
         &tetragon.upper_left.y, &matrix);
        module->vec2.apply_matrix3x3(module->vec2.ctx, &tetragon.upper_right.x,
         &tetragon.upper_right.y, &matrix);
        module->vec2.apply_matrix3x3(module->vec2.ctx, &tetragon.lower_left.x,
         &tetragon.lower_left.y, &matrix);
        module->vec2.apply_matrix3x3(module->vec2.ctx, &tetragon.lower_right.x,
         &tetragon.lower_right.y, &matrix);

        screen_to_clip(&tetragon.upper_left.x, &tetragon.upper_left.y,
         window_width, window_height);
        screen_to_clip(&tetragon.upper_right.x, &tetragon.upper_right.y,
         window_width, window_height);
        screen_to_clip(&tetragon.lower_left.x, &tetragon.lower_left.y,
         window_width, window_height);
        screen_to_clip(&tetragon.lower_right.x, &tetragon.lower_right.y,
         window_width, window_height);

        batcher_process_texture(&module->batcher, texture);

        module->sprite.export_uv(sprite, &uv);

        vertices_add(&module->vertices, tetragon.upper_left.x,
         tetragon.upper_left.y, pos_z, uv.upper_left.u, uv.upper_left.v);
        vertices_add(&module->vertices, tetragon.upper_right.x,
         tetragon.upper_right.y, pos_z, uv.upper_right.u, uv.upper_right.v);
        vertices_add(&module->vertices, tetragon.lower_left.x,
         tetragon.lower_left.y, pos_z, uv.lower_left.u, uv.lower_left.v);
        vertices_add(&module->vertices, tetragon.upper_right.x,
         tetragon.upper_right.y, pos_z, uv.upper_right.u, uv.upper_right.v);
        vertices_add(&module->vertices, tetragon.lower_left.x,
         tetragon.lower_left.y, pos_z, uv.lower_left.u, uv.lower_left.v);
        vertices_add(&module->vertices, tetragon.lower_right.x,
         tetragon.lower_right.y, pos_z, uv.lower_right.u, uv.lower_right.v);
    }

    batcher_finalize(&module->batcher);
}

static void st_render_process(st_modctx_t *render_ctx) {
    st_render_opengl_t *module = render_ctx->data;

    st_render_process_queue(render_ctx);

    module->gfxctx.make_current(module->gfxctx.handle);
    glClear((GLbitfield)GL_COLOR_BUFFER_BIT | (GLbitfield)GL_DEPTH_BUFFER_BIT);

    shdprog_use(&module->shdprog);
    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3)) {
        vao_bind(&module->vao);
    } else {
        vbo_bind(&module->vbo);
        vertattr_enable(&module->posattr);
        vertattr_enable(&module->texcrdattr);
    }

    vbo_set_vertices(&module->vbo, &module->vertices);

    for (size_t i = 0; i < batcher_get_entries_count(&module->batcher); i++) {
        GLenum error;

        if (!batcher_bind_texture(&module->batcher, i, 0))
            break;

        glDrawArrays(GL_TRIANGLES,
         batcher_get_first_vertex_index(&module->batcher, i),
         batcher_get_vertices_count(&module->batcher, i));

        error = glGetError();
        if (error != GL_NO_ERROR) {
            module->logger.error(module->logger.ctx,
             "render_opengl: Unable to draw array: %s", gluErrorString(error));

            break;
        }
    }

    if (glapi_least(module->gfxctx.gapi, ST_GAPI_GL3)) {
        vao_unbind(&module->vao);
    } else {
        vertattr_disable(&module->texcrdattr);
        vertattr_disable(&module->posattr);
        vbo_unbind(&module->vbo);
    }
    shdprog_unuse(&module->shdprog);
    module->gfxctx.swap_buffers(module->gfxctx.handle);
    module->drawq.clear(module->drawq.handle);
}
