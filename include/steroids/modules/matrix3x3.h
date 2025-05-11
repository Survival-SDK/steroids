#pragma once

#include "steroids/module.h"
#include "steroids/object.h"

#ifndef ST_MATRIX3X3_T_DEFINED
    ST_CLASS (
        float _private[6];
    ) st_matrix3x3_t;
#endif

typedef st_modctx_t *(*st_matrix3x3_init_t)(st_modctx_t *logger_ctx,
 st_modctx_t *angle_ctx);
typedef void (*st_matrix3x3_quit_t)(st_modctx_t *matrix3x3_ctx);

typedef void (*st_matrix3x3_clone_t)(const st_matrix3x3_t *matrix,
 st_matrix3x3_t *dst);
typedef void (*st_matrix3x3_custom_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float r1c1, float r1c2, float r1c3, float r2c1,
 float r2c2, float r2c3);
typedef void (*st_matrix3x3_identity_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix);
typedef void (*st_matrix3x3_translation_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float x, float y);
typedef void (*st_matrix3x3_scaling_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float hscale, float vscale);
typedef void (*st_matrix3x3_rrotation_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_drotation_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rhshearing_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dhshearing_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rvshearing_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dvshearing_t)(st_modctx_t *matrix3x3_ctx,
 st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_apply_t)(st_matrix3x3_t *matrix,
 const st_matrix3x3_t *other);
typedef void (*st_matrix3x3_translate_t)(st_matrix3x3_t *matrix, float x,
 float y);
typedef void (*st_matrix3x3_scale_t)(st_matrix3x3_t *matrix, float hscale,
 float vscale);
typedef void (*st_matrix3x3_rrotate_t)(st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_drotate_t)(st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rhshear_t)(st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dhshear_t)(st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_rvshear_t)(st_matrix3x3_t *matrix, float radians);
typedef void (*st_matrix3x3_dvshear_t)(st_matrix3x3_t *matrix, float degrees);
typedef void (*st_matrix3x3_get_data_t)(const st_matrix3x3_t *matrix,
 float *r1c1, float *r1c2, float *r1c3, float *r2c1, float *r2c2, float *r2c3);

typedef struct {
    st_matrix3x3_init_t        matrix3x3_init;
    st_matrix3x3_quit_t        matrix3x3_quit;
    st_matrix3x3_custom_t      matrix3x3_custom;
    st_matrix3x3_identity_t    matrix3x3_identity;
    st_matrix3x3_translation_t matrix3x3_translation;
    st_matrix3x3_scaling_t     matrix3x3_scaling;
    st_matrix3x3_rrotation_t   matrix3x3_rrotation;
    st_matrix3x3_drotation_t   matrix3x3_drotation;
    st_matrix3x3_rhshearing_t  matrix3x3_rhshearing;
    st_matrix3x3_dhshearing_t  matrix3x3_dhshearing;
    st_matrix3x3_rvshearing_t  matrix3x3_rvshearing;
    st_matrix3x3_dvshearing_t  matrix3x3_dvshearing;
} st_matrix3x3ctx_funcs_t;

typedef struct {
    st_matrix3x3_clone_t     clone;
    st_matrix3x3_apply_t     apply;
    st_matrix3x3_translate_t translate;
    st_matrix3x3_scale_t     scale;
    st_matrix3x3_rrotate_t   rrotate;
    st_matrix3x3_drotate_t   drotate;
    st_matrix3x3_rhshear_t   rhshear;
    st_matrix3x3_dhshear_t   dhshear;
    st_matrix3x3_rvshear_t   rvshear;
    st_matrix3x3_dvshear_t   dvshear;
    st_matrix3x3_get_data_t  get_data;
} st_matrix3x3_funcs_t;

#define ST_MATRIX3X3_CALL(object, func, ...) \
    ((st_matrix3x3_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
