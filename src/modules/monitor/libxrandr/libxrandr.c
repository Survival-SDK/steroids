#include "libxrandr.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ERRMSGBUF_SIZE 128

static st_monitorctx_t *st_monitor_init(const st_param_t params[]);
static void st_monitor_quit(st_monitorctx_t *monitor_ctx);
static void st_monitor_destroy(st_monitor_t *monitor);

static unsigned st_monitor_get_monitors_count(
 const st_monitorctx_t *monitor_ctx);
static st_monitor_t *st_monitor_open(st_monitorctx_t *monitor_ctx,
 unsigned index);
static unsigned st_monitor_get_width(const st_monitor_t *monitor);
static unsigned st_monitor_get_height(const st_monitor_t *monitor);
static unsigned st_monitor_get_index(const st_monitor_t *monitor);
static const char *st_monitor_get_name(const st_monitor_t *monitor);
static bool st_monitor_is_primary(const st_monitor_t *monitor);
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
    .get_index    = st_monitor_get_index,
    .get_name     = st_monitor_get_name,
    .is_primary   = st_monitor_is_primary,
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

st_moddata_t *st_module_monitor_libxrandr_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("monitor", "libxrandr", ST_MODULE_TYPE, mod_prereqs,
     st_monitor_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_monitor_libxrandr_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "monitor";
static const char *st_module_name = "libxrandr";

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
    Display         *display;

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

    display = XOpenDisplay(NULL);
    if (!display) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to open default display", st_module_subsystem,
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

        XCloseDisplay(display);

        return NULL;
    }

    monitor_ctx->fnv1a_ctx = fnv1a_ctx;
    monitor_ctx->htable_ctx = htable_ctx;
    monitor_ctx->logger_ctx = logger_ctx;
    monitor_ctx->display = display;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Monitor manager context initialized", st_module_subsystem, 
     st_module_name);

    return monitor_ctx;
}

static void st_monitor_quit(st_monitorctx_t *monitor_ctx) {
    ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, info,
     "%s_%s: Monitor manager context destroyed", st_module_subsystem, 
     st_module_name);
    XCloseDisplay(monitor_ctx->display);
    free(monitor_ctx);
}

static void st_monitor_destroy(st_monitor_t *monitor) {
    if (monitor->userdata)
        ST_HTABLE_CALL(monitor->userdata, destroy);
    if (monitor->name)
        XFree(monitor->name);
        
    free(monitor);
}

static XRRMonitorInfo *get_monitors_info(const st_monitorctx_t *monitor_ctx, unsigned *monitors_count) {
    int            monitors_count_int;
    Window          root_window;
    XRRMonitorInfo *monitors_info;

    root_window = DefaultRootWindow(monitor_ctx->display);
    monitors_info = XRRGetMonitors(monitor_ctx->display, root_window, True, 
     &monitors_count_int);

    if (monitors_count)
        *monitors_count = monitors_info ? monitors_count_int : 0;

    return monitors_info;
}

static unsigned st_monitor_get_monitors_count(
 const st_monitorctx_t *monitor_ctx) {
    unsigned monitors_count = 0;
    XRRMonitorInfo *monitors_info = get_monitors_info(monitor_ctx, &monitors_count);

    if (monitors_info)
        XRRFreeMonitors(monitors_info);

    return monitors_count;
}

static st_monitor_t *st_monitor_open(st_monitorctx_t *monitor_ctx,
 unsigned index) {
    st_monitor_t   *monitor;
    unsigned        monitors_count = 0;
    XRRMonitorInfo *monitors_info;

    monitors_info = get_monitors_info(monitor_ctx, &monitors_count);
    if (!monitors_info) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Unable to get monitors information", st_module_subsystem,
         st_module_name);

        return NULL;
    }
    
    if (index >= monitors_count) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Monitor index %u is out of range", st_module_subsystem,
         st_module_name, index);

        goto monitor_index_out_of_range;
    }
    
    monitor = (st_monitor_t *)st_object_new(sizeof(st_monitor_t), 
     &monitor_funcs, (st_object_dtor_t)st_monitor_destroy, 
     (st_object_t *)monitor_ctx);

    if (!monitor) {
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, error,
         "%s_%s: Unable to allocate memory for monitor structure",
         st_module_subsystem, st_module_name);

        goto monitor_creation_failed;
    }

    monitor->handle = monitor_ctx->display;
    monitor->index = index;
    /* TODO(edomin): Add monitor is_primary flag. We need get it from 
     * monitors_info[index].primary with !! cast to bool.
     */
    monitor->width = monitors_info[index].width;
    monitor->height = monitors_info[index].height;
    monitor->name = XGetAtomName(monitor_ctx->display, 
     monitors_info[index].name);
    if (!monitor->name)
        ST_LOGGERCTX_CALL(monitor_ctx->logger_ctx, warning,
         "%s_%s: Unable to get monitor name for index %u. Monitor name is not "
         "available for this monitor on this run", st_module_subsystem,
         st_module_name, index);

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

        goto userdata_creation_failed;
    }
    st_monitor_set_userdata(monitor, "root_window", 
     DefaultRootWindow(monitor_ctx->display));
    st_monitor_set_userdata(monitor, "x", monitors_info[index].x);
    st_monitor_set_userdata(monitor, "y", monitors_info[index].y);

    XRRFreeMonitors(monitors_info);

    return monitor;

userdata_creation_failed:
    st_object_destroy((st_object_t *)monitor);

monitor_creation_failed:
monitor_index_out_of_range:
    XRRFreeMonitors(monitors_info);

    return NULL;
}

static unsigned st_monitor_get_width(const st_monitor_t *monitor) {
    return monitor->width;
}

static unsigned st_monitor_get_height(const st_monitor_t *monitor) {
    return monitor->height;
}

static unsigned st_monitor_get_index(const st_monitor_t *monitor) {
    return monitor->index;
}

static const char *st_monitor_get_name(const st_monitor_t *monitor) {
    return monitor->name;
}

static bool st_monitor_is_primary(const st_monitor_t *monitor) {
    return monitor->is_primary;
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
