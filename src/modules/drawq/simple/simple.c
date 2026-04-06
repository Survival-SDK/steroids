#include "simple.h"

#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/sprite.h"
#include "steroids/modules/texture.h"

#define ERRMSGBUF_SIZE           1024
#define DYNARR_INITIAL_CAPACITY 16384

static st_drawqctx_t *st_drawq_init(const st_param_t params[]);
static void st_drawq_quit(st_drawqctx_t *drawq_ctx);
static void st_drawq_destroy(st_drawq_t *drawq);

static st_drawq_t *st_drawq_create(st_drawqctx_t *drawq_ctx);
static size_t st_drawq_len(const st_drawq_t *drawq);
static bool st_drawq_empty(const st_drawq_t *drawq);
static bool st_drawq_export_entry(const st_drawq_t *drawq,
 st_drawrec_t *drawrec, size_t index);
static const st_drawrec_t *st_drawq_get_all(const st_drawq_t *drawq);
static bool st_drawq_add(st_drawq_t *drawq, const st_sprite_t *sprite, float x,
 float y, float z, float hscale, float vscale, float angle, float hshear,
 float vshear, float pivot_x, float pivot_y);
static bool st_drawq_sort(st_drawq_t *drawq);
static bool st_drawq_clear(st_drawq_t *drawq);

static st_drawqctx_funcs_t drawqctx_funcs = {
    ST_MODCTX_FUNCS,
    .create = st_drawq_create,
};

static st_drawq_funcs_t drawq_funcs = {
    ST_OBJECT_FUNCS,
    .len          = st_drawq_len,
    .empty        = st_drawq_empty,
    .export_entry = st_drawq_export_entry,
    .get_all      = st_drawq_get_all,
    .add          = st_drawq_add,
    .sort         = st_drawq_sort,
    .clear        = st_drawq_clear,
};

static const st_modprerq_t mod_prereqs[] = {
    { "dynarr", NULL, },
    { "logger", NULL, },
    { "sprite", NULL, },
    {0},
};

st_moddata_t *st_module_drawq_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("drawq", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_drawq_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_drawq_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "drawq";
static const char *st_module_name = "simple";

static st_drawqctx_t *st_drawq_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_dynarrctx_t *dynarr_ctx = (st_dynarrctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "dynarr", NULL);
    st_drawqctx_t  *drawq_ctx;

    if (!logger_ctx) {
        fprintf(stderr,
         "%s_%s: Unable to get logger context\n", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    drawq_ctx = (st_drawqctx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_drawqctx_t), NULL, &drawqctx_funcs,
     (st_object_dtor_t)st_drawq_quit);
    if (!drawq_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create draw queue context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    drawq_ctx->dynarr_ctx = dynarr_ctx;
    drawq_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Draw queues context initialized", st_module_subsystem,
     st_module_name);

    return drawq_ctx;
}

static void st_drawq_quit(st_drawqctx_t *drawq_ctx) {
    ST_LOGGERCTX_CALL(drawq_ctx->logger_ctx, info,
     "%s_%s: Draw queues context destroyed", st_module_subsystem,
     st_module_name);
    free(drawq_ctx);
}

static void st_drawq_destroy(st_drawq_t *drawq) {
    ST_DYNARR_CALL(drawq->entries, destroy);
    free(drawq);
}

static st_drawq_t *st_drawq_create(st_drawqctx_t *drawq_ctx) {
    st_drawq_t *drawq = (st_drawq_t *)st_object_new(sizeof(st_drawq_t),
     &drawq_funcs, (st_object_dtor_t)st_drawq_destroy,
     (st_object_t *)drawq_ctx);

    if (!drawq) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(drawq_ctx->logger_ctx, error,
             "%s_%s: Unable to allocate memory for draw queue: %s",
             st_module_subsystem, st_module_name, errbuf);

        return NULL;
    }

    drawq->entries = ST_DYNARRCTX_CALL(drawq_ctx->dynarr_ctx, create,
     sizeof(st_drawrec_t), DYNARR_INITIAL_CAPACITY);
    if (!drawq->entries) {
        ST_LOGGERCTX_CALL(drawq_ctx->logger_ctx, error,
         "%s_%s: Unable to initialize dynamic array for draw queue",
         st_module_subsystem, st_module_name);
        free(drawq);

        return NULL;
    }

    return drawq;
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
 __attribute__((unused)) void *userdata) {
    const st_drawrec_t *left = leftptr;
    const st_drawrec_t *right = rightptr;
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
    return ST_DYNARR_CALL(drawq->entries, sort, st_drawrec_cmp, NULL);
}

static bool st_drawq_clear(st_drawq_t *drawq) {
    return ST_DYNARR_CALL(drawq->entries, clear);
}
