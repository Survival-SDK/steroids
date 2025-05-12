#include "simple.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define OFFSET_BASIS 0x811C9dC5u
#define PRIME        0x01000193u

static st_fnv1actx_t *st_fnv1a_init(const st_ctxctorparam_t params[]);
static void st_fnv1a_quit(st_fnv1actx_t *fnv1a_ctx);

static uint32_t st_fnv1a_u32hash_string(st_fnv1actx_t *fnv1a_ctx,
 const char *str);
static uint32_t st_fnv1a_u32hash_bytes(st_fnv1actx_t *fnv1a_ctx,
 const void *ptr, size_t size);
static st_u32hashstrfunc_t st_fnv1a_get_u32hashstr_func(
 st_fnv1actx_t *fnv1a_ctx);
static st_u32hashbytesfunc_t st_fnv1a_get_u32hashbytes_func(
 st_fnv1actx_t *fnv1a_ctx);

static st_fnv1actx_funcs_t fnv1actx_funcs = {
    st_modctx_funcs,
    .u32hash_string        = st_fnv1a_u32hash_string,
    .u32hash_bytes         = st_fnv1a_u32hash_bytes,
    .get_u32hashstr_func   = st_fnv1a_get_u32hashstr_func,
    .get_u32hashbytes_func = st_fnv1a_get_u32hashbytes_func,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_fnv1a_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("fnv1a", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_fnv1a_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_fnv1a_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static st_fnv1actx_t *st_fnv1a_init(const st_ctxctorparam_t params[]) {
    st_modsmgr_t          *modsmgr = st_modctx_get_param_as_ptr(params,
     "modsmgr");
    struct st_loggerctx_s *logger_ctx = (
     struct st_loggerctx_s *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    st_fnv1actx_t         *fnv1a_ctx = (st_fnv1actx_t *)st_modctx_new("fnv1a",
     "simple", sizeof(st_fnv1actx_t), NULL, &fnv1actx_funcs,
     (st_object_dtor_t)st_fnv1a_quit);

    if (!fnv1a_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "fnv1a_simple: unable to create new fnv1a ctx object");

        return NULL;
    }

    fnv1a_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "fnv1a_simple: FNV-1a hasher initialized");

    return fnv1a_ctx;
}

static uint32_t fnv1_hash_data(const void *ptr, size_t size) {
    uint32_t hash = OFFSET_BASIS;

    for (size_t i = 0; i < size; i++) {
        hash ^= ((const uint8_t *)ptr)[i];
        hash *= PRIME;
    }

    return hash;
}

static uint32_t fnv1_hash_string(const char *str) {
    return fnv1_hash_data(str, strlen(str));
}

static void st_fnv1a_quit(st_fnv1actx_t *fnv1a_ctx) {
    ST_LOGGERCTX_CALL(fnv1a_ctx->logger_ctx, info,
     "fnv1a_simple: FNV-1a hasher destroyed");
    free(fnv1a_ctx);
}

static uint32_t st_fnv1a_u32hash_string(
 __attribute__((unused)) st_fnv1actx_t *fnv1a_ctx, const char *str) {
    return fnv1_hash_string(str);
}

static uint32_t st_fnv1a_u32hash_bytes(
 __attribute__((unused)) st_fnv1actx_t *fnv1a_ctx, const void *ptr,
 size_t size) {
    return fnv1_hash_data(ptr, size);
}

static st_u32hashstrfunc_t st_fnv1a_get_u32hashstr_func(
 __attribute__((unused)) st_fnv1actx_t *fnv1a_ctx) {
    return fnv1_hash_string;
}

static st_u32hashbytesfunc_t st_fnv1a_get_u32hashbytes_func(
 __attribute__((unused)) st_fnv1actx_t *fnv1a_ctx) {
    return fnv1_hash_data;
}
