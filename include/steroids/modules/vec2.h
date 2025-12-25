#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/matrix3x3.h"

#ifndef ST_VEC2CTX_T_DEFINED
    typedef st_modctx_t st_vec2ctx_t;
#endif

typedef void (*st_vec2_add_t)(const st_vec2ctx_t *vec2_ctx, float *vec_x,
 float *vec_y, float add_x, float add_y);
typedef void (*st_vec2_sum_t)(const st_vec2ctx_t *vec2_ctx, float *sum_x,
 float *sum_y, float first_x, float first_y, float second_x, float second_y);
typedef void (*st_vec2_sub_t)(const st_vec2ctx_t *vec2_ctx, float *vec_x,
 float *vec_y, float sub_x, float sub_y);
typedef void (*st_vec2_diff_t)(const st_vec2ctx_t *vec2_ctx, float *diff_x,
 float *diff_y, float first_x, float first_y, float second_x, float second_y);
typedef void (*st_vec2_mul_t)(const st_vec2ctx_t *vec2_ctx, float *x, float *y,
 float scalar);
typedef void (*st_vec2_product_t)(const st_vec2ctx_t *vec2_ctx,
 float *product_x, float *product_y, float x, float y, float scalar);
typedef float (*st_vec2_len_t)(const st_vec2ctx_t *vec2_ctx, float x, float y);
typedef float (*st_vec2_distance_t)(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
typedef void (*st_vec2_normalize_t)(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_unit_t)(const st_vec2ctx_t *vec2_ctx, float *unit_x,
 float *unit_y, float x, float y);
typedef float (*st_vec2_dot_product_t)(const st_vec2ctx_t *vec2_ctx,
 float first_x, float first_y, float second_x, float second_y);
typedef float (*st_vec2_rangle_t)(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
typedef float (*st_vec2_dangle_t)(const st_vec2ctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
typedef void (*st_vec2_rrotate_t)(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y, float radians);
typedef void (*st_vec2_rrotation_t)(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float radians);
typedef void (*st_vec2_drotate_t)(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y, float degrees);
typedef void (*st_vec2_drotation_t)(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float degrees);
typedef void (*st_vec2_rotate90_t)(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_rotation90_t)(const st_vec2ctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y);
typedef void (*st_vec2_rotate180_t)(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_rotation180_t)(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y, float src_x, float src_y);
typedef void (*st_vec2_rotate270_t)(const st_vec2ctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_rotation270_t)(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y, float src_x, float src_y);
typedef void (*st_vec2_apply_matrix3x3_t)(const st_vec2ctx_t *vec2_ctx,
 float *x, float *y, const st_matrix3x3_t *matrix);
typedef void (*st_vec2_applying_matrix3x3_t)(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y, float src_x, float src_y,
 const st_matrix3x3_t *matrix);
typedef void (*st_vec2_default_basis_xvec_t)(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y);
typedef void (*st_vec2_default_basis_yvec_t)(const st_vec2ctx_t *vec2_ctx,
 float *dst_x, float *dst_y);

typedef struct {
    st_modctx_funcs_t;
    st_vec2_add_t                add;
    st_vec2_sum_t                sum;
    st_vec2_sub_t                sub;
    st_vec2_diff_t               diff;
    st_vec2_mul_t                mul;
    st_vec2_product_t            product;
    st_vec2_len_t                len;
    st_vec2_distance_t           distance;
    st_vec2_normalize_t          normalize;
    st_vec2_unit_t               unit;
    st_vec2_dot_product_t        dot_product;
    st_vec2_rangle_t             rangle;
    st_vec2_dangle_t             dangle;
    st_vec2_rrotate_t            rrotate;
    st_vec2_rrotation_t          rrotation;
    st_vec2_drotate_t            drotate;
    st_vec2_drotation_t          drotation;
    st_vec2_rotate90_t           rotate90;
    st_vec2_rotation90_t         rotation90;
    st_vec2_rotate180_t          rotate180;
    st_vec2_rotation180_t        rotation180;
    st_vec2_rotate270_t          rotate270;
    st_vec2_rotation270_t        rotation270;
    st_vec2_apply_matrix3x3_t    apply_matrix3x3;
    st_vec2_applying_matrix3x3_t applying_matrix3x3;
    st_vec2_default_basis_xvec_t default_basis_xvec;
    st_vec2_default_basis_yvec_t default_basis_yvec;
} st_vec2ctx_funcs_t;

#define ST_VEC2CTX_CALL(ctx, func, ...) \
    ((st_vec2ctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
