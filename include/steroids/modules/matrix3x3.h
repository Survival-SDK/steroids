#pragma once

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_MATRIX3X3CTX_T_DEFINED
    typedef st_modctx_t st_matrix3x3ctx_t;
#endif

typedef struct {
    float r1c1, r1c2, r1c3;
    float r2c1, r2c2, r2c3;
    /*       0,    0,    1 */
} st_matrix3x3_t;

typedef void (*st_matrix3x3_clone_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *dst, const st_matrix3x3_t *matrix);
typedef void (*st_matrix3x3_custom_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float r1c1, float r1c2, float r1c3, float r2c1,
 float r2c2, float r2c3);
typedef void (*st_matrix3x3_identity_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix);
typedef void (*st_matrix3x3_translation_t)(
 const st_matrix3x3ctx_t *matrix3x3_ctx, st_matrix3x3_t *matrix, float x,
 float y);
typedef void (*st_matrix3x3_scaling_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float hscale, float vscale);
typedef void (*st_matrix3x3_rrotation_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_drotation_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rhshearing_t)(
 const st_matrix3x3ctx_t *matrix3x3_ctx, st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dhshearing_t)(
 const st_matrix3x3ctx_t *matrix3x3_ctx, st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rvshearing_t)(
 const st_matrix3x3ctx_t *matrix3x3_ctx, st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dvshearing_t)(
 const st_matrix3x3ctx_t *matrix3x3_ctx, st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_apply_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, const st_matrix3x3_t *other);
typedef void (*st_matrix3x3_translate_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float x, float y);
typedef void (*st_matrix3x3_scale_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float hscale, float vscale);
typedef void (*st_matrix3x3_rrotate_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_drotate_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rhshear_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dhshear_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rvshear_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dvshear_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_get_data_t)(const st_matrix3x3ctx_t *matrix3x3_ctx,
 const st_matrix3x3_t *matrix, float *r1c1, float *r1c2, float *r1c3,
 float *r2c1, float *r2c2, float *r2c3);

typedef struct {
    st_modctx_funcs_t;
    st_matrix3x3_clone_t       clone;
    st_matrix3x3_custom_t      custom;
    st_matrix3x3_identity_t    identity;
    st_matrix3x3_translation_t translation;
    st_matrix3x3_scaling_t     scaling;
    st_matrix3x3_rrotation_t   rrotation;
    st_matrix3x3_drotation_t   drotation;
    st_matrix3x3_rhshearing_t  rhshearing;
    st_matrix3x3_dhshearing_t  dhshearing;
    st_matrix3x3_rvshearing_t  rvshearing;
    st_matrix3x3_dvshearing_t  dvshearing;
    st_matrix3x3_apply_t       apply;
    st_matrix3x3_translate_t   translate;
    st_matrix3x3_scale_t       scale;
    st_matrix3x3_rrotate_t     rrotate;
    st_matrix3x3_drotate_t     drotate;
    st_matrix3x3_rhshear_t     rhshear;
    st_matrix3x3_dhshear_t     dhshear;
    st_matrix3x3_rvshear_t     rvshear;
    st_matrix3x3_dvshear_t     dvshear;
    st_matrix3x3_get_data_t    get_data;
} st_matrix3x3ctx_funcs_t;

#define ST_MATRIX3X3CTX_CALL(ctx, func, ...) \
    ((st_matrix3x3ctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
