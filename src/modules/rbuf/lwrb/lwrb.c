#include "lwrb.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <lwrb.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <safeclib/safe_mem_lib.h>
#include <safeclib/safe_str_lib.h>
#pragma GCC diagnostic pop
#include <safeclib/safe_types.h>

#define ERR_MSG_BUF_SIZE 1024

static void              *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;
static char               err_msg_buf[ERR_MSG_BUF_SIZE];

void *st_module_rbuf_lwrb_get_func(const char *func_name) {
    st_modfuncstbl_t *funcs_table = &st_module_rbuf_lwrb_funcs_table;

    for (size_t i = 0; i < FUNCS_COUNT; i++) {
        if (strcmp(funcs_table->entries[i].func_name, func_name) == 0)
            return funcs_table->entries[i].func_pointer;
    }

    return NULL;
}

st_moddata_t *st_module_rbuf_lwrb_init(void *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    errno_t err = memcpy_s(&global_modsmgr_funcs, sizeof(st_modsmgr_funcs_t),
     modsmgr_funcs, sizeof(st_modsmgr_funcs_t));

    if (err) {
        strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
        fprintf(stderr, "Unable to init module \"hash_table_hash_table\": %s\n",
         err_msg_buf);

        return NULL;
    }

    global_modsmgr = modsmgr;

    return &st_module_rbuf_lwrb_data;
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(void *modsmgr, st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_rbuf_lwrb_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_rbuf_import_functions(st_modctx_t *rbuf_ctx,
 st_modctx_t *logger_ctx) {
    st_rbuf_lwrb_t *module = rbuf_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "rbuf_lwrb: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION("rbuf_lwrb", logger, debug);
    ST_LOAD_FUNCTION("rbuf_lwrb", logger, info);

    return true;
}

static st_modctx_t *st_rbuf_init(st_modctx_t *logger_ctx) {
    st_modctx_t    *rbuf_ctx;
    st_rbuf_lwrb_t *module;

    rbuf_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_rbuf_lwrb_data, sizeof(st_rbuf_lwrb_t));

    if (!rbuf_ctx)
        return NULL;

    rbuf_ctx->funcs = &st_rbuf_lwrb_funcs;

    module = rbuf_ctx->data;
    module->logger.ctx = logger_ctx;

    if (!st_rbuf_import_functions(rbuf_ctx, logger_ctx))
        return NULL;

    module->logger.info(module->logger.ctx, "rbuf_lwrb: Module initialized");

    return rbuf_ctx;
}

static void st_rbuf_quit(st_modctx_t *rbuf_ctx) {
    st_rbuf_lwrb_t *module = rbuf_ctx->data;

    module->logger.info(module->logger.ctx, "rbuf_lwrb: Module destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, rbuf_ctx);
}

static st_rbuf_t *st_rbuf_create(st_modctx_t *rbuf_ctx, size_t size) {
    st_rbuf_lwrb_t *module = rbuf_ctx->data;
    st_rbuf_t      *rbuf = malloc(sizeof(st_rbuf_t));

    if (!rbuf) {
        module->logger.error(module->logger.ctx,
         "rbuf_lwrb: Unable to allocate memory for ring buffer structure: %s",
         strerror(errno));

        return NULL;
    }

    rbuf->data = malloc(size);
    if (!rbuf->data) {
        module->logger.error(module->logger.ctx,
         "rbuf_lwrb: Unable to allocate memory for ring buffer data: %s",
         strerror(errno));

        goto malloc_data_fail;
    }

    if (!lwrb_init(&rbuf->handle, rbuf->data, size)) {
        module->logger.error(module->logger.ctx,
         "rbuf_lwrb: Unable to init ring buffer");

        goto init_fail;
    }

    rbuf->ctx = rbuf_ctx;

    return rbuf;

init_fail:
    free(rbuf->data);
malloc_data_fail:
    free(rbuf);

    return NULL;
}

static void st_rbuf_destroy(st_rbuf_t *rbuf) {
    lwrb_free(&rbuf->handle);
}

static bool st_rbuf_push(st_rbuf_t *rbuf, const void *data, size_t size) {
    if (lwrb_get_free(&rbuf->handle) < size)
        return false;

    return lwrb_write(&rbuf->handle, data, size) == size;
}

static bool st_rbuf_peek(const st_rbuf_t *rbuf, void *data, size_t size) {
    return lwrb_peek(&rbuf->handle, 0, data, size) == size;
}

static bool st_rbuf_pop(st_rbuf_t *rbuf, void *data, size_t size) {
    return lwrb_read(&rbuf->handle, data, size) == size;
}

static bool st_rbuf_drop(st_rbuf_t *rbuf, size_t size) {
    return lwrb_skip(&rbuf->handle, size) == size;
}

static bool st_rbuf_clear(st_rbuf_t *rbuf) {
    while (lwrb_get_full(&rbuf->handle)) {
        if (lwrb_skip(&rbuf->handle, 1) != 1)
            return false;
    }

    return true;
}

static size_t st_rbuf_get_free_space(const st_rbuf_t *rbuf) {
    return lwrb_get_free(&rbuf->handle);
}

static bool st_rbuf_is_empty(const st_rbuf_t *rbuf) {
    return lwrb_get_full(&rbuf->handle) == 0;
}
