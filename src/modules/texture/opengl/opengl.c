#include "opengl.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>

#include <GL/gl.h>

#include "steroids/object.h"

#define ERRMSGBUF_SIZE 128

static void (*glGenerateMipmap)(GLenum target);

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_texture_funcs_t texture_funcs = {
    .destroy    = st_texture_destroy,
    .bind       = st_texture_bind,
    .get_width  = st_texture_get_width,
    .get_height = st_texture_get_height,
};

ST_MODULE_DEF_GET_FUNC(texture_opengl)
ST_MODULE_DEF_INIT_FUNC(texture_opengl)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_texture_opengl_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_texture_import_functions(st_modctx_t *texture_ctx,
 st_modctx_t *bitmap_ctx, st_modctx_t *logger_ctx, st_gfxctx_t *gfxctx) {
    st_texture_opengl_t           *module = texture_ctx->data;
    st_modctx_t                   *gfxctx_ctx;
    st_modctx_t                   *glloader_ctx;
    st_glloader_init_t             st_glloader_init;
    st_glloader_quit_t             st_glloader_quit;
    st_glloader_get_proc_address_t st_glloader_get_proc_address;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "texture_opengl: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    gfxctx_ctx = st_object_get_owner(gfxctx);
    module->gfxctx.api = ST_GFXCTX_CALL(gfxctx, get_api);

    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", logger, info);
    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", logger, warning);

    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", bitmap, load);
    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", bitmap, memload);
    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", bitmap, destroy);
    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", bitmap, get_data);
    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", bitmap, get_width);
    ST_LOAD_FUNCTION_FROM_CTX("texture_opengl", bitmap, get_height);

    ST_LOAD_FUNCTION("texture_opengl", gldebug, NULL, init);
    ST_LOAD_FUNCTION("texture_opengl", gldebug, NULL, quit);
    ST_LOAD_FUNCTION("texture_opengl", gldebug, NULL, label_texture);
    ST_LOAD_FUNCTION("texture_opengl", gldebug, NULL, unlabel_texture);
    ST_LOAD_FUNCTION("texture_opengl", gldebug, NULL, get_error_msg);

    st_glloader_init = global_modsmgr_funcs.get_function(global_modsmgr,
     "glloader", gfxctx_ctx->name, "init");
    if (!st_glloader_init) {
        module->logger.error(module->logger.ctx,
         "texture_opengl: Unable to load function \"init\" from module "
         "\"glloader\"\n");

        return false;
    }

    glloader_ctx = st_glloader_init(logger_ctx, gfxctx);
    if (glloader_ctx) {
        st_glloader_quit = global_modsmgr_funcs.get_function_from_ctx(
         global_modsmgr, glloader_ctx, "quit");
        if (!st_glloader_quit) {
            module->logger.warning(module->logger.ctx,
             "texture_opengl: Unable to load function \"quit\" from module "
             "\"glloader\"\n");
        }

        st_glloader_get_proc_address =
         global_modsmgr_funcs.get_function_from_ctx(global_modsmgr,
         glloader_ctx, "get_proc_address");
        if (!st_glloader_get_proc_address) {
            module->logger.error(module->logger.ctx,
             "texture_opengl: Unable to load function \"get_proc_address\" "
             "from module \"gllogger\"\n");

            goto get_proc_addr_fail;
        }

        glGenerateMipmap = st_glloader_get_proc_address(glloader_ctx,
         "glGenerateMipmap");
        if (!glGenerateMipmap) {
            module->logger.warning(module->logger.ctx,
             "texture_opengl: Unable to load function \"glGenerateMipmap\". "
             "Mipmaps will not be supported in this OpenGL context");
        }
    } else {
        glGenerateMipmap = NULL;
    }

get_proc_addr_fail:
    if (glloader_ctx && st_glloader_quit)
        st_glloader_quit(glloader_ctx);

    return true;
}

static void st_texture_label(const st_texture_t *texture, const char *label) {
    st_texture_opengl_t *module = ((st_modctx_t *)st_object_get_owner(texture))->data;

    if (module->gldebug.ctx)
        module->gldebug.label_texture(module->gldebug.ctx, texture->id, label);
}

static void st_texture_unlabel(const st_texture_t *texture) {
    st_texture_opengl_t *module = ((st_modctx_t *)st_object_get_owner(texture))->data;

    if (module->gldebug.ctx)
        module->gldebug.unlabel_texture(module->gldebug.ctx, texture->id);
}

static st_modctx_t *st_texture_init(st_modctx_t *bitmap_ctx,
 st_modctx_t *logger_ctx, st_gfxctx_t *gfxctx) {
    st_modctx_t         *texture_ctx;
    st_texture_opengl_t *module;

    texture_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_texture_opengl_data, sizeof(st_texture_opengl_t));

    if (!texture_ctx)
        return NULL;

    texture_ctx->funcs = &st_texture_opengl_funcs;

    module = texture_ctx->data;
    module->bitmap.ctx = bitmap_ctx;
    module->logger.ctx = logger_ctx;
    module->gfxctx.handle = gfxctx;

    if (!st_texture_import_functions(texture_ctx, bitmap_ctx, logger_ctx,
     gfxctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, texture_ctx);

        return NULL;
    }

    module->gldebug.ctx = module->gldebug.init(logger_ctx, gfxctx);
    if (!module->gldebug.ctx)
        module->logger.warning(module->logger.ctx,
         "texture_opengl: Unable to initialize gldebug");

    module->logger.info(module->logger.ctx,
     "texture_opengl: Texture mgr initialized.");

    return texture_ctx;
}

static void st_texture_quit(st_modctx_t *texture_ctx) {
    st_texture_opengl_t *module = texture_ctx->data;

    if (module->gldebug.ctx)
        module->gldebug.quit(module->gldebug.ctx);

    module->logger.info(module->logger.ctx,
     "texture_opengl: Texture mgr destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, texture_ctx);
}

static bool glapi_least(st_modctx_t *texture_ctx, st_gapi_t api) {
    st_texture_opengl_t *module = texture_ctx->data;

    if (api < ST_GAPI_GL11 || api > ST_GAPI_GL46)
        return false;

    return module->gfxctx.api >= api;
}

static st_texture_t *st_texture_load_impl(st_modctx_t *texture_ctx,
 const st_bitmap_t *bitmap, const char *name) {
    st_texture_opengl_t *module = texture_ctx->data;
    GLenum               error;
    st_texture_t        *texture = malloc(sizeof(st_texture_t));

    if (!texture) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "texture_opengl: Unable to allocate memory for texture struct "
             "\"%s\": %s", name ? name : "(unnamed)", errbuf);

        return NULL;
    }

    st_object_make(texture, texture_ctx, &texture_funcs);
    texture->width = module->bitmap.get_width(bitmap);
    texture->height = module->bitmap.get_height(bitmap);

    ST_GFXCTX_CALL(module->gfxctx.handle, make_current);
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    st_texture_label(texture, name ? name : "(unnamed)");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    if (glapi_least(texture_ctx, ST_GAPI_GL3) && glGenerateMipmap) {
        float mip_max = log2f(
         ((float)texture->width > (float)texture->height)
          ? (float)texture->width
          : (float)texture->height
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint)mip_max);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)texture->width,
     (GLsizei)texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
     module->bitmap.get_data(bitmap));

    if (glapi_least(texture_ctx, ST_GAPI_GL3) && glGenerateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            module->logger.warning(module->logger.ctx,
             "texture_opengl: Unable to generate mipmap for texture \"%s\": %s",
             name ? name : "(unnamed)",
             module->gldebug.get_error_msg(module->gldebug.ctx, error));

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        }
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    if (glapi_least(texture_ctx, ST_GAPI_GL3) && glGenerateMipmap)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
         GL_NEAREST_MIPMAP_NEAREST);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
         GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

static st_texture_t *st_texture_load(st_modctx_t *texture_ctx,
 const char *filename) {
    st_texture_opengl_t *module = texture_ctx->data;
    st_texture_t        *texture;
    st_bitmap_t         *bitmap = module->bitmap.load(module->bitmap.ctx,
     filename);

    if (!bitmap)
        return NULL;

    texture = st_texture_load_impl(texture_ctx, bitmap, filename);

    module->bitmap.destroy(bitmap);

    return texture;
}

static st_texture_t *st_texture_memload(st_modctx_t *texture_ctx,
 const void *data, size_t size) {
    st_texture_opengl_t *module = texture_ctx->data;
    st_texture_t        *texture;
    st_bitmap_t         *bitmap = module->bitmap.memload(module->bitmap.ctx,
     data, size);

    if (!bitmap)
        return NULL;

    texture = st_texture_load_impl(texture_ctx, bitmap, NULL);

    module->bitmap.destroy(bitmap);

    return texture;
}

static void st_texture_destroy(st_texture_t *texture) {
    st_texture_unlabel(texture);
    glDeleteTextures(1, &texture->id);
    free(texture);
}

static bool st_texture_bind(const st_texture_t *texture, unsigned unit) {
    GLenum error;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    error = glGetError();
    if (error != GL_NO_ERROR) {
        st_texture_opengl_t *module = (
         (st_modctx_t *)st_object_get_owner(texture))->data;

        module->logger.error(module->logger.ctx,
         "texture_opengl: Unable to bind texture: %s",
         module->gldebug.get_error_msg(module->gldebug.ctx, error));

        return false;
    }

    return true;
}

static unsigned st_texture_get_width(const st_texture_t *texture) {
    return texture->width;
}

static unsigned st_texture_get_height(const st_texture_t *texture) {
    return texture->height;
}
