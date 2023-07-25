#include "simple.h"

#include <time.h>

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

void *st_module_timer_simple_get_func(const char *func_name) {
    st_modfuncstbl_t *funcs_table = &st_module_timer_simple_funcs_table;

    for (size_t i = 0; i < FUNCS_COUNT; i++) {
        if (strcmp(funcs_table->entries[i].func_name, func_name) == 0)
            return funcs_table->entries[i].func_pointer;
    }

    return NULL;
}

st_moddata_t *st_module_timer_simple_init(void *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    errno_t err = memcpy_s(&global_modsmgr_funcs, sizeof(st_modsmgr_funcs_t),
     modsmgr_funcs, sizeof(st_modsmgr_funcs_t));

    if (err) {
        strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
        fprintf(stderr, "Unable to init module \"timer_simple\": %s\n",
         err_msg_buf);

        return NULL;
    }

    global_modsmgr = modsmgr;

    return &st_module_timer_simple_data;
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(void *modsmgr, st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_timer_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_timer_import_functions(st_modctx_t *timer_ctx,
 st_modctx_t *logger_ctx) {
    st_timer_simple_t *module = timer_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "timer_simple: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("timer_simple", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("timer_simple", logger, info);

    return true;
}

static st_modctx_t *st_timer_init(st_modctx_t *logger_ctx) {
    st_modctx_t       *timer_ctx;
    st_timer_simple_t *module;

    timer_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_timer_simple_data, sizeof(st_timer_simple_t));

    if (!timer_ctx)
        return NULL;

    timer_ctx->funcs = &st_timer_simple_funcs;

    module = timer_ctx->data;
    module->logger.ctx = logger_ctx;

    if (!st_timer_import_functions(timer_ctx, logger_ctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, timer_ctx);

        return NULL;
    }

    module->logger.info(module->logger.ctx, "timer_simple: Timer initialized.");

    return timer_ctx;
}

static void st_timer_quit(st_modctx_t *timer_ctx) {
    st_timer_simple_t *module = timer_ctx->data;

    module->logger.info(module->logger.ctx, "timer_simple: Timer destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, timer_ctx);
}

static uint64_t st_timer_start(st_modctx_t *timer_ctx) {
    return (uint64_t)clock() / (uint64_t)(CLOCKS_PER_SEC / 1000); // NOLINT(readability-magic-numbers)
}

static unsigned st_timer_get_elapsed(st_modctx_t *timer_ctx, uint64_t start) {
    return (unsigned)(start - (uint64_t)clock() /
     (uint64_t)(CLOCKS_PER_SEC / 1000)); // NOLINT(readability-magic-numbers)
}

static void st_timer_sleep(st_modctx_t *timer_ctx, unsigned ms) {
    int             ret;
    struct timespec ts = {
        .tv_sec  = (long)ms / 1000, // NOLINT(readability-magic-numbers)
        .tv_nsec = ((long)ms % 1000) * 1000000, // NOLINT(readability-magic-numbers)
    };

    do
        ret = nanosleep(&ts, &ts);
    while (ret != 0 && errno == EINTR);
}

static void st_timer_sleep_for_fps(st_modctx_t *timer_ctx, unsigned fps) {
    st_timer_sleep(timer_ctx, 1000 / fps); // NOLINT(readability-magic-numbers)
}