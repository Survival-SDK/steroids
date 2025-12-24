#include "simple.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_anglectx_t *st_angle_init(const st_param_t params[]);
static void st_angle_quit(st_anglectx_t *angle_ctx);

static float st_angle_rtod(const st_anglectx_t *angle_ctx, float radians);
static float st_angle_dtor(const st_anglectx_t *angle_ctx, float degrees);
static void st_angle_rnormalize360(const st_anglectx_t *angle_ctx,
 float *radians);
static float st_angle_rnormalized360(const st_anglectx_t *angle_ctx,
 float radians);
static void st_angle_dnormalize360(const st_anglectx_t *angle_ctx,
 float *degrees);
static float st_angle_dnormalized360(const st_anglectx_t *angle_ctx,
 float degrees);
static float st_angle_rdsin(const st_anglectx_t *angle_ctx, float radians);
static float st_angle_dgsin(const st_anglectx_t *angle_ctx, float degrees);
static float st_angle_rdcos(const st_anglectx_t *angle_ctx, float radians);
static float st_angle_dgcos(const st_anglectx_t *angle_ctx, float degrees);
static float st_angle_rdtan(const st_anglectx_t *angle_ctx, float radians);
static float st_angle_dgtan(const st_anglectx_t *angle_ctx, float degrees);
static float st_angle_rdacos(const st_anglectx_t *angle_ctx, float angle_cos);
static float st_angle_dgacos(const st_anglectx_t *angle_ctx, float angle_cos);

static st_anglectx_funcs_t anglectx_funcs = {
    st_modctx_funcs,
    .rtod           = st_angle_rtod,
    .dtor           = st_angle_dtor,
    .rnormalize360  = st_angle_rnormalize360,
    .rnormalized360 = st_angle_rnormalized360,
    .dnormalize360  = st_angle_dnormalize360,
    .dnormalized360 = st_angle_dnormalized360,
    .rdsin          = st_angle_rdsin,
    .dgsin          = st_angle_dgsin,
    .rdcos          = st_angle_rdcos,
    .dgcos          = st_angle_dgcos,
    .rdtan          = st_angle_rdtan,
    .dgtan          = st_angle_dgtan,
    .rdacos         = st_angle_rdacos,
    .dgacos         = st_angle_dgacos,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_angle_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("angle", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_angle_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_angle_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "angle";
static const char *st_module_name = "simple";

static st_anglectx_t *st_angle_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_anglectx_t  *angle_ctx;

    if (!logger_ctx) {
        fprintf(stderr,
         "%s_%s: Unable to get logger context\n", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    angle_ctx = (st_anglectx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_anglectx_t), NULL, &anglectx_funcs,
     (st_object_dtor_t)st_angle_quit);
    if (!angle_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create angle context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    angle_ctx->modsmgr = modsmgr;
    angle_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Context initialized", st_module_subsystem, st_module_name);

    return angle_ctx;
}

static void st_angle_quit(st_anglectx_t *angle_ctx) {
    ST_LOGGERCTX_CALL(angle_ctx->logger_ctx, info,
     "%s_%s: Context destroyed", st_module_subsystem, st_module_name);
    free(angle_ctx);
}

static float st_angle_rtod(__attribute__((unused)) const st_anglectx_t *angle_ctx,
 float radians) {
    return radians * 180 / (float)M_PI;
}

static float st_angle_dtor(__attribute__((unused)) const st_anglectx_t *angle_ctx,
 float degrees) {
    return degrees * (float)M_PI / 180;
}

static void st_angle_rnormalize360(
 __attribute__((unused)) const st_anglectx_t *angle_ctx, float *radians) {
    *radians = fmodf(*radians, 2.0f * (float)M_PI);
    if (*radians < 0.0f)
        *radians += 2.0f * (float)M_PI;
}

static float st_angle_rnormalized360(const st_anglectx_t *angle_ctx,
 float radians) {
    st_angle_rnormalize360(angle_ctx, &radians);

    return radians;
}

static void st_angle_dnormalize360(
 __attribute__((unused)) const st_anglectx_t *angle_ctx, float *degrees) {
    *degrees = fmodf(*degrees, 360.0f);
    if (*degrees < 0.0f)
        *degrees += 360.0f;
}

static float st_angle_dnormalized360(const st_anglectx_t *angle_ctx,
 float degrees) {
    st_angle_dnormalize360(angle_ctx, &degrees);

    return degrees;
}

static float st_angle_rdsin(__attribute__((unused)) const st_anglectx_t *angle_ctx,
 float radians) {
    return sinf(radians);
}

static float st_angle_dgsin(const st_anglectx_t *angle_ctx, float degrees) {
    return sinf(st_angle_dtor(angle_ctx, degrees));
}

static float st_angle_rdcos(__attribute__((unused)) const st_anglectx_t *angle_ctx,
 float radians) {
    return cosf(radians);
}

static float st_angle_dgcos(const st_anglectx_t *angle_ctx, float degrees) {
    return cosf(st_angle_dtor(angle_ctx, degrees));
}

static float st_angle_rdtan(__attribute__((unused)) const st_anglectx_t *angle_ctx,
 float radians) {
    return tanf(radians);
}

static float st_angle_dgtan(const st_anglectx_t *angle_ctx, float degrees) {
    return tanf(st_angle_dtor(angle_ctx, degrees));
}

static float st_angle_rdacos(__attribute__((unused)) const st_anglectx_t *angle_ctx,
 float angle_cos) {
    return acosf(angle_cos);
}

static float st_angle_dgacos(const st_anglectx_t *angle_ctx, float angle_cos) {
    return st_angle_rtod(angle_ctx, acosf(angle_cos));
}
