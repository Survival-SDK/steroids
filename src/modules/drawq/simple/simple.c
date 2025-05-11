#include "simple.h"

#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "steroids/modules/sprite.h"

#define ERRMSGBUF_SIZE           1024
#define DYNARR_INITIAL_CAPACITY 16384

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_drawq_funcs_t drawq_funcs = {
    .destroy      = st_drawq_destroy,
    .len          = st_drawq_len,
    .empty        = st_drawq_empty,
    .export_entry = st_drawq_export_entry,
    .get_all      = st_drawq_get_all,
    .add          = st_drawq_add,
    .sort         = st_drawq_sort,
    .clear        = st_drawq_clear,
};

ST_MODULE_DEF_GET_FUNC(drawq_simple)
ST_MODULE_DEF_INIT_FUNC(drawq_simple)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_drawq_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_drawq_import_functions(st_modctx_t *drawq_ctx,
 st_modctx_t *dynarr_ctx, st_modctx_t *logger_ctx) {
    st_drawq_simple_t *module = drawq_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "drawq_simple: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("drawq_simple", dynarr, create);

    ST_LOAD_FUNCTION_FROM_CTX("drawq_simple", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("drawq_simple", logger, info);

    return true;
}

static st_modctx_t *st_drawq_init(st_modctx_t *dynarr_ctx,
 st_modctx_t *logger_ctx, st_modctx_t *sprite_ctx) {
    st_modctx_t       *drawq_ctx;
    st_drawq_simple_t *module;

    drawq_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_drawq_simple_data, sizeof(st_drawq_simple_t));

    if (!drawq_ctx)
        return NULL;

    module = drawq_ctx->data;

    drawq_ctx->funcs = &st_drawq_simple_funcs;

    module->dynarr.ctx = dynarr_ctx;
    module->logger.ctx = logger_ctx;
    module->sprite.ctx = sprite_ctx;

    if (!st_drawq_import_functions(drawq_ctx, dynarr_ctx, logger_ctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, drawq_ctx);

        return NULL;
    }

    module->logger.info(module->logger.ctx,
     "drawq_simple: Draw queues mgr initialized");

    return drawq_ctx;
}

static void st_drawq_quit(st_modctx_t *drawq_ctx) {
    st_drawq_simple_t *module = drawq_ctx->data;

    module->logger.info(module->logger.ctx,
     "drawq_simple: Draw queues mgr destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, drawq_ctx);
}

static st_drawq_t *st_drawq_create(st_modctx_t *drawq_ctx) {
    st_drawq_simple_t *module = drawq_ctx->data;
    st_drawq_t        *drawq = malloc(sizeof(st_drawq_t));

    if (!drawq) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "drawq_simple: Unable to allocate memory for draw queue: %s",
             errbuf);

        return NULL;
    }

    drawq->entries = module->dynarr.create(module->dynarr.ctx,
     sizeof(st_drawrec_t), DYNARR_INITIAL_CAPACITY);
    if (!drawq->entries) {
        module->logger.error(module->logger.ctx,
         "drawq_simple: Unable to initialize dynamic array for draw queue");
        free(drawq);

        return NULL;
    }

    st_object_make(drawq, drawq_ctx, &drawq_funcs);

    return drawq;
}

static void st_drawq_destroy(st_drawq_t *drawq) {
    ST_DYNARR_CALL(drawq->entries, destroy);
    free(drawq);
}

static size_t st_drawq_len(const st_drawq_t *drawq) {
    return ST_DYNARR_CALL(drawq->entries, get_elements_count);
}

static bool st_drawq_empty(const st_drawq_t *drawq) {
    return ST_DYNARR_CALL(drawq->entries, is_empty);
}

static bool st_drawq_export_entry(const st_drawq_t *drawq,
 st_drawrec_t *drawrec, size_t index) {
    return ST_DYNARR_CALL(drawq->entries, extract, drawrec, index);
}

static const st_drawrec_t *st_drawq_get_all(const st_drawq_t *drawq) {
    return ST_DYNARR_CALL(drawq->entries, get_all);
}

static bool st_drawq_add(st_drawq_t *drawq, const st_sprite_t *sprite, float x,
 float y, float z, float hscale, float vscale, float angle, float hshear,
 float vshear, float pivot_x, float pivot_y) {
    return ST_DYNARR_CALL(drawq->entries, append, &(st_drawrec_t){
        .sprite  = sprite,
        .x       = x,
        .y       = y,
        .z       = z,
        .hscale  = hscale,
        .vscale  = vscale,
        .angle   = angle,
        .hshear  = hshear,
        .vshear  = vshear,
        .pivot_x = pivot_x,
        .pivot_y = pivot_y,
    });
}

static int st_drawrec_cmp(const void *leftptr, const void *rightptr,
 void *userdata) {
    const st_drawrec_t *left = leftptr;
    const st_drawrec_t *right = rightptr;
    st_drawq_simple_t  *module = userdata;

    const st_texture_t *left_tex = ST_SPRITE_CALL(left->sprite, get_texture);
    const st_texture_t *right_tex = ST_SPRITE_CALL(right->sprite, get_texture);

    if (fabsf(left->z - right->z) <= FLT_EPSILON) {
        if (left_tex == right_tex)
            return 0;

        return (left_tex < right_tex) ? -1 : 1;
    }

    return (left->z > right->z) ? -1 : 1;
}

static bool st_drawq_sort(st_drawq_t *drawq) {
    st_drawq_simple_t *module = ((st_modctx_t *)st_object_get_owner(drawq))->data;

    return ST_DYNARR_CALL(drawq->entries, sort, st_drawrec_cmp, module);
}

static bool st_drawq_clear(st_drawq_t *drawq) {
    return ST_DYNARR_CALL(drawq->entries, clear);
}
