#include "simple.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_matrix3x3ctx_t *st_matrix3x3_init(const st_param_t params[]);
static void st_matrix3x3_quit(st_matrix3x3ctx_t *matrix3x3_ctx);

static void st_matrix3x3_clone(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *dst, const st_matrix3x3_t *matrix);
static void st_matrix3x3_custom(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float r1c1, float r1c2, float r1c3, float r2c1,
 float r2c2, float r2c3);
static void st_matrix3x3_identity(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix);
static void st_matrix3x3_translation(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float x, float y);
static void st_matrix3x3_scaling(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float hscale, float vscale);
static void st_matrix3x3_rrotation(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
static void st_matrix3x3_drotation(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
static void st_matrix3x3_rhshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
static void st_matrix3x3_dhshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
static void st_matrix3x3_rvshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
static void st_matrix3x3_dvshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
static void st_matrix3x3_apply(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, const st_matrix3x3_t *other);
static void st_matrix3x3_translate(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float x, float y);
static void st_matrix3x3_scale(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float hscale, float vscale);
static void st_matrix3x3_rrotate(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
static void st_matrix3x3_drotate(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
static void st_matrix3x3_rhshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
static void st_matrix3x3_dhshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
static void st_matrix3x3_rvshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
static void st_matrix3x3_dvshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
static void st_matrix3x3_get_data(const st_matrix3x3ctx_t *matrix3x3_ctx,
 const st_matrix3x3_t *matrix, float *r1c1, float *r1c2, float *r1c3,
 float *r2c1, float *r2c2, float *r2c3);

static st_matrix3x3ctx_funcs_t matrix3x3ctx_funcs = {
    ST_MODCTX_FUNCS,
    .clone       = st_matrix3x3_clone,
    .custom      = st_matrix3x3_custom,
    .identity    = st_matrix3x3_identity,
    .translation = st_matrix3x3_translation,
    .scaling     = st_matrix3x3_scaling,
    .rrotation   = st_matrix3x3_rrotation,
    .drotation   = st_matrix3x3_drotation,
    .rhshearing  = st_matrix3x3_rhshearing,
    .dhshearing  = st_matrix3x3_dhshearing,
    .rvshearing  = st_matrix3x3_rvshearing,
    .dvshearing  = st_matrix3x3_dvshearing,
    .apply       = st_matrix3x3_apply,
    .translate   = st_matrix3x3_translate,
    .scale       = st_matrix3x3_scale,
    .rrotate     = st_matrix3x3_rrotate,
    .drotate     = st_matrix3x3_drotate,
    .rhshear     = st_matrix3x3_rhshear,
    .dhshear     = st_matrix3x3_dhshear,
    .rvshear     = st_matrix3x3_rvshear,
    .dvshear     = st_matrix3x3_dvshear,
    .get_data    = st_matrix3x3_get_data,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "angle", NULL, },
    {0},
};

st_moddata_t *st_module_matrix3x3_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("matrix3x3", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_matrix3x3_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_matrix3x3_simple_init(modsmgr);
}
#endif

static st_matrix3x3ctx_t *st_matrix3x3_init(const st_param_t params[]) {
    st_modsmgr_t       *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t     *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_anglectx_t      *angle_ctx = (st_anglectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "angle", NULL);
    st_matrix3x3ctx_t  *matrix3x3_ctx = (st_matrix3x3ctx_t *)st_modctx_new(
     "matrix3x3", "simple", sizeof(st_matrix3x3ctx_t), NULL,
     &matrix3x3ctx_funcs, (st_object_dtor_t)st_matrix3x3_quit);

    if (!matrix3x3_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "matrix3x3_simple: unable to create new matrix3x3 ctx object");

        return NULL;
    }

    matrix3x3_ctx->logger_ctx = logger_ctx;
    matrix3x3_ctx->angle_ctx = angle_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "matrix3x3_simple: Matrix 3x3 utilities module context initialized");

    return matrix3x3_ctx;
}

static void st_matrix3x3_quit(st_matrix3x3ctx_t *matrix3x3_ctx) {
    ST_LOGGERCTX_CALL(matrix3x3_ctx->logger_ctx, info,
     "matrix3x3_simple: Matrix 3x3 utilities module context destroyed");
    free(matrix3x3_ctx);
}

static void st_matrix3x3_clone(
 __attribute__((unused)) const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *dst, const st_matrix3x3_t *matrix) {
    dst->r1c1 = matrix->r1c1;
    dst->r1c2 = matrix->r1c2;
    dst->r1c3 = matrix->r1c3;

    dst->r2c1 = matrix->r2c1;
    dst->r2c2 = matrix->r2c2;
    dst->r2c3 = matrix->r2c3;
}

static void st_matrix3x3_custom(
 __attribute__((unused)) const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float r1c1, float r1c2, float r1c3, float r2c1,
 float r2c2, float r2c3) {
    matrix->r1c1 = r1c1;
    matrix->r1c2 = r1c2;
    matrix->r1c3 = r1c3;

    matrix->r2c1 = r2c1;
    matrix->r2c2 = r2c2;
    matrix->r2c3 = r2c3;
}

static void st_matrix3x3_identity(
 __attribute__((unused)) const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix) {
    matrix->r1c1 = 1;
    matrix->r1c2 = 0;
    matrix->r1c3 = 0;

    matrix->r2c1 = 0;
    matrix->r2c2 = 1;
    matrix->r2c3 = 0;
}

static void st_matrix3x3_translation(
 __attribute__((unused)) const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float x, float y) {
    matrix->r1c1 = 1;
    matrix->r1c2 = 0;
    matrix->r1c3 = x;

    matrix->r2c1 = 0;
    matrix->r2c2 = 1;
    matrix->r2c3 = y;
}

static void st_matrix3x3_scaling(
 __attribute__((unused)) const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float hscale, float vscale) {
    matrix->r1c1 = hscale;
    matrix->r1c2 = 0;
    matrix->r1c3 = 0;

    matrix->r2c1 = 0;
    matrix->r2c2 = vscale;
    matrix->r2c3 = 0;
}

static void st_matrix3x3_rrotation(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians) {
    ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rnormalize360, &radians);

    matrix->r1c1 = ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rdcos, -radians);
    matrix->r1c2 = -ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rdsin, -radians);
    matrix->r1c3 = 0;

    matrix->r2c1 = ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rdsin, -radians);
    matrix->r2c2 = ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rdcos, -radians);
    matrix->r2c3 = 0;
}

static void st_matrix3x3_drotation(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees) {
    st_matrix3x3_rrotation(matrix3x3_ctx, matrix,
     ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, dtor, degrees));
}

static void st_matrix3x3_rhshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians) {
    ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rnormalize360, &radians);

    matrix->r1c1 = 1;
    matrix->r1c2 = ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rdtan, radians);
    matrix->r1c3 = 0;

    matrix->r2c1 = 0;
    matrix->r2c2 = 1;
    matrix->r2c3 = 0;
}

static void st_matrix3x3_dhshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees) {
    st_matrix3x3_rhshearing(matrix3x3_ctx, matrix,
     ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, dtor, degrees));
}

static void st_matrix3x3_rvshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians) {
    ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rnormalize360, &radians);

    matrix->r1c1 = 1;
    matrix->r1c2 = 0;
    matrix->r1c3 = 0;

    matrix->r2c1 = ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, rdtan, radians);
    matrix->r2c2 = 1;
    matrix->r2c3 = 0;
}

static void st_matrix3x3_dvshearing(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees) {
    st_matrix3x3_rvshearing(matrix3x3_ctx, matrix,
     ST_ANGLECTX_CALL(matrix3x3_ctx->angle_ctx, dtor, degrees));
}

static void st_matrix3x3_apply(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, const st_matrix3x3_t *other) {
    st_matrix3x3_t old;

    st_matrix3x3_clone(matrix3x3_ctx, &old, matrix);

    matrix->r1c1 = old.r1c1 * other->r1c1
                 + old.r1c2 * other->r2c1;
    matrix->r1c2 = old.r1c1 * other->r1c2
                 + old.r1c2 * other->r2c2;
    matrix->r1c3 = old.r1c1 * other->r1c3
                 + old.r1c2 * other->r2c3
                 + old.r1c3;

    matrix->r2c1 = old.r2c1 * other->r1c1
                 + old.r2c2 * other->r2c1;
    matrix->r2c2 = old.r2c1 * other->r1c2
                 + old.r2c2 * other->r2c2;
    matrix->r2c3 = old.r2c1 * other->r1c3
                 + old.r2c2 * other->r2c3
                 + old.r2c3;
}

static void st_matrix3x3_translate(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float x, float y) {
    st_matrix3x3_t other;

    st_matrix3x3_translation(matrix3x3_ctx, &other, x, y);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_scale(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float hscale, float vscale) {
    st_matrix3x3_t other;

    st_matrix3x3_scaling(matrix3x3_ctx, &other, hscale, vscale);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_rrotate(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians) {
    st_matrix3x3_t other;

    st_matrix3x3_rrotation(matrix3x3_ctx, &other, radians);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_drotate(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees) {
    st_matrix3x3_t other;

    st_matrix3x3_drotation(matrix3x3_ctx, &other, degrees);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_rhshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians) {
    st_matrix3x3_t other;

    st_matrix3x3_rhshearing(matrix3x3_ctx, &other, radians);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_dhshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees) {
    st_matrix3x3_t other;

    st_matrix3x3_dhshearing(matrix3x3_ctx, &other, degrees);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_rvshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians) {
    st_matrix3x3_t other;

    st_matrix3x3_rvshearing(matrix3x3_ctx, &other, radians);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_dvshear(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees) {
    st_matrix3x3_t other;

    st_matrix3x3_dvshearing(matrix3x3_ctx, &other, degrees);
    st_matrix3x3_apply(matrix3x3_ctx, matrix, &other);
}

static void st_matrix3x3_get_data(
 __attribute__((unused)) const st_matrix3x3ctx_t *matrix3x3_ctx,
 const st_matrix3x3_t *matrix, float *r1c1, float *r1c2, float *r1c3,
 float *r2c1, float *r2c2, float *r2c3) {
    if (r1c1)
        *r1c1 = matrix->r1c1;
    if (r1c2)
        *r1c2 = matrix->r1c2;
    if (r1c3)
        *r1c3 = matrix->r1c3;
    if (r2c1)
        *r2c1 = matrix->r2c1;
    if (r2c2)
        *r2c2 = matrix->r2c2;
    if (r2c3)
        *r2c3 = matrix->r2c3;
}
