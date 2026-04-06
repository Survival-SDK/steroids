#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/object.h"
#include "steroids/params.h"

#define ST_MODCTX_CALL(object, func, ...) \
    ((const st_modctx_funcs_t *)object->funcs)->func(object, ## __VA_ARGS__)

struct st_modctx;

typedef const char *(*st_modctx_get_subsystem_t)(
 const struct st_modctx *modctx);
typedef const char *(*st_modctx_get_name_t)(const struct st_modctx *modctx);

typedef struct {
    st_object_funcs_t;
    st_modctx_get_subsystem_t get_subsystem;
    st_modctx_get_name_t      get_name;
} st_modctx_funcs_t;

typedef struct st_modctx {
    st_object_t;
    const char *st_subsystem;
    const char *st_name;
} st_modctx_t;

typedef st_modctx_t *(*st_ctx_ctor_t)(const st_param_t params[]);

static const char *st_modctx_get_subsystem(const st_modctx_t *modctx);
static const char *st_modctx_get_name(const st_modctx_t *modctx);

static const st_modctx_funcs_t st_modctx_funcs = {
    st_object_funcs,
    .get_subsystem = st_modctx_get_subsystem,
    .get_name      = st_modctx_get_name,
};

static inline st_modctx_t *st_modctx_new(const char *subsystem, 
 const char *name, size_t size, void *module, const void *funcs, 
 st_object_dtor_t dtor) {
    st_modctx_t *ctx = (st_modctx_t *)st_object_new(
     size ?: sizeof(st_modctx_t), funcs ?: &st_modctx_funcs, dtor, module);

    ctx->st_subsystem = subsystem;
    ctx->st_name      = name;

    return ctx;
}

static const char *st_modctx_get_subsystem(const st_modctx_t *modctx) {
    return modctx->st_subsystem;
}

static const char *st_modctx_get_name(const st_modctx_t *modctx) {
    return modctx->st_name;
}

static inline const st_param_t *st_modctx_get_param(const st_param_t params[],
 const char *key) {
    const st_param_t *param;

    if (!params || !key)
        return NULL;

    param = &params[0];
    while (memcmp(param, &(st_param_t){0},
     sizeof(st_param_t)) != 0) {
        if (param->key != NULL && strcmp(param->key, key) == 0)
            return param;

        param++;
    }

    return NULL;
}

static inline void *st_modctx_get_param_as_ptr(const st_param_t params[],
 const char *key) {
    const st_param_t *param = st_modctx_get_param(params, key);

    return param
        ? (void *)param->value
        : NULL;
}

static inline int st_modctx_get_param_as_int(const st_param_t params[],
 const char *key) {
    const st_param_t *param = st_modctx_get_param(params, key);

    return param
        ? (int)param->value
        : 0;
}

static inline bool st_modctx_get_param_as_bool(const st_param_t params[],
 const char *key) {
    return !!st_modctx_get_param(params, key);
}
