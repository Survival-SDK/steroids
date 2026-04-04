#include "simple.h"

#include <errno.h>
#include <time.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_timerctx_t *st_timer_init(const st_param_t params[]);
static void st_timer_quit(st_timerctx_t *timer_ctx);

static uint64_t st_timer_start(const st_timerctx_t *timer_ctx);
static unsigned st_timer_get_elapsed(const st_timerctx_t *timer_ctx, 
 uint64_t start);
static void st_timer_sleep(const st_timerctx_t *timer_ctx, unsigned ms);
static void st_timer_sleep_for_fps(const st_timerctx_t *timer_ctx, 
 unsigned fps);

static st_timerctx_funcs_t timerctx_funcs = {
    st_modctx_funcs,
    .start         = st_timer_start,
    .get_elapsed   = st_timer_get_elapsed,
    .sleep         = st_timer_sleep,
    .sleep_for_fps = st_timer_sleep_for_fps,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_timer_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("timer", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_timer_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_timer_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "timer";
static const char *st_module_name = "simple";

static st_timerctx_t *st_timer_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx;
    st_timerctx_t  *timer_ctx;

    if (!modsmgr)
        return NULL;
    
    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    timer_ctx = (st_timerctx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_timerctx_t), NULL, &timerctx_funcs,
     (st_object_dtor_t)st_timer_quit);
    if (!timer_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create timer context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    timer_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Timer context initialized", st_module_subsystem, st_module_name);

    return timer_ctx;
}

static void st_timer_quit(st_timerctx_t *timer_ctx) {
    ST_LOGGERCTX_CALL(timer_ctx->logger_ctx, info,
     "%s_%s: Timer context destroyed", st_module_subsystem, st_module_name);
    free(timer_ctx);
}

static uint64_t st_timer_start(
 __attribute__((unused)) const st_timerctx_t *timer_ctx) {
    return (uint64_t)clock() / (uint64_t)(CLOCKS_PER_SEC / 1000);
}

static unsigned st_timer_get_elapsed(
 __attribute__((unused)) const st_timerctx_t *timer_ctx, uint64_t start) {
    return (unsigned)((uint64_t)clock() / (uint64_t)(CLOCKS_PER_SEC / 1000) - 
     start);
}

static void st_timer_sleep(
 __attribute__((unused)) const st_timerctx_t *timer_ctx, unsigned ms) {
    int             ret;
    struct timespec ts = {
        .tv_sec  = (long)ms / 1000,
        .tv_nsec = ((long)ms % 1000) * 1000000,
    };

    do
        ret = nanosleep(&ts, &ts);
    while (ret != 0 && errno == EINTR);
}

static void st_timer_sleep_for_fps(const st_timerctx_t *timer_ctx, 
 unsigned fps) {
    st_timer_sleep(timer_ctx, 1000 / fps);
}
