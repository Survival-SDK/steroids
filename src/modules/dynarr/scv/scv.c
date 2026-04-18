#include "scv.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <scv.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_dynarrctx_t *st_dynarr_init(const st_param_t params[]);
static void st_dynarr_quit(st_dynarrctx_t *dynarr_ctx);

static st_dynarr_t *st_dynarr_create(st_dynarrctx_t *dynarr_ctx,
 size_t data_size, size_t initial_capacity);
static void st_dynarr_destroy(st_dynarr_t *dynarr);
static bool st_dynarr_append(st_dynarr_t *dynarr, const void *data);
static bool st_dynarr_set(st_dynarr_t *dynarr, size_t index, const void *data);
static bool st_dynarr_clear(st_dynarr_t *dynarr);
static bool st_dynarr_sort(st_dynarr_t *dynarr,
 int (*cmpfunc)(const void *, const void *, void *), void *userptr);
static bool st_dynarr_extract(const st_dynarr_t *dynarr, void *dst,
 size_t index);
static const void *st_dynarr_get(const st_dynarr_t *dynarr, size_t index);
static const void *st_dynarr_get_last(const st_dynarr_t *dynarr);
static const void *st_dynarr_get_all(const st_dynarr_t *dynarr);
static size_t st_dynarr_get_elements_count(const st_dynarr_t *dynarr);
static bool st_dynarr_is_empty(const st_dynarr_t *dynarr);

static st_dynarrctx_funcs_t dynarrctx_funcs = {
    ST_MODCTX_FUNCS,
    .create = st_dynarr_create,
};

static st_dynarr_funcs_t dynarr_funcs = {
    ST_OBJECT_FUNCS,
    .append             = st_dynarr_append,
    .set                = st_dynarr_set,
    .clear              = st_dynarr_clear,
    .sort               = st_dynarr_sort,
    .extract            = st_dynarr_extract,
    .get                = st_dynarr_get,
    .get_last           = st_dynarr_get_last,
    .get_all            = st_dynarr_get_all,
    .get_elements_count = st_dynarr_get_elements_count,
    .is_empty           = st_dynarr_is_empty,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_dynarr_scv_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("dynarr", "scv", ST_MODULE_TYPE, mod_prereqs,
     st_dynarr_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_dynarr_scv_init(modsmgr);
}
#endif

static st_dynarrctx_t *st_dynarr_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_dynarrctx_t *dynarr_ctx = (st_dynarrctx_t *)st_modctx_new("dynarr",
     "scv", sizeof(st_dynarrctx_t), NULL, &dynarrctx_funcs,
     (st_object_dtor_t)st_dynarr_quit);

    if (!dynarr_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "dynarr_scv: unable to create new dynarr ctx object");

        return NULL;
    }

    dynarr_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "dynarr_scv: dynamic arrays manipulation module context initialized");

    return dynarr_ctx;
}

static void st_dynarr_quit(st_dynarrctx_t *dynarr_ctx) {
    ST_LOGGERCTX_CALL(dynarr_ctx->logger_ctx, info,
     "dynarr_scv: dynamic arrays manipulation module context destroyed");
    free(dynarr_ctx);
}

static st_dynarr_t *st_dynarr_create(st_dynarrctx_t *dynarr_ctx,
 size_t data_size, size_t initial_capacity) {
    struct scv_vector *handle = scv_new(data_size, initial_capacity);
    st_dynarr_t       *dynarr;

    if (!handle) {
        ST_LOGGERCTX_CALL(dynarr_ctx->logger_ctx, error,
         "dynarr_scv: Unable to create dynamic array handle");

        return NULL;
    }

    dynarr = (st_dynarr_t *)st_object_new(sizeof(st_dynarr_t), &dynarr_funcs,
     (st_object_dtor_t)st_dynarr_destroy, (st_object_t *)dynarr_ctx);
    if (!dynarr) {
        ST_LOGGERCTX_CALL(dynarr_ctx->logger_ctx, error,
         "dynarr_scv: Unable to allocate memory for dynamic array");
        scv_delete(handle);

        return NULL;
    }

    dynarr->handle = handle;

    return dynarr;
}

static void st_dynarr_destroy(st_dynarr_t *dynarr) {
    if (dynarr) {
        scv_delete(dynarr->handle);
        free(dynarr);
    }
}

static bool st_dynarr_append(st_dynarr_t *dynarr, const void *data) {
    return scv_push_back(dynarr->handle, data) == SCV_OK;
}

static bool st_dynarr_set(st_dynarr_t *dynarr, size_t index, const void *data) {
    return scv_replace(dynarr->handle, index, index + 1, data, 1) == SCV_OK;
}

static bool st_dynarr_clear(st_dynarr_t *dynarr) {
    return scv_clear(dynarr->handle) == SCV_OK;
}

static bool st_dynarr_sort(st_dynarr_t *dynarr,
 int (*cmpfunc)(const void *, const void *, void *), void *userptr) {
    qsort_r(scv_data(dynarr->handle), scv_size(dynarr->handle),
     scv_objsize(dynarr->handle), cmpfunc, userptr);

    return true;
}

static bool st_dynarr_extract(const st_dynarr_t *dynarr, void *dst,
 size_t index) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-qual"
        memcpy(dst, scv_at(dynarr->handle, index),
         scv_objsize(dynarr->handle));
    #pragma GCC diagnostic pop

    return true;
}

static const void *st_dynarr_get(const st_dynarr_t *dynarr, size_t index) {
    return scv_at(dynarr->handle, index);
}

static const void *st_dynarr_get_last(const st_dynarr_t *dynarr) {
    return scv_back(dynarr->handle);
}

static const void *st_dynarr_get_all(const st_dynarr_t *dynarr) {
    return scv_data(dynarr->handle);
}

static size_t st_dynarr_get_elements_count(const st_dynarr_t *dynarr) {
    return scv_size(dynarr->handle);
}

static bool st_dynarr_is_empty(const st_dynarr_t *dynarr) {
    return scv_empty(dynarr->handle);
}
