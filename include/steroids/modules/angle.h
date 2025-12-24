#pragma once

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_ANGLECTX_T_DEFINED
    typedef st_modctx_t st_anglectx_t;
#endif

typedef float (*st_angle_rtod_t)(const st_anglectx_t *angle_ctx, float radians);
typedef float (*st_angle_dtor_t)(const st_anglectx_t *angle_ctx, float degrees);
typedef void (*st_angle_rnormalize360_t)(const st_anglectx_t *angle_ctx,
 float *radians);
typedef float (*st_angle_rnormalized360_t)(const st_anglectx_t *angle_ctx,
 float radians);
typedef void (*st_angle_dnormalize360_t)(const st_anglectx_t *angle_ctx,
 float *radians);
typedef float (*st_angle_dnormalized360_t)(const st_anglectx_t *angle_ctx,
 float degrees);
typedef float (*st_angle_rdsin_t)(const st_anglectx_t *angle_ctx, float radians);
typedef float (*st_angle_dgsin_t)(const st_anglectx_t *angle_ctx, float degrees);
typedef float (*st_angle_rdcos_t)(const st_anglectx_t *angle_ctx, float radians);
typedef float (*st_angle_dgcos_t)(const st_anglectx_t *angle_ctx, float degrees);
typedef float (*st_angle_rdtan_t)(const st_anglectx_t *angle_ctx, float radians);
typedef float (*st_angle_dgtan_t)(const st_anglectx_t *angle_ctx, float degrees);
typedef float (*st_angle_rdacos_t)(const st_anglectx_t *angle_ctx,
 float angle_cos);
typedef float (*st_angle_dgacos_t)(const st_anglectx_t *angle_ctx,
 float angle_cos);

typedef struct {
    st_modctx_funcs_t;
    st_angle_rtod_t           rtod;
    st_angle_dtor_t           dtor;
    st_angle_rnormalize360_t  rnormalize360;
    st_angle_rnormalized360_t rnormalized360;
    st_angle_dnormalize360_t  dnormalize360;
    st_angle_dnormalized360_t dnormalized360;
    st_angle_rdsin_t          rdsin;
    st_angle_dgsin_t          dgsin;
    st_angle_rdcos_t          rdcos;
    st_angle_dgcos_t          dgcos;
    st_angle_rdtan_t          rdtan;
    st_angle_dgtan_t          dgtan;
    st_angle_rdacos_t         rdacos;
    st_angle_dgacos_t         dgacos;
} st_anglectx_funcs_t;

#define ST_ANGLECTX_CALL(ctx, func, ...) \
    ((st_anglectx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
