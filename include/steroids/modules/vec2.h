#pragma once

#include "steroids/module.h"
#include "steroids/modules/matrix3x3.h"

typedef st_modctx_t *(*st_vec2_init_t)(st_modctx_t *logger_ctx,
 st_modctx_t *angle_ctx);
typedef void (*st_vec2_quit_t)(st_modctx_t *vec2_ctx);

typedef void (*st_vec2_add_t)(st_modctx_t *vec2_ctx, float *vec_x,
 float *vec_y, float add_x, float add_y);
typedef void (*st_vec2_sum_t)(st_modctx_t *vec2_ctx, float *sum_x,
 float *sum_y, float first_x, float first_y, float second_x, float second_y);
typedef void (*st_vec2_sub_t)(st_modctx_t *vec2_ctx, float *vec_x,
 float *vec_y, float sub_x, float sub_y);
typedef void (*st_vec2_diff_t)(st_modctx_t *vec2_ctx, float *diff_x,
 float *diff_y, float first_x, float first_y, float second_x, float second_y);
typedef void (*st_vec2_mul_t)(st_modctx_t *vec2_ctx, float *x, float *y,
 float scalar);
typedef void (*st_vec2_product_t)(st_modctx_t *vec2_ctx, float *product_x,
 float *product_y, float x, float y, float scalar);
typedef float (*st_vec2_len_t)(st_modctx_t *vec2_ctx, float x, float y);
typedef float (*st_vec2_distance_t)(st_modctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
typedef void (*st_vec2_normalize_t)(st_modctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_unit_t)(st_modctx_t *vec2_ctx, float *unit_x,
 float *unit_y, float x, float y);
typedef float (*st_vec2_dot_product_t)(st_modctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
typedef float (*st_vec2_rangle_t)(st_modctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
typedef float (*st_vec2_dangle_t)(st_modctx_t *vec2_ctx, float first_x,
 float first_y, float second_x, float second_y);
typedef void (*st_vec2_rrotate_t)(st_modctx_t *vec2_ctx, float *x, float *y,
 float radians);
typedef void (*st_vec2_rrotation_t)(st_modctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float radians);
typedef void (*st_vec2_drotate_t)(st_modctx_t *vec2_ctx, float *x, float *y,
 float degrees);
typedef void (*st_vec2_drotation_t)(st_modctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y, float degrees);
typedef void (*st_vec2_rotate90_t)(st_modctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_rotation90_t)(st_modctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y);
typedef void (*st_vec2_rotate180_t)(st_modctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_rotation180_t)(st_modctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y);
typedef void (*st_vec2_rotate270_t)(st_modctx_t *vec2_ctx, float *x,
 float *y);
typedef void (*st_vec2_rotation270_t)(st_modctx_t *vec2_ctx, float *dst_x,
 float *dst_y, float src_x, float src_y);
typedef void (*st_vec2_apply_matrix3x3_t)(st_modctx_t *vec2_ctx, float *x,
 float *y, const st_matrix3x3_t *matrix);
typedef void (*st_vec2_applying_matrix3x3_t)(st_modctx_t *vec2_ctx,
 float *dst_x, float *dst_y, float src_x, float src_y,
 const st_matrix3x3_t *matrix);
typedef void (*st_vec2_default_basis_xvec_t)(st_modctx_t *vec2_ctx,
 float *dst_x, float *dst_y);
typedef void (*st_vec2_default_basis_yvec_t)(st_modctx_t *vec2_ctx,
 float *dst_x, float *dst_y);

typedef struct {
    st_vec2_init_t               vec2_init;
    st_vec2_quit_t               vec2_quit;
    st_vec2_add_t                vec2_add;
    st_vec2_sum_t                vec2_sum;
    st_vec2_sub_t                vec2_sub;
    st_vec2_diff_t               vec2_diff;
    st_vec2_mul_t                vec2_mul;
    st_vec2_product_t            vec2_product;
    st_vec2_len_t                vec2_len;
    st_vec2_distance_t           vec2_distance;
    st_vec2_normalize_t          vec2_normalize;
    st_vec2_unit_t               vec2_unit;
    st_vec2_dot_product_t        vec2_dot_product;
    st_vec2_rangle_t             vec2_rangle;
    st_vec2_dangle_t             vec2_dangle;
    st_vec2_rrotate_t            vec2_rrotate;
    st_vec2_rrotation_t          vec2_rrotation;
    st_vec2_drotate_t            vec2_drotate;
    st_vec2_drotation_t          vec2_drotation;
    st_vec2_rotate90_t           vec2_rotate90;
    st_vec2_rotation90_t         vec2_rotation90;
    st_vec2_rotate180_t          vec2_rotate180;
    st_vec2_rotation180_t        vec2_rotation180;
    st_vec2_rotate270_t          vec2_rotate270;
    st_vec2_rotation270_t        vec2_rotation270;
    st_vec2_apply_matrix3x3_t    vec2_apply_matrix3x3;
    st_vec2_applying_matrix3x3_t vec2_applying_matrix3x3;
    st_vec2_default_basis_xvec_t vec2_default_basis_xvec;
    st_vec2_default_basis_yvec_t vec2_default_basis_yvec;
} st_vec2_funcs_t;
