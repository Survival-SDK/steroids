#include "simple.h"

#include <dlfcn.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/types/object.h"

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_soctx_funcs_t soctx_funcs = {
    .quit    = st_so_quit,
    .open    = st_so_open,
    .memopen = st_so_memopen,
};

static st_so_funcs_t so_funcs = {
    .close       = st_so_close,
    .load_symbol = st_so_load_symbol,
};

ST_MODULE_DEF_GET_FUNC(so_simple)
ST_MODULE_DEF_INIT_FUNC(so_simple)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_so_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "so";
static const char *st_module_name = "simple";

static void st_so_free(void *so) {
    st_soctx_t *so_ctx = st_object_get_owner(so);

    if (dlclose(((st_so_t *)so)->handle) != 0)
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, warning,
         "so_simple: Unable to close so file. %s", dlerror());
}

static st_soctx_t *st_so_init(struct st_loggerctx_s *logger_ctx) {
    st_soctx_t *so_ctx = st_modctx_new(st_module_subsystem, st_module_name,
     sizeof(st_soctx_t), NULL, &soctx_funcs);

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
     "so_simple: So manager context initialized");

    return so_ctx;
}

static void st_so_quit(st_soctx_t *so_ctx) {
    st_dlist_destroy(so_ctx->opened_handles);

    ST_LOGGERCTX_CALL(so_ctx->logger_ctx, info,
     "so_simple: So manager context destroyed");
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

    st_object_make(&so, so_ctx, &so_funcs);

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
    st_soctx_t  *so_ctx = st_object_get_owner(so);
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
    st_soctx_t *so_ctx = st_object_get_owner(so);
    void       *symbol = dlsym(so->handle, name);

    if (symbol)
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, info,
         "so_simple: Symbol loaded \"%s\"", name);
    else
        ST_LOGGERCTX_CALL(so_ctx->logger_ctx, error,
         "so_simple: Unable to load symbol \"%s\"", name);

    return symbol;
}
