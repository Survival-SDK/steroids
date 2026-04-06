#include "opengl.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/bitmap.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/gldebug.h"
#include "steroids/modules/glloader.h"

#define ERRMSGBUF_SIZE 128

static void (*glGenerateMipmap)(GLenum target);

static st_texturectx_t *st_texture_init(const st_param_t params[]);
static void st_texture_quit(st_texturectx_t *texture_ctx);
static void st_texture_destroy(st_texture_t *texture);

static st_texture_t *st_texture_load(st_texturectx_t *texture_ctx,
 const char *filename);
static st_texture_t *st_texture_memload(st_texturectx_t *texture_ctx,
 const void *data, size_t size);
static bool st_texture_bind(const st_texture_t *texture, unsigned unit);
static unsigned st_texture_get_width(const st_texture_t *texture);
static unsigned st_texture_get_height(const st_texture_t *texture);

static st_texturectx_funcs_t texturectx_funcs = {
    ST_MODCTX_FUNCS,
    .load    = st_texture_load,
    .memload = st_texture_memload,
};

static st_texture_funcs_t texture_funcs = {
    ST_OBJECT_FUNCS,
    .bind       = st_texture_bind,
    .get_width  = st_texture_get_width,
    .get_height = st_texture_get_height,
};

static const st_modprerq_t mod_prereqs[] = {
    { "bitmap", NULL, },
    { "gfxctx", NULL, },
    { "gldebug", "opengl", },
    { "glloader", NULL, },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_texture_opengl_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("texture", "opengl", ST_MODULE_TYPE, mod_prereqs,
     st_texture_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_texture_opengl_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "texture";
static const char *st_module_name = "opengl";

static void st_texture_label(const st_texture_t *texture, const char *label) {
    st_texturectx_t *texture_ctx = (st_texturectx_t *)st_object_get_owner(
     (const st_object_t *)texture);

    if (texture_ctx->gldebug_ctx)
        ST_GLDEBUGCTX_CALL(texture_ctx->gldebug_ctx, label_texture, texture->id,
         label);
}

static void st_texture_unlabel(const st_texture_t *texture) {
    st_texturectx_t *texture_ctx = (st_texturectx_t *)st_object_get_owner(
     (const st_object_t *)texture);

    if (texture_ctx->gldebug_ctx)
        ST_GLDEBUGCTX_CALL(texture_ctx->gldebug_ctx, unlabel_texture,
         texture->id);
}

static bool glapi_least(st_texturectx_t *texture_ctx, st_gapi_t api) {
    if (api < ST_GAPI_GL11 || api > ST_GAPI_GL46)
        return false;

    return texture_ctx->gfxctx_api >= api;
}

static st_texturectx_t *st_texture_init(const st_param_t params[]) {
    st_gfxctx_t      *gfxctx = st_modctx_get_param_as_ptr(params, "gfxctx");
    st_gfxctxctx_t   *gfxctx_ctx = (st_gfxctxctx_t *)st_object_get_owner(
     (st_object_t *)gfxctx);
    st_modsmgr_t     *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_bitmapctx_t   *bitmap_ctx = (st_bitmapctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "bitmap", NULL);
    st_loggerctx_t   *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_glloaderctx_t *glloader_ctx = (st_glloaderctx_t *)ST_MODSMGR_CALL(
     modsmgr, get_singleton, "glloader", 
     ST_GFXCTXCTX_CALL(gfxctx_ctx, get_name));
    st_gldebugctx_t  *gldebug_ctx = (st_gldebugctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "gldebug", "opengl");
    st_texturectx_t  *texture_ctx;

    if (!bitmap_ctx || !logger_ctx || !glloader_ctx || !gldebug_ctx) {
        if (logger_ctx)
            ST_LOGGERCTX_CALL(logger_ctx, error,
             "%s_%s: Unable to get required module contexts",
             st_module_subsystem, st_module_name);
        else
            fprintf(stderr,
             "%s_%s: Unable to get logger context\n", st_module_subsystem,
             st_module_name);

        return NULL;
    }

    if (!gfxctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: gfxctx parameter is required", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    texture_ctx = (st_texturectx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_texturectx_t), NULL, &texturectx_funcs,
     (st_object_dtor_t)st_texture_quit);
    if (!texture_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create texture context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    texture_ctx->bitmap_ctx = bitmap_ctx;
    texture_ctx->logger_ctx = logger_ctx;
    texture_ctx->gldebug_ctx = gldebug_ctx;
    texture_ctx->glloader_ctx = glloader_ctx;
    texture_ctx->gfxctx = gfxctx;
    texture_ctx->gfxctx_api = ST_GFXCTX_CALL(gfxctx, get_api);

    gfxctx_ctx = (st_modctx_t *)st_object_get_owner((st_object_t *)gfxctx);
    
    glGenerateMipmap = (void (*)(GLenum))ST_GLLOADERCTX_CALL(glloader_ctx,
     get_proc_address, "glGenerateMipmap");
    if (!glGenerateMipmap)
        ST_LOGGERCTX_CALL(logger_ctx, warning,
         "%s_%s: Unable to load function \"glGenerateMipmap\". "
         "Mipmaps will not be supported in this OpenGL context",
         st_module_subsystem, st_module_name);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Context initialized", st_module_subsystem, st_module_name);

    return texture_ctx;
}

static void st_texture_quit(st_texturectx_t *texture_ctx) {
    ST_LOGGERCTX_CALL(texture_ctx->logger_ctx, info,
     "%s_%s: Context destroyed", st_module_subsystem, st_module_name);
    free(texture_ctx);
}

static void st_texture_destroy(st_texture_t *texture) {
    st_texture_unlabel(texture);
    glDeleteTextures(1, &texture->id);
    free(texture);
}

static st_texture_t *st_texture_load_impl(st_texturectx_t *texture_ctx,
 const st_bitmap_t *bitmap, const char *name) {
    GLenum        error;
    st_texture_t *texture = (st_texture_t *)st_object_new(sizeof(st_texture_t),
     &texture_funcs, (st_object_dtor_t)st_texture_destroy,
     (st_object_t *)texture_ctx);

    if (!texture) {
        ST_LOGGERCTX_CALL(texture_ctx->logger_ctx, error,
         "%s_%s: Unable to create texture object \"%s\"", st_module_subsystem,
         st_module_name, name ? name : "(unnamed)");

        return NULL;
    }

    texture->width = ST_BITMAP_CALL(bitmap, get_width);
    texture->height = ST_BITMAP_CALL(bitmap, get_height);

    if (!ST_GFXCTX_CALL(texture_ctx->gfxctx, make_current)) {
        ST_LOGGERCTX_CALL(texture_ctx->logger_ctx, error,
         "%s_%s: Unable to make OpenGL context current. Texture creation failed",
         st_module_subsystem, st_module_name);

        free(texture);

        return NULL;
    }

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
     ST_BITMAP_CALL(bitmap, get_data));

    if (glapi_least(texture_ctx, ST_GAPI_GL3) && glGenerateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            ST_LOGGERCTX_CALL(texture_ctx->logger_ctx, warning,
             "%s_%s: Unable to generate mipmap for texture \"%s\": %s",
             st_module_subsystem, st_module_name, name ? name : "(unnamed)",
             ST_GLDEBUGCTX_CALL(texture_ctx->gldebug_ctx, get_error_msg,
              error));

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

static st_texture_t *st_texture_load(st_texturectx_t *texture_ctx,
 const char *filename) {
    st_texture_t *texture;
    st_bitmap_t  *bitmap = ST_BITMAPCTX_CALL(texture_ctx->bitmap_ctx, load,
     filename);

    if (!bitmap)
        return NULL;

    texture = st_texture_load_impl(texture_ctx, bitmap, filename);

    ST_BITMAP_CALL(bitmap, destroy);

    return texture;
}

static st_texture_t *st_texture_memload(st_texturectx_t *texture_ctx,
 const void *data, size_t size) {
    st_texture_t *texture;
    st_bitmap_t  *bitmap = ST_BITMAPCTX_CALL(texture_ctx->bitmap_ctx, memload,
     data, size);

    if (!bitmap)
        return NULL;

    texture = st_texture_load_impl(texture_ctx, bitmap, NULL);

    ST_BITMAP_CALL(bitmap, destroy);

    return texture;
}

static bool st_texture_bind(const st_texture_t *texture, unsigned unit) {
    GLenum error;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    error = glGetError();
    if (error != GL_NO_ERROR) {
        st_texturectx_t *texture_ctx = (st_texturectx_t *)st_object_get_owner(
         (const st_object_t *)texture);

        ST_LOGGERCTX_CALL(texture_ctx->logger_ctx, error,
         "%s_%s: Unable to bind texture: %s", st_module_subsystem,
         st_module_name, ST_GLDEBUGCTX_CALL(texture_ctx->gldebug_ctx,
          get_error_msg, error));

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
