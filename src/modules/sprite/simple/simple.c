#include "simple.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ERRMSGBUF_SIZE 128

static st_spritectx_t *st_sprite_init(const st_param_t params[]);
static void st_sprite_quit(st_spritectx_t *sprite_ctx);
static void st_sprite_destroy(st_sprite_t *sprite);

static st_sprite_t *st_sprite_from_texture(st_spritectx_t *sprite_ctx,
 const st_texture_t *texture);
static const st_texture_t *st_sprite_get_texture(const st_sprite_t *sprite);
static unsigned st_sprite_get_width(const st_sprite_t *sprite);
static unsigned st_sprite_get_height(const st_sprite_t *sprite);
static void st_sprite_export_uv(const st_sprite_t *sprite, st_uv_t *dstuv);

static st_spritectx_funcs_t spritectx_funcs = {
    st_modctx_funcs,
    .from_texture = st_sprite_from_texture,
};

static st_sprite_funcs_t sprite_funcs = {
    st_object_funcs,
    .get_texture = st_sprite_get_texture,
    .get_width   = st_sprite_get_width,
    .get_height  = st_sprite_get_height,
    .export_uv   = st_sprite_export_uv,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "texture", NULL, },
    {0},
};

st_moddata_t *st_module_sprite_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("sprite", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_sprite_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_sprite_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "sprite";
static const char *st_module_name = "simple";

static st_spritectx_t *st_sprite_init(const st_param_t params[]) {
    st_modsmgr_t    *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t  *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_spritectx_t  *sprite_ctx;

    if (!logger_ctx) {
        fprintf(stderr,
         "%s_%s: Unable to get logger context\n", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    sprite_ctx = (st_spritectx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_spritectx_t), NULL, &spritectx_funcs,
     (st_object_dtor_t)st_sprite_quit);
    if (!sprite_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create sprite context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    sprite_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Context initialized", st_module_subsystem, st_module_name);

    return sprite_ctx;
}

static void st_sprite_quit(st_spritectx_t *sprite_ctx) {
    ST_LOGGERCTX_CALL(sprite_ctx->logger_ctx, info,
     "%s_%s: Context destroyed", st_module_subsystem, st_module_name);
    free(sprite_ctx);
}

static void st_sprite_destroy(st_sprite_t *sprite) {
    free(sprite);
}

static st_sprite_t *st_sprite_from_texture(st_spritectx_t *sprite_ctx,
 const st_texture_t *texture) {
    st_sprite_t *sprite = (st_sprite_t *)st_object_new(sizeof(st_sprite_t),
     &sprite_funcs, (st_object_dtor_t)st_sprite_destroy,
     (st_object_t *)sprite_ctx);

    if (!sprite) {
        ST_LOGGERCTX_CALL(sprite_ctx->logger_ctx, error,
         "%s_%s: Unable to create sprite object", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    sprite->texture = texture;
    sprite->width = ST_TEXTURE_CALL(texture, get_width);
    sprite->height = ST_TEXTURE_CALL(texture, get_height);

    sprite->uv.upper_left.u  = 0.0f;
    sprite->uv.upper_left.v  = 0.0f;
    sprite->uv.upper_right.u = 1.0f;
    sprite->uv.upper_right.v = 0.0f;
    sprite->uv.lower_left.u  = 0.0f;
    sprite->uv.lower_left.v  = 1.0f;
    sprite->uv.lower_right.u = 1.0f;
    sprite->uv.lower_right.v = 1.0f;

    return sprite;
}

static const st_texture_t *st_sprite_get_texture(const st_sprite_t *sprite) {
    return sprite->texture;
}

static unsigned st_sprite_get_width(const st_sprite_t *sprite) {
    return sprite->width;
}

static unsigned st_sprite_get_height(const st_sprite_t *sprite) {
    return sprite->height;
}

static void st_sprite_export_uv(const st_sprite_t *sprite, st_uv_t *dstuv) {
    *dstuv = sprite->uv;
}
