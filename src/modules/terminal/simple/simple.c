#include "simple.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_terminalctx_t *st_terminal_init(const st_param_t params[]);
static void st_terminal_quit(st_terminalctx_t *terminal_ctx);

static int st_terminal_get_rows_count(const st_terminalctx_t *terminal_ctx);
static int st_terminal_get_cols_count(const st_terminalctx_t *terminal_ctx);

static st_terminalctx_funcs_t terminalctx_funcs = {
    st_modctx_funcs,
    .get_rows_count = st_terminal_get_rows_count,
    .get_cols_count = st_terminal_get_cols_count,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_terminal_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("terminal", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_terminal_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_terminal_simple_init(modsmgr);
}
#endif

static st_terminalctx_t *st_terminal_init(const st_param_t params[]) {
    st_terminalctx_t *terminal_ctx;
    st_modsmgr_t     *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t   *logger_ctx;

    if (!modsmgr)
        return NULL;

    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    terminal_ctx = (st_terminalctx_t *)st_modctx_new("terminal", "simple",
     sizeof(st_terminalctx_t), NULL, &terminalctx_funcs,
     (st_object_dtor_t)st_terminal_quit);
    if (!terminal_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "terminal_simple: unable to create new terminal ctx object");

        return NULL;
    }
    terminal_ctx->modsmgr = modsmgr;
    terminal_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info, 
     "terminal_simple: Terminal utils initialized");

    return terminal_ctx;
}
static void st_terminal_quit(st_terminalctx_t *terminal_ctx) {
    ST_LOGGERCTX_CALL(terminal_ctx->logger_ctx, info,
     "terminal_simple: Terminal utils destroyed");
    free(terminal_ctx);
}

static int st_terminal_get_rows_count(
 __attribute__((unused)) const st_terminalctx_t *terminal_ctx) {
    struct winsize ws;

    return ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 ? ws.ws_row : -1;
}

static int st_terminal_get_cols_count(
 __attribute__((unused)) const st_terminalctx_t *terminal_ctx) {
    struct winsize ws;

    return ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 ? ws.ws_col : -1;
}
