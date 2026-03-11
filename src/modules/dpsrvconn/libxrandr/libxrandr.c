#include "libxrandr.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <X11/Xatom.h> // NOLINT(llvm-include-order)
#include <X11/XKBlib.h> // NOLINT(llvm-include-order)
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ATOM_BITS 32

static st_dpsrvconnctx_t *st_dpsrvconn_init(const st_param_t params[]);
static void st_dpsrvconn_quit(st_dpsrvconnctx_t *dpsrvconn_ctx);
static void st_dpsrvconn_close_monitor(st_monitor_t *monitor);
static void st_dpsrvconn_window_destroy(st_window_t *window);

static int st_dpsrvconn_get_monitors_count(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
static int st_dpsrvconn_get_primary_monitor_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
static const st_monitor_t *st_dpsrvconn_get_monitor_by_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx, unsigned index);
static st_monitor_t *st_dpsrvconn_get_monitor_by_id(
 st_dpsrvconnctx_t *dpsrvconn_ctx, uintptr_t id);
static const st_monitor_t *st_dpsrvconn_get_primary_monitor(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
static st_window_t *st_dpsrvconn_open_window(st_dpsrvconnctx_t *dpsrvconn_ctx,
 st_monitor_t *monitor, int x, int y, unsigned width, unsigned height,
 bool fullscreen, const char *title);
static void st_dpsrvconn_process(st_dpsrvconnctx_t *dpsrvconn_ctx);

static unsigned st_dpsrvconn_get_monitor_width(const st_monitor_t *monitor);
static unsigned st_dpsrvconn_get_monitor_height(const st_monitor_t *monitor);
static unsigned st_dpsrvconn_get_monitor_index(const st_monitor_t *monitor);
static const char *st_dpsrvconn_get_monitor_name(const st_monitor_t *monitor);
static bool st_dpsrvconn_is_monitor_primary(const st_monitor_t *monitor);
static void *st_dpsrvconn_get_monitor_device_handle(
 const st_monitor_t *monitor);
//  static void st_monitor_set_userdata(const st_monitor_t *monitor,
//   const char *key, uintptr_t value);
//  static bool st_monitor_get_userdata(const st_monitor_t *monitor, uintptr_t *dst,
//   const char *key);

static bool st_dpsrvconn_is_window_xed(const st_window_t *window);
static const st_monitor_t *st_dpsrvconn_get_window_monitor(
 const st_window_t *window);
static void *st_dpsrvconn_get_window_handle(const st_window_t *window);
static unsigned st_dpsrvconn_get_window_width(const st_window_t *window);
static unsigned st_dpsrvconn_get_window_height(const st_window_t *window);

static st_dpsrvconnctx_funcs_t dpsrvconnctx_funcs = {
    st_modctx_funcs,
    .get_monitors_count        = st_dpsrvconn_get_monitors_count,
    .get_primary_monitor_index = st_dpsrvconn_get_primary_monitor_index,
    .get_monitor_by_index      = st_dpsrvconn_get_monitor_by_index,
    .get_monitor_by_id         = st_dpsrvconn_get_monitor_by_id,
    .get_primary_monitor       = st_dpsrvconn_get_primary_monitor,
    .open_window               = st_dpsrvconn_open_window,
    .process                   = st_dpsrvconn_process,
};

static st_monitor_funcs_t monitor_funcs = {
    st_object_funcs,
    .get_width         = st_dpsrvconn_get_monitor_width,
    .get_height        = st_dpsrvconn_get_monitor_height,
    .get_index         = st_dpsrvconn_get_monitor_index,
    .get_name          = st_dpsrvconn_get_monitor_name,
    .is_primary        = st_dpsrvconn_is_monitor_primary,
    .get_device_handle = st_dpsrvconn_get_monitor_device_handle,
};

static st_window_funcs_t window_funcs = {
    st_object_funcs,
    .xed         = st_dpsrvconn_is_window_xed,
    .get_monitor = st_dpsrvconn_get_window_monitor,
    .get_handle  = st_dpsrvconn_get_window_handle,
    .get_width   = st_dpsrvconn_get_window_width,
    .get_height  = st_dpsrvconn_get_window_height,
};

static const st_modprerq_t mod_prereqs[] = {
    // { "fnv1a", NULL, },
    // { "htable", NULL, },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_dpsrvconn_libxrandr_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("dpsrvconn", "libxrandr", ST_MODULE_TYPE, mod_prereqs,
     st_dpsrvconn_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_dpsrvconn_libxrandr_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "dpsrvconn";
static const char *st_module_name = "libxrandr";

static XRRMonitorInfo *get_monitors_info(Display *display, 
 unsigned *monitors_count) {
    int             monitors_count_int;
    XRRMonitorInfo *monitors_info;
    Window          root_window = DefaultRootWindow(display);

    monitors_info = XRRGetMonitors(display, root_window, True, 
     &monitors_count_int);

    if (monitors_count)
        *monitors_count = monitors_info ? monitors_count_int : 0;

    return monitors_info;
}

/* Monitor object destructor.
 * We need not to free memory because monitor object created with placement new 
 */
static void st_dpsrvconn_close_monitor(st_monitor_t *monitor) {
    if (monitor->name)
        XFree(monitor->name);
}

static void remove_monitor(st_dpsrvconnctx_t *dpsrvconn_ctx, unsigned index) {
    st_evmondisc_t event = {
        .id = dpsrvconn_ctx->monitors[index].output_id,
        .index = index,
    };
    st_dpsrvconn_close_monitor(&dpsrvconn_ctx->monitors[index]);
    dpsrvconn_ctx->monitors[index] 
     = dpsrvconn_ctx->monitors[dpsrvconn_ctx->monitors_count - 1];
    dpsrvconn_ctx->monitors_count--;

    ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
     dpsrvconn_ctx->evtypes[EV_MONITOR_DISCONNECTED], &event);
}

static void remove_missing_monitors(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 const RROutput monitors_ids[MONITORS_MAX], size_t found_count) {
    for (unsigned i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (!memmem(monitors_ids, sizeof(RROutput) * found_count, 
         &dpsrvconn_ctx->monitors[i].output_id, sizeof(RROutput))) {
            remove_monitor(dpsrvconn_ctx, i);
            /* recall func because iteration is broken after removing monitor */
            remove_missing_monitors(dpsrvconn_ctx, monitors_ids, found_count);
            break;
        }
    }
}

static void add_monitor(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 XRRMonitorInfo *monitor_info, unsigned index) {
    st_monitor_t  *new_monitor;
    st_evmonconn_t event = {0};
    if (dpsrvconn_ctx->monitors_count >= MONITORS_MAX) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Maximum number of monitors reached", st_module_subsystem,
         st_module_name);

        return;
    }

    new_monitor = (st_monitor_t *)st_object_placement_new(
     &dpsrvconn_ctx->monitors[dpsrvconn_ctx->monitors_count], &monitor_funcs,
     (st_object_dtor_t)st_dpsrvconn_close_monitor, 
     (st_object_t *)dpsrvconn_ctx);

    new_monitor->index = index;

    if (monitor_info->noutput > 1) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, debug,
        "%s_%s: Monitor has %d outputs (using first for identification)",
        st_module_subsystem, st_module_name, monitor_info->noutput);
    }
    new_monitor->output_id = monitor_info->noutput > 0 
        ? monitor_info->outputs[0] 
        : None;
    new_monitor->name = XGetAtomName(dpsrvconn_ctx->display, 
     monitor_info->name);
    new_monitor->is_primary = monitor_info->primary;
    new_monitor->x = monitor_info->x;
    new_monitor->y = monitor_info->y;
    new_monitor->width = monitor_info->width;
    new_monitor->height = monitor_info->height;

    dpsrvconn_ctx->monitors_count++;

    event.monitor = new_monitor;
    event.id = new_monitor->output_id;
    event.index = index;
    ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
     dpsrvconn_ctx->evtypes[EV_MONITOR_CONNECTED], &event);
}

static void add_new_monitors(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 const RROutput monitors_ids[MONITORS_MAX], size_t found_count, 
 XRRMonitorInfo *monitors_info) {
    RROutput current_monitors_ids[MONITORS_MAX] = {0};
    
    for (unsigned i = 0; i < dpsrvconn_ctx->monitors_count; i++)
        current_monitors_ids[i] = dpsrvconn_ctx->monitors[i].output_id;

    for (size_t i = 0; i < found_count; i++) {
        if (!memmem(current_monitors_ids, 
         sizeof(RROutput) * dpsrvconn_ctx->monitors_count, &monitors_ids[i], 
         sizeof(RROutput))) {
            add_monitor(dpsrvconn_ctx, &monitors_info[i], i);
            /* recall func because iteration is broken after adding monitor */
            add_new_monitors(dpsrvconn_ctx, monitors_ids, found_count, 
             monitors_info);
            break;
        }
    }
}

static void update_monitors_data(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 const XRRMonitorInfo *monitors_info, size_t found_count) {
    for (size_t i = 0; i < found_count; i++) {
        const XRRMonitorInfo monitor_info = monitors_info[i];
        RROutput             output_id = monitor_info.noutput > 0 
            ? monitor_info.outputs[0] 
            : None;
        st_monitor_t        *monitor = st_dpsrvconn_get_monitor_by_id(
         dpsrvconn_ctx, output_id);
        char                *current_monitor_name = monitor 
            ? XGetAtomName(dpsrvconn_ctx->display, monitor_info.name)
            : NULL;

        if (!monitor)
            continue;
        assert(strcmp(monitor->name, current_monitor_name) == 0);
        XFree(current_monitor_name);

        if (monitor->is_primary != monitor_info.primary) {
            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, info,
             "%s_%s: Monitor %s is %s primary", st_module_subsystem,
             st_module_name, monitor->name, 
             monitor_info.primary ? "now" : "no longer");
            monitor->is_primary = monitor_info.primary;
        }

        if (monitor->x != monitor_info.x || monitor->y != monitor_info.y) {
            monitor->x = monitor_info.x;
            monitor->y = monitor_info.y;
        }

        if (monitor->index != i) {
            st_evmonreidx_t event = {
                .monitor = monitor,
                .new_index = i,
            };
            monitor->index = i;
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MONITOR_REINDEX], &event);
        }

        if ((monitor->width != monitor_info.width) 
         || (monitor->height != monitor_info.height)) {
            st_evmonresize_t event = {
                .monitor    = monitor,
                .new_width  = monitor_info.width,
                .new_height = monitor_info.height,
            };

            monitor->width = monitor_info.width;
            monitor->height = monitor_info.height;
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MONITOR_RESIZE], &event);
        }
    }
}

static void update_monitors(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    unsigned        found_count = 0;
    XRRMonitorInfo *monitors_info = get_monitors_info(dpsrvconn_ctx->display,
     &found_count);
    RROutput        found_monitors_ids[MONITORS_MAX] = {0};

    if (!monitors_info) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to get monitors information", st_module_subsystem,
         st_module_name);
    }

    for (unsigned i = 0; i < found_count; i++) {
        if (i >= MONITORS_MAX) {
            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
             "%s_%s: Found more than maximum allowed %u monitors. This "
             "monitor will not be available", st_module_subsystem, 
             st_module_name, MONITORS_MAX);

            break;
        }

        found_monitors_ids[i] = monitors_info[i].noutput > 0 
            ? monitors_info[i].outputs[0] 
            : None;
    }

    found_count = found_count > MONITORS_MAX ? MONITORS_MAX : found_count;

    remove_missing_monitors(dpsrvconn_ctx, found_monitors_ids, found_count);
    add_new_monitors(dpsrvconn_ctx, found_monitors_ids, found_count, 
     monitors_info);
    update_monitors_data(dpsrvconn_ctx, monitors_info, found_count);

    XRRFreeMonitors(monitors_info);
}

static st_dpsrvconnctx_t *st_dpsrvconn_init(const st_param_t params[]) {
    st_modsmgr_t      *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t    *logger_ctx;
    st_eventsctx_t    *events_ctx;
    st_dpsrvconnctx_t *dpsrvconn_ctx;
    Display           *display;
    int               randr_event_base;

    /* Needed by XLib functions but not used */
    int               randr_error_base; 
    int               dri_opcode; 
    int               dri_event; 
    int               dri_error; 

    if (!modsmgr)
        return NULL;
    
    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    events_ctx = (st_eventsctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "events", NULL);
    if (!events_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get events context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    dpsrvconn_ctx = (st_dpsrvconnctx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_dpsrvconnctx_t), NULL, &dpsrvconnctx_funcs,
     (st_object_dtor_t)st_dpsrvconn_quit);
    if (!dpsrvconn_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create display server connection context", 
         st_module_subsystem, st_module_name);

        return NULL;
    }

    display = XOpenDisplay(NULL);
    if (!display) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to open default display", st_module_subsystem,
         st_module_name);

        goto open_display_fail;
    }

    if (!XRRQueryExtension(display, &randr_event_base, &randr_error_base)) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: XRandR extension not available", st_module_subsystem,
         st_module_name);

        goto query_extension_fail;
    }

    dpsrvconn_ctx->logger_ctx = logger_ctx;
    dpsrvconn_ctx->events_ctx = events_ctx;
    dpsrvconn_ctx->display = display;
    dpsrvconn_ctx->root_window = DefaultRootWindow(display);
    dpsrvconn_ctx->dri2_available = XQueryExtension(display, "DRI2", 
     &dri_opcode, &dri_event, &dri_error);
    dpsrvconn_ctx->dri3_available = XQueryExtension(display, "DRI3", 
     &dri_opcode, &dri_event, &dri_error);
    memset(dpsrvconn_ctx->monitors, 0, sizeof(st_monitor_t) * MONITORS_MAX);
    dpsrvconn_ctx->monitors_count = 0;
    update_monitors(dpsrvconn_ctx);

    dpsrvconn_ctx->randr_event_base = randr_event_base;
    XRRSelectInput(display, dpsrvconn_ctx->root_window, 
     RRScreenChangeNotifyMask | RROutputChangeNotifyMask 
     | RROutputPropertyNotifyMask | RRCrtcChangeNotifyMask);

    /* Register event types */
    dpsrvconn_ctx->evtypes[EV_MONITOR_CONNECTED] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_connected",
     sizeof(st_evmonconn_t));
    dpsrvconn_ctx->evtypes[EV_MONITOR_DISCONNECTED] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_disconnected",
     sizeof(st_evmondisc_t));
    dpsrvconn_ctx->evtypes[EV_MONITOR_REINDEX] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_reindex",
     sizeof(st_evmonreidx_t));
    dpsrvconn_ctx->evtypes[EV_MONITOR_RESIZE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_resize",
     sizeof(st_evmonresize_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_PRESS] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_press",
     sizeof(st_evwinunsigned_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_RELEASE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_release",
     sizeof(st_evwinunsigned_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_WHEEL] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_wheel",
     sizeof(st_evwininteger_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_MOVE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_move",
     sizeof(st_evwinuvec2_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_ENTER] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_enter",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_LEAVE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_leave",
     sizeof(st_evwinnoargs_t));

    dpsrvconn_ctx->evtypes[EV_KEY_PRESS] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "key_press",
     sizeof(st_evwinu64_t));
    dpsrvconn_ctx->evtypes[EV_KEY_RELEASE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "key_release",
     sizeof(st_evwinu64_t));
    dpsrvconn_ctx->evtypes[EV_KEY_INPUT] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "key_input",
     sizeof(st_evwinsymbol_t));

    dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_IN] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_focus_in",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_OUT] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_focus_out",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_RESIZE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_resize",
     sizeof(st_evwinuvec2_t));
    dpsrvconn_ctx->evtypes[EV_WIN_PLACE_ON_TOP] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_place_on_top",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_PLACE_ON_BOTTOM] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_place_on_bottom",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_CREATE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_create",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_DESTROY] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_destroy",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_SHOW] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_show",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_HIDE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_hide",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_MONITOR_CHANGED] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_monitor_changed",
     sizeof(st_evwinptr_t));

    dpsrvconn_ctx->xcontext = XrmUniqueQuark();

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Display server connection context initialized", 
     st_module_subsystem, st_module_name);

    return dpsrvconn_ctx;

query_extension_fail:
    XCloseDisplay(display);
open_display_fail:
    free(dpsrvconn_ctx);
    
    return NULL;
}

static void st_dpsrvconn_quit(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, info,
     "%s_%s: Display server connection context destroyed", st_module_subsystem, 
     st_module_name);
    XCloseDisplay(dpsrvconn_ctx->display);
    free(dpsrvconn_ctx);
}

static int st_dpsrvconn_get_monitors_count(
 const st_dpsrvconnctx_t *dpsrvconn_ctx) {
    return dpsrvconn_ctx->monitors_count;
}

static int st_dpsrvconn_get_primary_monitor_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx) {
    for (unsigned i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (dpsrvconn_ctx->monitors[i].is_primary)
            return i;
    }

    return -1;
}

static const st_monitor_t *st_dpsrvconn_get_monitor_by_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx, unsigned index) {
    if (index >= dpsrvconn_ctx->monitors_count) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Incorrect monitor index %u", st_module_subsystem,
         st_module_name, index);

        return NULL;
    }

    return &dpsrvconn_ctx->monitors[index];
}

static st_monitor_t *st_dpsrvconn_get_monitor_by_id(
 st_dpsrvconnctx_t *dpsrvconn_ctx, uintptr_t id) {
    for (unsigned i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (dpsrvconn_ctx->monitors[i].output_id == id)
            return &dpsrvconn_ctx->monitors[i];
    }

    return NULL;
}

static const st_monitor_t *st_dpsrvconn_get_primary_monitor(
 const st_dpsrvconnctx_t *dpsrvconn_ctx) {
    int primary_index = st_dpsrvconn_get_primary_monitor_index(dpsrvconn_ctx);

    if (primary_index == -1) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: No primary monitor found", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    return st_dpsrvconn_get_monitor_by_index(dpsrvconn_ctx, primary_index);
}

static bool is_ewmh_supported(Display *display, Window root_window, 
 const Atom net_wm_fullscreen_monitors) {
    Atom           net_supported;
    Atom           actual_type;
    int            actual_format;
    unsigned long  nitems;
    unsigned long  bytes_after;
    unsigned char *prop = NULL;
    Bool           ewmh_supported = False;
    
    net_supported = XInternAtom(display, "_NET_SUPPORTED", False);
    if (XGetWindowProperty(display, root_window, net_supported, 0, 1024, False, 
     XA_ATOM, &actual_type, &actual_format, &nitems, &bytes_after, &prop) 
     == Success && prop) {
        Atom *atoms = (Atom *)prop;
        for (unsigned long i = 0; i < nitems; i++) {
            if (atoms[i] == net_wm_fullscreen_monitors) {
                ewmh_supported = True;
                break;
            }
        }
        XFree(prop);
    }

    return !!ewmh_supported;
}

static st_monitor_t *xwindow_get_actual_monitor(
 st_dpsrvconnctx_t *dpsrvconn_ctx, Window window) {
    XWindowAttributes attrs;
    
    if (!XGetWindowAttributes(dpsrvconn_ctx->display, window, &attrs)) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to get XWindow attributes", st_module_subsystem,
         st_module_name);
        
        return NULL;
    }
    
    for (unsigned i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        st_monitor_t *monitor = &dpsrvconn_ctx->monitors[i];
        
        if (attrs.x >= monitor->x 
         && attrs.x < monitor->x + (int)monitor->width 
         && attrs.y >= monitor->y 
         && attrs.y < monitor->y + (int)monitor->height)
            return monitor;
    }
    
    ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
     "%s_%s: Window out of bounds of any monitor", st_module_subsystem, 
     st_module_name);
    
    return NULL;
}

static void fullscreen_window(st_dpsrvconnctx_t *dpsrvconn_ctx, Window window, 
 const st_monitor_t *monitor) {
    XWindowAttributes attrs;
    XEvent            event = {0};
    Atom              net_wm_fullscreen_monitors;
    Bool              ewmh_supported = False;
    Display          *display = dpsrvconn_ctx->display;

    XGetWindowAttributes(display, window, &attrs);

    net_wm_fullscreen_monitors = XInternAtom(display, 
     "_NET_WM_FULLSCREEN_MONITORS", False);

    if (!is_ewmh_supported(display, attrs.root, net_wm_fullscreen_monitors)) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
         "%s_%s: EWMH _NET_WM_FULLSCREEN_MONITORS is NOT supported, "
         "using fallback XMoveResizeWindow for monitor %u", st_module_subsystem,
         st_module_name, monitor->index);
        
        XMoveResizeWindow(display, window, monitor->x, monitor->y, 
         monitor->width, monitor->height);
        XFlush(display);
    }

    event.xclient.type = ClientMessage;
    event.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", false);
    event.xclient.display = display;
    event.xclient.window = window;
    event.xclient.format = ATOM_BITS;
    event.xclient.data.l[0] = 1; /* _NET_WM_STATE_ADD */
    event.xclient.data.l[1] = (long)XInternAtom(display,
     "_NET_WM_STATE_FULLSCREEN", false);
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 1; /* source indication: application */
    XSendEvent(display, attrs.root, false,
     SubstructureNotifyMask | SubstructureRedirectMask, &event); // NOLINT(hicpp-signed-bitwise)
    XFlush(display);
}

static st_window_t *st_dpsrvconn_open_window(st_dpsrvconnctx_t *dpsrvconn_ctx,
 st_monitor_t *monitor, int x, int y, unsigned width, unsigned height,
 bool fullscreen, const char *title) {
    XSetWindowAttributes event_attrs = {
        .event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | // NOLINT(hicpp-signed-bitwise)
         ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | // NOLINT(hicpp-signed-bitwise)
         PointerMotionMask | ExposureMask | StructureNotifyMask | // NOLINT(hicpp-signed-bitwise)
         ResizeRedirectMask | FocusChangeMask, // NOLINT(hicpp-signed-bitwise)
    };
    XSetWindowAttributes override_redirect_attrs = {
        .override_redirect = False,
    };
    XWMHints             hints = { .input = True, .flags = InputHint }; // NOLINT(hicpp-signed-bitwise)
    XIMStyles           *im_styles = NULL;
    XIMStyle             im_best_match_style = 0;
    st_window_t         *window;
    int                  set_context_result;

    if (ST_DPSRVCONNCTX_CALL(dpsrvconn_ctx, get_monitors_count) > 1
     && !ST_MONITOR_CALL(monitor, is_primary) 
     && getenv("WAYLAND_DISPLAY")) {
        if (fullscreen) {
            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
             "%s_%s: Fullscreen is not supported on XWayland non-primary "
             "monitor. Window will be windowed", st_module_subsystem,
             st_module_name);

            fullscreen = false;
        }
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
         "%s_%s: Manually placing window on XWayland non-primary monitor "
         "is not supported. Window will be placed on the primary or first "
         "available monitor", st_module_subsystem, st_module_name);
    }

    window = (st_window_t *)st_object_new(sizeof(st_window_t), &window_funcs,
     (st_object_dtor_t)st_dpsrvconn_window_destroy, 
     (st_object_t *)dpsrvconn_ctx);

    if (!window) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to allocate memory for window object", 
         st_module_subsystem, st_module_name);

        return NULL;
    }

    window->width = width;
    window->height = height;

    window->handle = XCreateWindow(dpsrvconn_ctx->display, 
     dpsrvconn_ctx->root_window, monitor->x + x, monitor->y + y, width, height, 
     0, CopyFromParent, InputOutput, CopyFromParent, CWEventMask, &event_attrs); // NOLINT(hicpp-signed-bitwise)
    if (!window->handle) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to create window", st_module_subsystem, st_module_name);

        goto create_window_fail;
    }

    XChangeWindowAttributes(dpsrvconn_ctx->display, window->handle, 
     CWOverrideRedirect, &override_redirect_attrs);  // NOLINT(hicpp-signed-bitwise)

    window->wm_delete_msg = XInternAtom(dpsrvconn_ctx->display, 
     "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpsrvconn_ctx->display, window->handle, 
     &window->wm_delete_msg, 1);
    window->xed = false;

    XSetWMHints(dpsrvconn_ctx->display, window->handle, &hints);
    XStoreName(dpsrvconn_ctx->display, window->handle, title);

    if (fullscreen) {
        Atom net_wm_fullscreen_monitors = XInternAtom(dpsrvconn_ctx->display, 
         "_NET_WM_FULLSCREEN_MONITORS", False);
    
        if (is_ewmh_supported(dpsrvconn_ctx->display, 
         dpsrvconn_ctx->root_window, net_wm_fullscreen_monitors)) {
            unsigned monitor_index = ST_MONITOR_CALL(monitor, get_index);
            long     monitors[4] = { 
                monitor_index, 
                monitor_index, 
                monitor_index,
                monitor_index,
            };

            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, debug,
             "%s_%s: EWMH _NET_WM_FULLSCREEN_MONITORS is supported", 
             st_module_subsystem, st_module_name);
            
            XChangeProperty(dpsrvconn_ctx->display, window->handle, 
             net_wm_fullscreen_monitors, XA_CARDINAL, ATOM_BITS, 
             PropModeReplace, (unsigned char*)monitors, 4);
            XFlush(dpsrvconn_ctx->display);
        }
    }

    XMapWindow(dpsrvconn_ctx->display, window->handle);

    if (fullscreen) {
        fullscreen_window(dpsrvconn_ctx, window->handle, monitor);
    } else {
        XChangeProperty(dpsrvconn_ctx->display, window->handle,
         XInternAtom(dpsrvconn_ctx->display, "_HILDON_NON_COMPOSITED_WINDOW", 
          False),
         XA_INTEGER, ATOM_BITS, PropModeReplace, (unsigned char*)(int[]){1}, 1);
    }

    window->input_method = XOpenIM(dpsrvconn_ctx->display, NULL, NULL, NULL);
    if (!window->input_method) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to open X input method", st_module_subsystem,
         st_module_name);

        goto open_im_fail;
    }

    if (XGetIMValues(window->input_method, XNQueryInputStyle, &im_styles, NULL)
     != NULL || !im_styles) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to get input method styles", st_module_subsystem,
         st_module_name);

        goto get_im_values_fail;
    }

    for (int i = 0; i < im_styles->count_styles; i++) {
        XIMStyle style = im_styles->supported_styles[i];

        if (style == (
         (unsigned long)XIMPreeditNothing | (unsigned long)XIMStatusNothing
        )) {
            im_best_match_style = style;
            break;
        }
    }

    XFree(im_styles);

    if (!im_best_match_style) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to get best input method style", st_module_subsystem,
         st_module_name);

        goto best_match_fail;
    }

    window->input_context = XCreateIC(window->input_method, XNInputStyle,
     im_best_match_style, XNClientWindow, window->handle, XNFocusWindow,
     window->handle, NULL);
    if (!window->input_context) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to create input context", st_module_subsystem,
         st_module_name);

        goto create_ic_fail;
    }

    XkbSetDetectableAutoRepeat(dpsrvconn_ctx->display, true, NULL);

    set_context_result = XSaveContext(dpsrvconn_ctx->display, window->handle, 
     dpsrvconn_ctx->xcontext, (XPointer)window);
    if (set_context_result != 0) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to set X11 context for window. Not enough memory", 
         st_module_subsystem, st_module_name);

        goto set_context_fail;
    }

    XFlush(dpsrvconn_ctx->display);
    XSync(dpsrvconn_ctx->display, False);

    window->monitor = xwindow_get_actual_monitor(dpsrvconn_ctx, window->handle);

    ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, debug,
     "%s_%s: Window created on monitor %s", st_module_subsystem, st_module_name, 
     window->monitor 
        ? (window->monitor->name ? window->monitor->name : "(unnamed)")
        : "(out of bounds of any monitor)"
    );

    return window;

set_context_fail:
create_ic_fail:
best_match_fail:
get_im_values_fail:
    XCloseIM(window->input_method);
open_im_fail:
    XDestroyWindow(dpsrvconn_ctx->display, window->handle);
create_window_fail:
    free(window);

    return NULL;
}

static void st_dpsrvconn_window_destroy(st_window_t *window) {
    st_dpsrvconnctx_t *dpsrvconn_ctx = (st_dpsrvconnctx_t *)st_object_get_owner(
     (st_object_t *)window);
    
    XDeleteContext(dpsrvconn_ctx->display, window->handle, 
     dpsrvconn_ctx->xcontext);
    XCloseIM(window->input_method);
    XDestroyWindow(dpsrvconn_ctx->display, window->handle);
    free(window);
}

static st_window_t *get_window_by_xwindow(st_dpsrvconnctx_t *dpsrvconn_ctx,
 Window xwindow) {
    XPointer data = NULL;

    if (XFindContext(dpsrvconn_ctx->display, xwindow, dpsrvconn_ctx->xcontext, 
     &data) == 0)
        return (st_window_t *)data;

    return NULL;
}

static inline void handle_rnotify_event(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 const XEvent *xevent) {
    XRRNotifyEvent *rrevent = (XRRNotifyEvent *)xevent;
    
    switch (rrevent->subtype) {
        case RRNotify_OutputChange: {
            XRROutputChangeNotifyEvent *output_event = 
             (XRROutputChangeNotifyEvent *)rrevent;
            
            if (output_event->connection == RR_Connected)
                ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, info,
                 "%s_%s: Monitor connected (output %lu)",
                 st_module_subsystem, st_module_name, output_event->output);
            else if (output_event->connection == RR_Disconnected)
                ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, info,
                 "%s_%s: Monitor disconnected (output %lu)",
                 st_module_subsystem, st_module_name, output_event->output);

            update_monitors(dpsrvconn_ctx);
            break;
        }
        case RRNotify_CrtcChange:
            update_monitors(dpsrvconn_ctx);
            break;

        default:
            break;
    }
}

static inline void handle_randr_event(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 XEvent *xevent) {
    int randr_event_type = xevent->type - dpsrvconn_ctx->randr_event_base;
    
    switch (randr_event_type) {
        case RRScreenChangeNotify:
            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, info,
             "%s_%s: Screen configuration changed, refreshing monitors list",
             st_module_subsystem, st_module_name);
            XRRUpdateConfiguration(xevent);
            update_monitors(dpsrvconn_ctx);
            break;
        case RRNotify:
            handle_rnotify_event(dpsrvconn_ctx, xevent);
            break;
    }
}

static inline void handle_x_event(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 XEvent *xevent) {
    switch (xevent->type) {
        case ClientMessage: {
            st_window_t *event_window = get_window_by_xwindow(
                dpsrvconn_ctx, xevent->xclient.window);

            if (!event_window) {
                ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
                 "%s_%s: Received ClientMessage for unknown window %u",
                 st_module_subsystem, st_module_name, xevent->xclient.window);
                break;
            }

            if (xevent->xclient.message_type 
                == XInternAtom(dpsrvconn_ctx->display, "WM_PROTOCOLS", False)
            ) {
                if (xevent->xclient.data.l[0] 
                 == (long)event_window->wm_delete_msg)
                    event_window->xed = true;
            }
            break;
        }
        case ButtonPress: {
            if (xevent->xbutton.button == Button4 
             || xevent->xbutton.button == Button5) {
                st_evwininteger_t event = {
                    .window = get_window_by_xwindow(dpsrvconn_ctx,
                     xevent->xbutton.window),
                    .value = (xevent->xbutton.button == Button4)
                        ? 1
                        : -1,
                };
                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_MOUSE_WHEEL], &event);
            } else {
                st_evwinunsigned_t event = {
                    .window = get_window_by_xwindow(dpsrvconn_ctx,
                     xevent->xbutton.window),
                    .value = xevent->xbutton.button - 1,
                };
                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_MOUSE_PRESS], &event);
            }

            break;
        }
        case ButtonRelease: {
            st_evwinunsigned_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xbutton.window),
                .value = xevent->xbutton.button - 1,
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MOUSE_RELEASE], &event);

            break;
        }
        case MotionNotify: {
            st_evwinuvec2_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xmotion.window),
                .hvalue = (unsigned)xevent->xmotion.x,
                .vvalue = (unsigned)xevent->xmotion.y,
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MOUSE_MOVE], &event);
            break;
        }
        case EnterNotify: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xcrossing.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MOUSE_ENTER], &event);
            break;
        }
        case LeaveNotify: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xcrossing.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MOUSE_LEAVE], &event);
            break;
        }
        case KeyPress: {
            st_window_t     *window = get_window_by_xwindow(dpsrvconn_ctx,
             xevent->xkey.window);
            st_evwinsymbol_t input_event = {
                .window = window,
                .value  = "\0\0\0\0",
            };
            Status           status = 0;
            st_evwinu64_t    press_event = {
                .window = window,
                .value  = XkbKeycodeToKeysym(dpsrvconn_ctx->display,
                 (unsigned char)xevent->xkey.keycode, 0, 0),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_KEY_PRESS], &press_event);

            Xutf8LookupString(window->input_context, &xevent->xkey,
             input_event.value, 4, 0, &status);
            if (status == XBufferOverflow)
                ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
                 "%s_%s: Buffer overflow on lookup inputted UTF-8 character", 
                 st_module_subsystem, st_module_name);
            else if(status == XLookupChars)
                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_KEY_INPUT], &input_event);

            break;
        }
        case KeyRelease: {
            st_evwinu64_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xkey.window),
                .value = XkbKeycodeToKeysym(dpsrvconn_ctx->display,
                 (unsigned char)xevent->xkey.keycode, 0, 0),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_KEY_RELEASE], &event);

            break;
        }
        case FocusIn: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xfocus.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_IN], &event);
            break;
        }
        case FocusOut: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xfocus.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_OUT], &event);
            break;
        }
        case ResizeRequest: {
            st_evwinuvec2_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xresizerequest.window),
                .hvalue = (unsigned)xevent->xresizerequest.width,
                .vvalue = (unsigned)xevent->xresizerequest.height,
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_RESIZE], &event);
            break;
        }
        case CirculateNotify: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xcirculate.window),
            };
            unsigned evtype_index = (xevent->xcirculate.place == PlaceOnTop)
                ? EV_WIN_PLACE_ON_TOP
                : EV_WIN_PLACE_ON_BOTTOM;
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[evtype_index], &event);
            break;
        }
        case CreateNotify: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xcreatewindow.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_CREATE], &event);
            break;
        }
        case DestroyNotify: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xdestroywindow.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_DESTROY], &event);
            break;
        }
        case MapNotify: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xmap.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_SHOW], &event);
            break;
        }
        case UnmapNotify: {
            st_evwinnoargs_t event = {
                .window = get_window_by_xwindow(dpsrvconn_ctx,
                 xevent->xunmap.window),
            };
            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_HIDE], &event);
            break;
        }
        case ConfigureNotify: {
            st_monitor_t *new_monitor;
            st_window_t  *window = get_window_by_xwindow(dpsrvconn_ctx, 
             xevent->xconfigure.window);

            if (!window)
                break;

            if (window->width != xevent->xconfigure.width 
             || window->height != xevent->xconfigure.height) {
                st_evwinuvec2_t event = {
                    .window = window,
                    .hvalue = xevent->xconfigure.width,
                    .vvalue = xevent->xconfigure.height,
                };
                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                dpsrvconn_ctx->evtypes[EV_WIN_RESIZE], &event);

                window->width  = xevent->xconfigure.width;
                window->height = xevent->xconfigure.height;
            }

            new_monitor = xwindow_get_actual_monitor(dpsrvconn_ctx, 
             window->handle);
            if (window->monitor != new_monitor) {
                st_evwinptr_t event = {
                    .window = window,
                    .ptr    = new_monitor,
                };
                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_WIN_MONITOR_CHANGED], &event);
                window->monitor = new_monitor;
            }

            break;
        }
        case GravityNotify:
        default:
            break;
    }
}


static void st_dpsrvconn_process(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    while (XPending(dpsrvconn_ctx->display)) {
        XEvent xevent;

        XNextEvent(dpsrvconn_ctx->display, &xevent);

        if (xevent.type >= dpsrvconn_ctx->randr_event_base) {
            handle_randr_event(dpsrvconn_ctx, &xevent);
            continue;
        }

        handle_x_event(dpsrvconn_ctx, &xevent);
    }
}

static unsigned st_dpsrvconn_get_monitor_width(const st_monitor_t *monitor) {
    return monitor->width;
}

static unsigned st_dpsrvconn_get_monitor_height(const st_monitor_t *monitor) {
    return monitor->height;
}

static unsigned st_dpsrvconn_get_monitor_index(const st_monitor_t *monitor) {
    return monitor->index;
}

static const char *st_dpsrvconn_get_monitor_name(const st_monitor_t *monitor) {
    return monitor->name;
}

static bool st_dpsrvconn_is_monitor_primary(const st_monitor_t *monitor) {
    return monitor->is_primary;
}

static void *st_dpsrvconn_get_monitor_device_handle(
 const st_monitor_t *monitor) {
    return ((st_dpsrvconnctx_t *)ST_MONITOR_CALL(monitor, get_owner))->display;
}

static bool st_dpsrvconn_is_window_xed(const st_window_t *window) {
    return window->xed;
}

static const st_monitor_t *st_dpsrvconn_get_window_monitor(
 const st_window_t *window) {
    return window->monitor;
}

static void *st_dpsrvconn_get_window_handle(const st_window_t *window) {
    return (void *)(uintptr_t)window->handle;
}

static unsigned st_dpsrvconn_get_window_width(const st_window_t *window) {
    return window->width;
}

static unsigned st_dpsrvconn_get_window_height(const st_window_t *window) {
    return window->height;
}
