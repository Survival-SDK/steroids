#include "simple.h"

#include <errno.h>
#include <stdio.h>

#include "steroids/modules/atlas.h"
#include "steroids/object.h"

#define ERRMSGBUF_SIZE 128

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_sprite_funcs_t sprite_funcs = {
    .destroy     = st_sprite_destroy,
    .get_texture = st_sprite_get_texture,
    .get_width   = st_sprite_get_width,
    .get_height  = st_sprite_get_height,
    .export_uv   = st_sprite_export_uv,
};

ST_MODULE_DEF_GET_FUNC(sprite_simple)
ST_MODULE_DEF_INIT_FUNC(sprite_simple)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_sprite_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_sprite_import_functions(st_modctx_t *sprite_ctx,
 st_modctx_t *logger_ctx) {
    st_sprite_simple_t *module = sprite_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "sprite_simple: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("sprite_simple", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("sprite_simple", logger, info);

    return true;
}

static st_modctx_t *st_sprite_init(st_modctx_t *logger_ctx) {
    st_modctx_t        *sprite_ctx;
    st_sprite_simple_t *module;

    sprite_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_sprite_simple_data, sizeof(st_sprite_simple_t));

    if (!sprite_ctx)
        return NULL;

    sprite_ctx->funcs = &st_sprite_simple_funcs;

    module = sprite_ctx->data;
    module->logger.ctx = logger_ctx;

    if (!st_sprite_import_functions(sprite_ctx, logger_ctx))
        goto fail;

    module->logger.info(module->logger.ctx,
     "sprite_simple: Sprites mgr initialized");

    return sprite_ctx;

fail:
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, sprite_ctx);

    return NULL;
}

static void st_sprite_quit(st_modctx_t *sprite_ctx) {
    st_sprite_simple_t *module = sprite_ctx->data;

    module->logger.info(module->logger.ctx,
     "sprite_simple: Sprites mgr destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, sprite_ctx);
}

static st_sprite_t *st_sprite_create(st_modctx_t *sprite_ctx,
 const st_atlas_t *atlas, size_t clip_num) {
    st_sprite_simple_t *module = sprite_ctx->data;
    unsigned            texture_width;
    unsigned            texture_height;
    unsigned            clip_x;
    unsigned            clip_y;
    st_sprite_t        *sprite = malloc(sizeof(st_sprite_t));

    if (!sprite) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "sprite_simple: Unable to allocate memory for new sprite: %s",
             errbuf);

        return NULL;
    }

    st_object_make(sprite, sprite_ctx, &sprite_funcs);
    sprite->texture = ST_ATLAS_CALL(atlas, get_texture);

    texture_width = ST_TEXTURE_CALL(sprite->texture, get_width);
    texture_height = ST_TEXTURE_CALL(sprite->texture, get_height);
    clip_x = ST_ATLAS_CALL(atlas, get_clip_x, clip_num);
    clip_y = ST_ATLAS_CALL(atlas, get_clip_y, clip_num);

    sprite->width = ST_ATLAS_CALL(atlas, get_clip_width, clip_num);
    sprite->height = ST_ATLAS_CALL(atlas, get_clip_height, clip_num);

    sprite->uv.upper_left.u = (float)clip_x / (float)texture_width;
    sprite->uv.upper_left.v = (float)clip_y / (float)texture_height;
    sprite->uv.upper_right.u = (float)(clip_x + sprite->width) /
     (float)texture_width;
    sprite->uv.upper_right.v = (float)clip_y / (float)texture_height;
    sprite->uv.lower_left.u = (float)clip_x / (float)texture_width;
    sprite->uv.lower_left.v = (float)(clip_y + sprite->height) /
     (float)texture_height;
    sprite->uv.lower_right.u = (float)(clip_x + sprite->width) /
     (float)texture_width;
    sprite->uv.lower_right.v = (float)(clip_y + sprite->height) /
     (float)texture_height;

    return sprite;
}

static st_sprite_t *st_sprite_from_texture(st_modctx_t *sprite_ctx,
 const st_texture_t *texture) {
    st_sprite_simple_t *module = sprite_ctx->data;
    st_sprite_t        *sprite = malloc(sizeof(st_sprite_t));

    if (!sprite) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "sprite_simple: Unable to allocate memory for new sprite: %s",
             errbuf);

        return NULL;
    }

    st_object_make(sprite, sprite_ctx, &sprite_funcs);
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

static void st_sprite_destroy(st_sprite_t *sprite) {
    free(sprite);
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
