#include "xlib.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ERRMSGBUF_SIZE        128
#define DISPLAY_NAME_SIZE_MAX 128

static st_monitorctx_t *st_monitor_init(const st_param_t params[]);
static void st_monitor_quit(st_monitorctx_t *monitor_ctx);
static void st_monitor_destroy(st_monitor_t *monitor);

static unsigned st_monitor_get_monitors_count(
 const st_monitorctx_t *monitor_ctx);
static st_monitor_t *st_monitor_open(st_monitorctx_t *monitor_ctx,
 unsigned index);
static unsigned st_monitor_get_width(const st_monitor_t *monitor);
static unsigned st_monitor_get_height(const st_monitor_t *monitor);
static void *st_monitor_get_handle(const st_monitor_t *monitor);
static void st_monitor_set_userdata(const st_monitor_t *monitor,
 const char *key, uintptr_t value);
static bool st_monitor_get_userdata(const st_monitor_t *monitor, uintptr_t *dst,
 const char *key);

static st_monitorctx_funcs_t monitorctx_funcs = {
    st_modctx_funcs,
    .get_monitors_count = st_monitor_get_monitors_count,
    .open               = st_monitor_open,
};

static st_monitor_funcs_t monitor_funcs = {
    st_object_funcs,
    .get_width    = st_monitor_get_width,
    .get_height   = st_monitor_get_height,
    .get_handle   = st_monitor_get_handle,
    .set_userdata = st_monitor_set_userdata,
    .get_userdata = st_monitor_get_userdata,
};

static const st_modprerq_t mod_prereqs[] = {
    { "fnv1a", NULL, },
    { "htable", NULL, },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_monitor_xlib_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("monitor", "xlib", ST_MODULE_TYPE, mod_prereqs,
     st_monitor_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_monitor_xlib_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "monitor";
static const char *st_module_name = "xlib";

static bool st_keyeqfunc(const void *left, const void *right) {
    return left && right && strcmp(left, right) == 0;
}

static st_monitorctx_t *st_monitor_init(const st_param_t params[]) {
    st_modsmgr_t     *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_fnv1actx_t    *fnv1a_ctx = (st_fnv1actx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "fnv1a", NULL);
    st_htablectx_t   *htable_ctx = (st_htablectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "htable", NULL);
    st_loggerctx_t   *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_monitorctx_t  *monitor_ctx;

    if (!fnv1a_ctx || !htable_ctx || !logger_ctx) {
        if (logger_ctx)
            ST_LOGGERCTX_CALL(logger_ctx, error,
             "%s_%s: Unable to get required module contexts", 
             st_module_subsystem, st_module_name);
        else
            fprintf(stderr,
             "%s_%s: Unable to get logger context\n", st_module_subsystem,
             st_module_name);

        return NULL;
    }

    monitor_ctx = (st_monitorctx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_monitorctx_t), NULL, &monitorctx_funcs,
     (st_object_dtor_t)st_monitor_quit);

    if (!monitor_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create monitor context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    monitor_ctx->fnv1a_ctx = fnv1a_ctx;
    monitor_ctx->htable_ctx = htable_ctx;
    monitor_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Monitor manager context initialized", st_module_subsystem, 
     st_module_name);

    return monitor_ctx;
}

static void st_monitor_quit(st_monitorctx_t *monitor_ctx) {
    ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, info,
     "%s_%s: Monitor manager context destroyed", st_module_subsystem, 
     st_module_name);
    free(monitor_ctx);
}

static void st_monitor_destroy(st_monitor_t *monitor) {
    if (monitor->handle)
        XCloseDisplay(monitor->handle);
    if (monitor->userdata)
        ST_HTABLE_CALL(monitor->userdata, destroy);
    free(monitor);
}

static unsigned st_monitor_get_monitors_count(
 const st_monitorctx_t *monitor_ctx) {
    unsigned monitors_count;
    Display *display = XOpenDisplay(NULL);

    if (!display) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Unable to open default display", st_module_subsystem,
         st_module_name);

        return 0;
    }

    monitors_count = (unsigned)ScreenCount(display);
    XCloseDisplay(display);

    return monitors_count;
}

static st_monitor_t *st_monitor_open(st_monitorctx_t *monitor_ctx,
 unsigned index) {
    char          display_name[DISPLAY_NAME_SIZE_MAX];
    st_monitor_t *monitor;
    Window        root_window;
    int           ret = snprintf(display_name, DISPLAY_NAME_SIZE_MAX, ":0.%u", 
     index);

    if (ret < 0 || ret == DISPLAY_NAME_SIZE_MAX) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Unable to construct display name for display with index %u",
         st_module_subsystem, st_module_name, index);

        return NULL;
    }

    monitor = (st_monitor_t *)st_object_new(sizeof(st_monitor_t), 
     &monitor_funcs, (st_object_dtor_t)st_monitor_destroy, 
     (st_object_t *)monitor_ctx);

    if (!monitor) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Unable to allocate memory for monitor structure",
         st_module_subsystem, st_module_name);

        return NULL;
    }

    monitor->handle = XOpenDisplay(display_name);

    if (!monitor->handle) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Unable to open display", st_module_subsystem, st_module_name);
        st_object_destroy((st_object_t *)monitor);

        return NULL;
    }

    root_window = DefaultRootWindow(monitor->handle);
    monitor->index = index;

    monitor->userdata = ST_HTABLECTX_CALL(monitor_ctx->htable_ctx, create,
     (unsigned int (*)(const void *))ST_FNV1ACTX_CALL(
        monitor_ctx->fnv1a_ctx, 
        get_u32hashstr_func
     ),
     st_keyeqfunc, free, NULL);
    if (!monitor->userdata) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Unable to create userdata hashtable", st_module_subsystem,
         st_module_name);
        st_object_destroy((st_object_t *)monitor);

        return NULL;
    }
    st_monitor_set_userdata(monitor, "root_window", root_window);

    return monitor;
}

static unsigned st_monitor_get_width(const st_monitor_t *monitor) {
    int width = XDisplayWidth(monitor->handle, (int)monitor->index);

    return width > 0 ? (unsigned)width : 0u;
}

static unsigned st_monitor_get_height(const st_monitor_t *monitor) {
    int height = XDisplayHeight(monitor->handle, (int)monitor->index);

    return height > 0 ? (unsigned)height : 0u;
}

static void *st_monitor_get_handle(const st_monitor_t *monitor) {
    return monitor->handle;
}

static void st_monitor_set_userdata(const st_monitor_t *monitor,
 const char *key, uintptr_t value) {
    st_monitorctx_t *monitor_ctx = (st_monitorctx_t *)st_object_get_owner(
     (const st_object_t *)monitor);
    char            *keydup = strdup(key);

    if (keydup) {
        ST_HTABLE_CALL(monitor->userdata, insert, NULL, keydup, (void *)value);
    } else {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
             "%s_%s: Unable to allocate memory for userdata key \"%s\": %s",
             st_module_subsystem, st_module_name, key, errbuf);
    }
}

static bool st_monitor_get_userdata(const st_monitor_t *monitor, uintptr_t *dst,
 const char *key) {
    st_htiter_t it;
    void       *userdata;

    if (!ST_HTABLE_CALL(monitor->userdata, find, &it, key))
        return false;

    userdata = ST_HTITER_CALL(&it, get_value);
    *dst = (uintptr_t)userdata;

    return true;
}
