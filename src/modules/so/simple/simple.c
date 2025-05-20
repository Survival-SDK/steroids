#include "simple.h"

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_soctx_t *st_so_init(const st_param_t params[]);
static void st_so_quit(st_soctx_t *so_ctx);
static void st_so_close(st_so_t *so);

static st_so_t *st_so_open(st_soctx_t *so_ctx, const char *filename);
static st_so_t *st_so_memopen(st_soctx_t *so_ctx, const void *data,
 size_t size);
static void *st_so_load_symbol(st_so_t *so, const char *name);

static st_soctx_funcs_t soctx_funcs = {
    st_modctx_funcs,
    .open    = st_so_open,
    .memopen = st_so_memopen,
};

static st_so_funcs_t so_funcs = {
    st_object_funcs,
    .load_symbol = st_so_load_symbol,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_so_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("so", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_so_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_so_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static void st_so_free(void *so) {
    st_soctx_t *so_ctx = (st_soctx_t *)st_object_get_owner(so);

    if (dlclose(((st_so_t *)so)->handle) != 0)
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, warning,
         "so_simple: Unable to close so file. %s", dlerror());
}

static st_soctx_t *st_so_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_soctx_t     *so_ctx = (st_soctx_t *)st_modctx_new("so", "simple",
     sizeof(st_soctx_t), NULL, &soctx_funcs, (st_object_dtor_t)st_so_quit);

    if (so_ctx == NULL) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "so_simple: unable to create new so ctx object");

        return NULL;
    }

    so_ctx->logger_ctx = logger_ctx;
    so_ctx->opened_handles = st_dlist_create(sizeof(st_so_t), st_so_free);
    if (!so_ctx->opened_handles) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "so_simple: Unable to create list of so file entries");
        free(so_ctx);

        return NULL;
    }

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "so_simple: Shared libraries manager context initialized");

    return so_ctx;
}

static void st_so_quit(st_soctx_t *so_ctx) {
    st_dlist_destroy(so_ctx->opened_handles);

    ST_LOGGERCTX_CALL(so_ctx->logger_ctx, info,
     "so_simple: Shared libraries manager context destroyed");
    free(so_ctx);
}

static st_so_t *st_so_open(st_soctx_t *so_ctx, const char *filename) {
    void        *handle = dlopen(filename, RTLD_LAZY);
    st_dlnode_t *node;
    st_so_t      so = { .handle = handle };

    if (handle) {
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, info,
         "so_simple: So file \"%s\" opened", filename);
    } else {
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, error,
         "so_simple: Unable to open so file \"%s\": %s", filename, dlerror());

        return NULL;
    }

    st_object_placement_new(&so, &so_funcs, st_object_fake_dtor,
     (st_object_t *)so_ctx);

    node = st_dlist_push_back(so_ctx->opened_handles, &so);
    if (!node) {
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, info,
         "so_simple: Unable to create node for so file entry: \"%s\"",
         filename);
        dlclose(handle);

        return NULL;
    }

    return st_dlist_get_data(node);
}

static st_so_t *st_so_memopen(st_soctx_t *so_ctx,
 __attribute__((unused)) const void *data, __attribute__((unused))size_t size) {
    ST_LOGGERCTX_CALL(so_ctx->logger_ctx, error,
     "so_simple: Unable to open so from memory. Not implemented yet");

    return NULL;
}

static void st_so_close(st_so_t *so) {
    st_soctx_t  *so_ctx = (st_soctx_t *)st_object_get_owner(so);
    st_dlnode_t *node = st_dlist_get_head(so_ctx->opened_handles);

    while (node) {
        if (st_dlist_get_data(node) == so) {
            st_dlist_remove(node);

            break;
        }

        node = st_dlist_get_next(node);
    }

    ST_LOGGERCTX_CALL(so_ctx->logger_ctx, info, "so_simple: So file closed");
}

static void *st_so_load_symbol(st_so_t *so, const char *name) {
    st_soctx_t *so_ctx = (st_soctx_t *)st_object_get_owner(so);
    void       *symbol = dlsym(so->handle, name);

    if (symbol)
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, info,
         "so_simple: Symbol loaded \"%s\"", name);
    else
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, error,
         "so_simple: Unable to load symbol \"%s\"", name);

    return symbol;
}
