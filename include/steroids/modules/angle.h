#pragma once

#include "steroids/module.h"

typedef st_modctx_t *(*st_angle_init_t)(st_modctx_t *logger_ctx);
typedef void (*st_angle_quit_t)(st_modctx_t *angle_ctx);

typedef float (*st_angle_rtod_t)(st_modctx_t *angle_ctx, float radians);
typedef float (*st_angle_dtor_t)(st_modctx_t *angle_ctx, float degrees);
typedef void (*st_angle_rnormalize360_t)(st_modctx_t *angle_ctx,
 float *radians);
typedef float (*st_angle_rnormalized360_t)(st_modctx_t *angle_ctx,
 float radians);
typedef void (*st_angle_dnormalize360_t)(st_modctx_t *angle_ctx,
 float *radians);
typedef float (*st_angle_dnormalized360_t)(st_modctx_t *angle_ctx,
 float degrees);
typedef float (*st_angle_rdsin_t)(st_modctx_t *angle_ctx, float radians);
typedef float (*st_angle_dgsin_t)(st_modctx_t *angle_ctx, float degrees);
typedef float (*st_angle_rdcos_t)(st_modctx_t *angle_ctx, float radians);
typedef float (*st_angle_dgcos_t)(st_modctx_t *angle_ctx, float degrees);
typedef float (*st_angle_rdtan_t)(st_modctx_t *angle_ctx, float radians);
typedef float (*st_angle_dgtan_t)(st_modctx_t *angle_ctx, float degrees);
typedef float (*st_angle_rdacos_t)(st_modctx_t *angle_ctx, float angle_cos);
typedef float (*st_angle_dgacos_t)(st_modctx_t *angle_ctx, float angle_cos);

typedef struct {
    st_angle_init_t           angle_init;
    st_angle_quit_t           angle_quit;
    st_angle_rtod_t           angle_rtod;
    st_angle_dtor_t           angle_dtor;
    st_angle_rnormalize360_t  angle_rnormalize360;
    st_angle_rnormalized360_t angle_rnormalized360;
    st_angle_dnormalize360_t  angle_dnormalize360;
    st_angle_dnormalized360_t angle_dnormalized360;
    st_angle_rdsin_t          angle_rdsin;
    st_angle_dgsin_t          angle_dgsin;
    st_angle_rdcos_t          angle_rdcos;
    st_angle_dgcos_t          angle_dgcos;
    st_angle_rdtan_t          angle_rdtan;
    st_angle_dgtan_t          angle_dgtan;
    st_angle_rdacos_t         angle_rdacos;
    st_angle_dgacos_t         angle_dgacos;
} st_angle_funcs_t;
