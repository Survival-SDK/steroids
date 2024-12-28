#pragma once

#include <stdlib.h>

#include "steroids/types/object.h"

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

static const char *st_modctx_get_subsystem(const st_modctx_t *modctx);
static const char *st_modctx_get_name(const st_modctx_t *modctx);

static const st_modctx_funcs_t st_modctx_funcs = { 
    st_object_funcs,
    .get_subsystem = st_modctx_get_subsystem,
    .get_name      = st_modctx_get_name,
};

static st_modctx_t *st_modctx_new(const char *subsystem, const char *name,
 size_t size, void *module, st_object_dtor_t dtor, const void *funcs) {
    st_modctx_t *ctx = (st_modctx_t *)st_object_new(
     size ?: sizeof(st_modctx_t), dtor, funcs ?: &st_modctx_funcs, module);
    
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
