#pragma once

#include "steroids/types/object.h"

#define ST_MODDATA_CALL(object, func, ...) \
    ((const st_moddata_funcs_t *)object->funcs)->func(object, ## __VA_ARGS__)

struct st_moddata;

typedef struct {
    const char *subsystem;
    const char *name;
} st_modprerq_t;

typedef const char *(*st_moddata_get_subsystem_t)(
 const struct st_moddata *moddata);
typedef const char *(*st_moddata_get_name_t)(const struct st_moddata *moddata);
typedef const char *(*st_moddata_get_type_t)(const struct st_moddata *moddata);
typedef const st_modprerq_t *(*st_moddata_get_prereqs_t)(
 const struct st_moddata *moddata);
typedef void *(*st_moddata_get_ctx_ctor_t)(const struct st_moddata *moddata);

typedef struct {
    st_object_funcs_t;
    st_moddata_get_subsystem_t get_subsystem;
    st_moddata_get_name_t      get_name;
    st_moddata_get_type_t      get_type;
    st_moddata_get_prereqs_t   get_prereqs;
    st_moddata_get_ctx_ctor_t  get_ctx_ctor;
} st_moddata_funcs_t;

typedef struct st_moddata {
    st_object_t;
    const char          *st_subsystem;
    const char          *st_name;
    const char          *st_type;
    const st_modprerq_t *st_prereqs;
    void                *st_ctx_ctor;
} st_moddata_t;

static const char *st_moddata_get_subsystem(const struct st_moddata *moddata);
static const char *st_moddata_get_name(const struct st_moddata *moddata);
static const char *st_moddata_get_type(const struct st_moddata *moddata);
static const st_modprerq_t *st_moddata_get_prereqs(const struct st_moddata *moddata);
static void *st_moddata_get_ctx_ctor(const struct st_moddata *moddata);

static const st_moddata_funcs_t st_moddata_funcs = {
    st_object_funcs,
    .get_subsystem = st_moddata_get_subsystem,
    .get_name      = st_moddata_get_name,
    .get_type      = st_moddata_get_type,
    .get_prereqs   = st_moddata_get_prereqs,
    .get_ctx_ctor  = st_moddata_get_ctx_ctor,
};

static st_moddata_t *st_moddata_new(const char *subsystem, const char *name,
 const char *type, const st_modprerq_t *prereqs, void *ctx_ctor,
 void *modsmgr) {
    st_moddata_t *moddata = (st_moddata_t *)st_object_new(
     sizeof(st_moddata_t), &st_moddata_funcs, NULL, modsmgr);

    moddata->st_subsystem = subsystem;
    moddata->st_name      = name;
    moddata->st_type      = type;
    moddata->st_prereqs   = prereqs;
    moddata->st_ctx_ctor  = ctx_ctor;

    return moddata;
}

static const char *st_moddata_get_subsystem(const st_moddata_t *moddata) {
    return moddata->st_subsystem;
}

static const char *st_moddata_get_name(const st_moddata_t *moddata) {
    return moddata->st_name;
}

static const char *st_moddata_get_type(const st_moddata_t *moddata) {
    return moddata->st_type;
}

static const st_modprerq_t *st_moddata_get_prereqs(
 const st_moddata_t *moddata) {
    return moddata->st_prereqs;
}

static void *st_moddata_get_ctx_ctor(const st_moddata_t *moddata) {
    return moddata->st_ctx_ctor;
}
