#include "simple.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/matrix3x3.h"

static st_vec2ctx_t *st_vec2_init(const st_param_t params[]);
static void st_vec2_quit(st_vec2ctx_t *vec2_ctx);

static void st_vec2_add(const st_vec2ctx_t *vec2_ctx, float *vec_x,
 float *vec_y, float add_x, float add_y);
static void st_vec2_sum(const st_vec2ctx_t *vec2_ctx, float *sum_x,
 float *sum_y, float first_x, float first_y, float second_x, float second_y);
static void st_vec2_sub(const st_vec2ctx_t *vec2_ctx, float *vec_x,
 float *vec_y, float sub_x, float sub_y);
static void st_vec2_diff(const st_vec2ctx_t *vec2_ctx, float *diff_x,
 float *diff_y, float first_x, float first_y, float second_x, float second_y);
static void st_vec2_mul(const st_vec2ctx_t *vec2_ctx, float *x, float *y,
 float scalar);
static void st_vec2_product(const st_vec2ctx_t *vec2_ctx, float *product_x,
 float *product_y, float x, float y, float scalar);
static float st_vec2_len(const st_vec2ctx_t *vec2_ctx, float x, float y);
static float st_vec2_distance(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
static void st_vec2_normalize(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y);
static void st_vec2_unit(const st_vec2ctx_t *vec2_ctx, float *unit_x,
 float *unit_y, float x, float y);
static float st_vec2_dot_product(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
static float st_vec2_rangle(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
static float st_vec2_dangle(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
static void st_vec2_rrotate(const st_vec2ctx_t *vec2_ctx, float *x, float *y,
 float radians);
static void st_vec2_rrotation(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float radians);
static void st_vec2_drotate(const st_vec2ctx_t *vec2_ctx, float *x, float *y,
 float degrees);
static void st_vec2_drotation(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float degrees);
static void st_vec2_rotate90(const st_vec2ctx_t *vec2_ctx, float *x, float *y);
static void st_vec2_rotation90(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y);
static void st_vec2_rotate180(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y);
static void st_vec2_rotation180(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y);
static void st_vec2_rotate270(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y);
static void st_vec2_rotation270(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y);
static void st_vec2_apply_matrix3x3(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y, const st_matrix3x3_t *matrix);
static void st_vec2_applying_matrix3x3(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y, float src_x, float src_y,
 const st_matrix3x3_t *matrix);
static void st_vec2_default_basis_xvec(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y);
static void st_vec2_default_basis_yvec(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y);

static st_vec2ctx_funcs_t vec2ctx_funcs = {
    ST_MODCTX_FUNCS,
    .add                = st_vec2_add,
    .sum                = st_vec2_sum,
    .sub                = st_vec2_sub,
    .diff               = st_vec2_diff,
    .mul                = st_vec2_mul,
    .product            = st_vec2_product,
    .len                = st_vec2_len,
    .distance           = st_vec2_distance,
    .normalize          = st_vec2_normalize,
    .unit               = st_vec2_unit,
    .dot_product        = st_vec2_dot_product,
    .rangle             = st_vec2_rangle,
    .dangle             = st_vec2_dangle,
    .rrotate            = st_vec2_rrotate,
    .rrotation          = st_vec2_rrotation,
    .drotate            = st_vec2_drotate,
    .drotation          = st_vec2_drotation,
    .rotate90           = st_vec2_rotate90,
    .rotation90         = st_vec2_rotation90,
    .rotate180          = st_vec2_rotate180,
    .rotation180        = st_vec2_rotation180,
    .rotate270          = st_vec2_rotate270,
    .rotation270        = st_vec2_rotation270,
    .apply_matrix3x3    = st_vec2_apply_matrix3x3,
    .applying_matrix3x3 = st_vec2_applying_matrix3x3,
    .default_basis_xvec = st_vec2_default_basis_xvec,
    .default_basis_yvec = st_vec2_default_basis_yvec,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "angle", NULL, },
    {0},
};

st_moddata_t *st_module_vec2_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("vec2", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_vec2_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_vec2_simple_init(modsmgr);
}
#endif

static st_vec2ctx_t *st_vec2_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_anglectx_t  *angle_ctx = (st_anglectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "angle", NULL);
    st_vec2ctx_t   *vec2_ctx = (st_vec2ctx_t *)st_modctx_new("vec2", "simple",
     sizeof(st_vec2ctx_t), NULL, &vec2ctx_funcs,
     (st_object_dtor_t)st_vec2_quit);

    if (!vec2_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "vec2_simple: unable to create new vec2 ctx object");

        return NULL;
    }

    vec2_ctx->logger_ctx = logger_ctx;
    vec2_ctx->angle_ctx = angle_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "vec2_simple: 2D Vector utilities module context initialized");

    return vec2_ctx;
}

static void st_vec2_quit(st_vec2ctx_t *vec2_ctx) {
    ST_LOGGERCTX_CALL(vec2_ctx->logger_ctx, info,
     "vec2_simple: 2D Vector utilities module context destroyed");
    free(vec2_ctx);
}

static void st_vec2_add(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
 float *vec_x, float *vec_y, float add_x, float add_y) {
    *vec_x += add_x;
    *vec_y += add_y;
}

static void st_vec2_sum(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
 float *sum_x, float *sum_y, float first_x, float first_y, float second_x,
 float second_y) {
    *sum_x = first_x + second_x;
    *sum_y = first_y + second_y;
}

static void st_vec2_sub(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
 float *vec_x, float *vec_y, float sub_x, float sub_y) {
    *vec_x -= sub_x;
    *vec_y -= sub_y;
}

static void st_vec2_diff(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
 float *diff_x, float *diff_y, float first_x, float first_y, float second_x,
 float second_y) {
    *diff_x = first_x - second_x;
    *diff_y = first_y - second_y;
}

static void st_vec2_mul(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
 float *x, float *y, float scalar) {
    *x *= scalar;
    *y *= scalar;
}

static void st_vec2_product(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
 float *product_x, float *product_y, float x, float y, float scalar) {
    *product_x = x * scalar;
    *product_y = y * scalar;
}

static float st_vec2_len(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
 float x, float y) {
    return sqrtf(x * x + y * y);
}

static float st_vec2_distance(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y) {
    float diff_x;
    float diff_y;

    st_vec2_diff(vec2_ctx, &diff_x, &diff_y, first_x, first_y, second_x,
     second_y);
    return st_vec2_len(vec2_ctx, diff_x, diff_y);
}

static void st_vec2_normalize(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y) {
    float len = st_vec2_len(vec2_ctx, *x, *y);

    *x /= len;
    *y /= len;
}

static void st_vec2_unit(const st_vec2ctx_t *vec2_ctx, float *unit_x,
 float *unit_y, float x, float y) {
    float len = st_vec2_len(vec2_ctx, x, y);

    *unit_x = x / len;
    *unit_y = y / len;
}

static float st_vec2_dot_product(
 __attribute__((unused)) const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y) {
    return first_x * second_x + first_y * second_y;
}

static float st_vec2_rangle(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y) {
    float dot_product = st_vec2_dot_product(vec2_ctx, first_x, first_y,
     second_x, second_y);
    float first_len = st_vec2_len(vec2_ctx, first_x, first_y);
    float second_len = st_vec2_len(vec2_ctx, second_x, second_y);

    return ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, rdacos,
     dot_product / (first_len * second_len));
}

static float st_vec2_dangle(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y) {
    return ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, rtod,
     st_vec2_rangle(vec2_ctx, first_x, first_y, second_x, second_y));
}

static void st_vec2_rrotation(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float radians) {
    ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, rnormalize360, &radians);

    *dst_x = (src_x * ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, rdcos, radians)
     - (src_y * ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, rdsin, radians)));
    *dst_y = (src_x * ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, rdsin, radians)
     + (src_y * ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, rdcos, radians)));
}

static void st_vec2_rrotate(const st_vec2ctx_t *vec2_ctx, float *x, float *y,
 float radians) {
    float src_x = *x;
    float src_y = *y;

    st_vec2_rrotation(vec2_ctx, x, y, src_x, src_y, radians);
}

static void st_vec2_drotation(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float degrees) {
    return st_vec2_rrotation(vec2_ctx, dst_x, dst_y, src_x, src_y,
     ST_ANGLECTX_CALL(vec2_ctx->angle_ctx, dtor, degrees));
}

static void st_vec2_drotate(const st_vec2ctx_t *vec2_ctx, float *x, float *y,
 float degrees) {
    float src_x = *x;
    float src_y = *y;

    st_vec2_drotation(vec2_ctx, x, y, src_x, src_y, degrees);
}

static void st_vec2_rotation90(
 __attribute__((unused)) const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y) {
    *dst_x = -src_y;
    *dst_y = src_x;
}

static void st_vec2_rotate90(const st_vec2ctx_t *vec2_ctx, float *x, float *y) {
    float new_x;
    float new_y;

    st_vec2_rotation90(vec2_ctx, &new_x, &new_y, *x, *y);
    *x = new_x;
    *y = new_y;
}

static void st_vec2_rotation180(
 __attribute__((unused)) const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y) {
    *dst_x = -src_x;
    *dst_y = -src_y;
}

static void st_vec2_rotate180(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y) {
    float new_x;
    float new_y;

    st_vec2_rotation180(vec2_ctx, &new_x, &new_y, *x, *y);
    *x = new_x;
    *y = new_y;
}

static void st_vec2_rotation270(
 __attribute__((unused)) const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y) {
    *dst_x = src_y;
    *dst_y = -src_x;
}

static void st_vec2_rotate270(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y) {
    float new_x;
    float new_y;

    st_vec2_rotation270(vec2_ctx, &new_x, &new_y, *x, *y);
    *x = new_x;
    *y = new_y;
}

static void st_vec2_applying_matrix3x3(
 __attribute__((unused)) const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, const st_matrix3x3_t *matrix) {
    *dst_x = matrix->r1c1 * src_x + matrix->r1c2 * src_y + matrix->r1c3;
    *dst_y = matrix->r2c1 * src_x + matrix->r2c2 * src_y + matrix->r2c3;
}

static void st_vec2_apply_matrix3x3(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y, const st_matrix3x3_t *matrix) {
    float new_x;
    float new_y;

    st_vec2_applying_matrix3x3(vec2_ctx, &new_x, &new_y, *x, *y, matrix);
    *x = new_x;
    *y = new_y;
}

static void st_vec2_default_basis_xvec(
 __attribute__((unused)) const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y) {
    *dst_x = 1.0f;
    *dst_y = 0.0f;
}

static void st_vec2_default_basis_yvec(
 __attribute__((unused)) const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y) {
    *dst_x = 0.0f;
    *dst_y = 1.0f;
}
