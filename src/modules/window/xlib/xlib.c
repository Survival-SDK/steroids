#include "xlib.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h> // NOLINT(llvm-include-order)
#include <X11/XKBlib.h> // NOLINT(llvm-include-order)
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ATOM_BITS 32

static st_windowctx_t *st_window_init(const st_param_t params[]);
static void st_window_quit(st_windowctx_t *window_ctx);

static st_window_t *st_window_create(st_windowctx_t *window_ctx,
 st_monitor_t *monitor, int x, int y, unsigned width, unsigned height,
 bool fullscreen, const char *title);
static void st_window_process(st_windowctx_t *window_ctx);

static void st_window_destroy(st_window_t *window);
static bool st_window_xed(const st_window_t *window);
static st_monitor_t *st_window_get_monitor(const st_window_t *window);
static void *st_window_get_handle(const st_window_t *window);
static unsigned st_window_get_width(const st_window_t *window);
static unsigned st_window_get_height(const st_window_t *window);

static st_windowctx_funcs_t windowctx_funcs = {
    st_modctx_funcs,
    .create  = st_window_create,
    .process = st_window_process,
};

static st_window_funcs_t window_funcs = {
    st_object_funcs,
    .xed         = st_window_xed,
    .get_monitor = st_window_get_monitor,
    .get_handle  = st_window_get_handle,
    .get_width   = st_window_get_width,
    .get_height  = st_window_get_height,
};

static const st_modprerq_t mod_prereqs[] = {
    {"events", NULL},
    {"logger", NULL},
    {"monitor", NULL},
    {0}
};

st_moddata_t *st_module_window_xlib_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("window", "xlib", ST_MODULE_TYPE, mod_prereqs,
     st_window_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_window_xlib_init(modsmgr);
}
#endif

static void st_window_free(void *window) {
    st_window_t *win = (st_window_t *)window;

    XCloseIM(win->input_method);
    XDestroyWindow((Display *)ST_MONITOR_CALL(win->monitor, get_handle),
     win->handle);
    // ST_OBJECT_CALL(win->monitor, destroy);
    // ST_OBJECT_CALL(win, destroy);
}

static st_windowctx_t *st_window_init(const st_param_t params[]) {
    st_windowctx_t *window_ctx;
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx;

    if (!modsmgr)
        return NULL;

    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    window_ctx = (st_windowctx_t *)st_modctx_new("window", "xlib",
     sizeof(st_windowctx_t), NULL, &windowctx_funcs,
     (st_object_dtor_t)st_window_quit);
    if (!window_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "window_xlib: Unable to create new window ctx object");

        return NULL;
    }

    window_ctx->modsmgr = modsmgr;
    window_ctx->logger_ctx = logger_ctx;
    window_ctx->events_ctx = (st_eventsctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "events", NULL);
    if (!window_ctx->events_ctx) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to get events context");

        goto get_events_ctx_fail;
    }

    window_ctx->monitor_ctx = (st_monitorctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "monitor", NULL);
    if (!window_ctx->monitor_ctx) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to get monitor context");

        goto get_monitor_ctx_fail;
    }

    window_ctx->windows = st_dlist_create(sizeof(st_window_t), st_window_free);
    if (!window_ctx->windows) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to create list for windows entries");

        goto create_windows_list_fail;
    }

    /* Register event types */
    window_ctx->evtypes[EV_MOUSE_PRESS] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_mouse_press",
     sizeof(st_evwinunsigned_t));
    window_ctx->evtypes[EV_MOUSE_RELEASE] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_mouse_release",
     sizeof(st_evwinunsigned_t));
    window_ctx->evtypes[EV_MOUSE_WHEEL] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_mouse_wheel",
     sizeof(st_evwininteger_t));
    window_ctx->evtypes[EV_MOUSE_MOVE] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_mouse_move",
     sizeof(st_evwinuvec2_t));
    window_ctx->evtypes[EV_MOUSE_ENTER] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_mouse_enter",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_MOUSE_LEAVE] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_mouse_leave",
     sizeof(st_evwinnoargs_t));

    window_ctx->evtypes[EV_KEY_PRESS] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_key_press",
     sizeof(st_evwinu64_t));
    window_ctx->evtypes[EV_KEY_RELEASE] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_key_release",
     sizeof(st_evwinu64_t));
    window_ctx->evtypes[EV_KEY_INPUT] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_key_input",
     sizeof(st_evwinsymbol_t));

    window_ctx->evtypes[EV_FOCUS_IN] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_focus_in",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_FOCUS_OUT] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_focus_out",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_RESIZE] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_resize",
     sizeof(st_evwinuvec2_t));
    window_ctx->evtypes[EV_PLACE_ON_TOP] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_place_on_top",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_PLACE_ON_BOTTOM] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_place_on_bottom",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_CREATE] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_create",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_DESTROY] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_destroy",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_SHOW] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_show",
     sizeof(st_evwinnoargs_t));
    window_ctx->evtypes[EV_HIDE] = ST_EVENTSCTX_CALL(
     window_ctx->events_ctx, register_type, "window_hide",
     sizeof(st_evwinnoargs_t));

    ST_LOGGERCTX_CALL(window_ctx->logger_ctx, info,
     "window_xlib: Windows mgr initialized");

    return window_ctx;

create_windows_list_fail:
get_monitor_ctx_fail:
get_events_ctx_fail:
    free(window_ctx);

    return NULL;   
}

static void st_window_quit(st_windowctx_t *window_ctx) {
    if (window_ctx->windows)
        st_dlist_destroy(window_ctx->windows);

    ST_LOGGERCTX_CALL(window_ctx->logger_ctx, info,
     "window_xlib: Windows mgr destroyed");

    free(window_ctx);
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

static void fullscreen_window(st_windowctx_t *window_ctx, Window window, 
 const st_monitor_t *monitor) {
    XWindowAttributes attrs;
    XEvent            event = {0};
    Atom              net_wm_fullscreen_monitors;
    Bool              ewmh_supported = False;
    Display          *display = (Display *)ST_MONITOR_CALL(monitor, get_handle);

    XGetWindowAttributes(display, window, &attrs);

    net_wm_fullscreen_monitors = XInternAtom(display, 
     "_NET_WM_FULLSCREEN_MONITORS", False);

    if (!is_ewmh_supported(display, attrs.root, net_wm_fullscreen_monitors)) {
        uintptr_t monitor_x;
        uintptr_t monitor_y;

        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, warning,
         "window_xlib: EWMH _NET_WM_FULLSCREEN_MONITORS is NOT supported, "
         "using fallback XMoveResizeWindow for monitor %u", 
         ST_MONITOR_CALL(monitor, get_index));

        if (!ST_MONITOR_CALL(monitor, get_userdata, &monitor_x, "x") ||
         !ST_MONITOR_CALL(monitor, get_userdata, &monitor_y, "y")) {
            ST_LOGGERCTX_CALL(window_ctx->logger_ctx, warning,
             "window_xlib: Unable to get monitor coordinates in display space. "
             "Window will be placed on the screen, but not fullscreened");

            return;
        }
        
        XMoveResizeWindow(display, window, 
         monitor_x, 
         monitor_y, 
         ST_MONITOR_CALL(monitor, get_width), 
         ST_MONITOR_CALL(monitor, get_height));
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

static st_window_t *st_window_create(st_windowctx_t *window_ctx,
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
    st_dlnode_t         *node;
    uintptr_t            root_window;
    Display             *display = (Display *)ST_MONITOR_CALL(monitor,
     get_handle);
    uintptr_t            monitor_x;
    uintptr_t            monitor_y;

    if (ST_MONITORCTX_CALL(window_ctx->monitor_ctx, get_monitors_count) > 1
     && !ST_MONITOR_CALL(monitor, is_primary) 
     && getenv("WAYLAND_DISPLAY")) {
        if (fullscreen) {
            ST_LOGGERCTX_CALL(window_ctx->logger_ctx, warning,
             "window_xlib: Fullscreen is not supported on XWayland non-primary "
             "monitor. Window will be windowed");

            fullscreen = false;
        }
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, warning,
         "window_xlib: Manually placing window on XWayland non-primary monitor "
         "is not supported. Window will be placed on the primary or first "
         "available monitor");
    }
    
    if (!ST_MONITOR_CALL(monitor, get_userdata, &root_window, "root_window")) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to get root window from monitor");

        return NULL;
    }

    if (!ST_MONITOR_CALL(monitor, get_userdata, &monitor_x, "x") ||
     !ST_MONITOR_CALL(monitor, get_userdata, &monitor_y, "y")) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to get monitor coordinates in display space");

        return NULL;
    }

    window = (st_window_t *)st_object_new(sizeof(st_window_t), &window_funcs, 
     (st_object_dtor_t)st_window_destroy, (st_object_t *)window_ctx);
    if (!window) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to create new window object");

        return NULL;
    }

    window->handle = XCreateWindow(display, root_window, monitor_x + x, 
     monitor_y + y, width, height, 0, CopyFromParent, InputOutput, 
     CopyFromParent, CWEventMask, &event_attrs); // NOLINT(hicpp-signed-bitwise)
    if (!window->handle) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to create window");

        goto create_window_fail;
    }

    XChangeWindowAttributes(display, window->handle, CWOverrideRedirect,
     &override_redirect_attrs);  // NOLINT(hicpp-signed-bitwise)

    window->wm_delete_msg = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window->handle, &window->wm_delete_msg, 1);
    window->xed = false;

    XSetWMHints(display, window->handle, &hints);
    XStoreName(display, window->handle, title);

    if (fullscreen) {
        Atom net_wm_fullscreen_monitors = XInternAtom(display, 
         "_NET_WM_FULLSCREEN_MONITORS", False);
    
        if (is_ewmh_supported(display, root_window, net_wm_fullscreen_monitors)
         ) {
            unsigned monitor_index = ST_MONITOR_CALL(monitor, get_index);
            long     monitors[4] = { 
                monitor_index, 
                monitor_index, 
                monitor_index,
                monitor_index,
            };

            ST_LOGGERCTX_CALL(window_ctx->logger_ctx, debug,
             "window_xlib: EWMH _NET_WM_FULLSCREEN_MONITORS is supported");
            
            XChangeProperty(display, window->handle, net_wm_fullscreen_monitors, 
            XA_CARDINAL, ATOM_BITS, PropModeReplace, (unsigned char*)monitors, 
             4);
            XFlush(display);
        }
    }

    XMapWindow(display, window->handle);

    if (fullscreen) {
        fullscreen_window(window_ctx, window->handle, monitor);
    } else {
        XChangeProperty(display, window->handle,
         XInternAtom(display, "_HILDON_NON_COMPOSITED_WINDOW", False),
         XA_INTEGER, ATOM_BITS, PropModeReplace, (unsigned char*)(int[]){1}, 1);
    }

    window->input_method = XOpenIM(display, NULL, NULL, NULL);
    if (!window->input_method) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to open X input method");

        goto open_im_fail;
    }

    if (XGetIMValues(window->input_method, XNQueryInputStyle, &im_styles, NULL)
     != NULL || !im_styles) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to get input method styles");

        goto get_im_values_fail;
    }

    for (int i = 0; i < im_styles->count_styles; i++) {
        XIMStyle style = im_styles->supported_styles[i];

        if (style == ((unsigned long)XIMPreeditNothing | (unsigned long)XIMStatusNothing)) {
            im_best_match_style = style;
            break;
        }
    }

    XFree(im_styles);

    if (!im_best_match_style) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to get best input method style");

        goto best_match_fail;
    }

    window->input_context = XCreateIC(window->input_method, XNInputStyle,
     im_best_match_style, XNClientWindow, window->handle, XNFocusWindow,
     window->handle, NULL);
    if (!window->input_context) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to create input context");

        goto create_ic_fail;
    }

    XkbSetDetectableAutoRepeat(display, true, NULL);

    window->monitor = monitor;
    window->width = width;
    window->height = height;

    node = st_dlist_push_back(window_ctx->windows, window);
    if (!node) {
        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, error,
         "window_xlib: Unable to create list entry for window");

        goto dlist_push_back_fail;
    }

    return window;

dlist_push_back_fail:
create_ic_fail:
best_match_fail:
get_im_values_fail:
    XCloseIM(window->input_method);
open_im_fail:
    XDestroyWindow(display, window->handle);
create_window_fail:
    ST_OBJECT_CALL(window, destroy);

    return NULL;
}

static void st_window_destroy(st_window_t *window) {
    st_windowctx_t *window_ctx = (st_windowctx_t *)st_object_get_owner(
     (st_object_t *)window);
    st_dlnode_t    *node = st_dlist_get_head(window_ctx->windows);

    while (node) {
        if (st_dlist_get_data(node) == window) {
            st_dlist_remove(node);

            break;
        }

        node = st_dlist_get_next(node);
    }
}

static st_window_t *get_window_by_xwindow(st_windowctx_t *window_ctx,
 Window xwindow) {
    st_dlnode_t *node = st_dlist_get_head(window_ctx->windows);

    while (node) {
        st_window_t *window = st_dlist_get_data(node);

        if (window->handle == xwindow)
            return window;

        node = st_dlist_get_next(node);
    }

    return NULL;
}

static void st_window_process(st_windowctx_t *window_ctx) {
    st_dlnode_t *node = st_dlist_get_head(window_ctx->windows);

    while (node) {
        st_window_t *window = st_dlist_get_data(node);
        Display     *display = (Display *)ST_MONITOR_CALL(window->monitor,
         get_handle);

        while (XPending(display)) {
            XEvent xevent;

            XNextEvent(display, &xevent);
            switch (xevent.type) {
                case ClientMessage: {
                    st_window_t *event_window = get_window_by_xwindow(
                     window_ctx, xevent.xclient.window);

                    if (xevent.xclient.data.l[0] ==
                     (long)event_window->wm_delete_msg)
                        event_window->xed = true;

                    break;
                }
                case ButtonPress: {
                    if (xevent.xbutton.button == Button4 ||
                     xevent.xbutton.button == Button5) {
                        st_evwininteger_t event = {
                            .window = get_window_by_xwindow(window_ctx,
                             xevent.xbutton.window),
                            .value = (xevent.xbutton.button == Button4)
                                ? 1
                                : -1,
                        };
                        ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                         window_ctx->evtypes[EV_MOUSE_WHEEL], &event);
                    } else {
                        st_evwinunsigned_t event = {
                            .window = get_window_by_xwindow(window_ctx,
                             xevent.xbutton.window),
                            .value = xevent.xbutton.button - 1,
                        };
                        ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                         window_ctx->evtypes[EV_MOUSE_PRESS], &event);
                    }

                    break;
                }
                case ButtonRelease: {
                    st_evwinunsigned_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xbutton.window),
                        .value = xevent.xbutton.button - 1,
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_MOUSE_RELEASE], &event);

                    break;
                }
                case MotionNotify: {
                    st_evwinuvec2_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xmotion.window),
                        .hvalue = (unsigned)xevent.xmotion.x,
                        .vvalue = (unsigned)xevent.xmotion.y,
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_MOUSE_MOVE], &event);
                    break;
                }
                case EnterNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcrossing.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_MOUSE_ENTER], &event);
                    break;
                }
                case LeaveNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcrossing.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_MOUSE_LEAVE], &event);
                    break;
                }
                case KeyPress: {
                    st_evwinsymbol_t input_event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xkey.window),
                        .value = "\0\0\0\0",
                    };
                    Status           status = 0;
                    st_evwinu64_t    press_event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xkey.window),
                        .value = XkbKeycodeToKeysym(display,
                         (unsigned char)xevent.xkey.keycode, 0, 0),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_KEY_PRESS], &press_event);

                    Xutf8LookupString(window->input_context, &xevent.xkey,
                     input_event.value, 4, 0, &status);
                    if (status == XBufferOverflow)
                        ST_LOGGERCTX_CALL(window_ctx->logger_ctx, warning,
                         "window_xlib: Buffer overflow on lookup inputted "
                         "UTF-8 character");
                    else if(status == XLookupChars)
                        ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                         window_ctx->evtypes[EV_KEY_INPUT], &input_event);

                    break;
                }
                case KeyRelease: {
                    st_evwinu64_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xkey.window),
                        .value = XkbKeycodeToKeysym(display,
                         (unsigned char)xevent.xkey.keycode, 0, 0),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_KEY_RELEASE], &event);

                    break;
                }
                case FocusIn: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xfocus.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_FOCUS_IN], &event);
                    break;
                }
                case FocusOut: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xfocus.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_FOCUS_OUT], &event);
                    break;
                }
                case ResizeRequest: {
                    st_evwinuvec2_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xresizerequest.window),
                        .hvalue = (unsigned)xevent.xresizerequest.width,
                        .vvalue = (unsigned)xevent.xresizerequest.height,
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_RESIZE], &event);
                    break;
                }
                case CirculateNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcirculate.window),
                    };
                    if (xevent.xcirculate.place == PlaceOnTop)
                        ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                         window_ctx->evtypes[EV_PLACE_ON_TOP], &event);
                    else
                        ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                         window_ctx->evtypes[EV_PLACE_ON_BOTTOM], &event);
                    break;
                }
                case CreateNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcreatewindow.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_CREATE], &event);
                    break;
                }
                case DestroyNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xdestroywindow.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_DESTROY], &event);
                    break;
                }
                case MapNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xmap.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_SHOW], &event);
                    break;
                }
                case UnmapNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xunmap.window),
                    };
                    ST_EVENTSCTX_CALL(window_ctx->events_ctx, push,
                     window_ctx->evtypes[EV_HIDE], &event);
                    break;
                }
                case ConfigureNotify:
                case GravityNotify:
                default:
                    break;
            }
        }

        node = st_dlist_get_next(node);
    }
}

static bool st_window_xed(const st_window_t *window) {
    return window->xed;
}

static st_monitor_t *st_window_get_monitor(const st_window_t *window) {
    return window->monitor;
}

static void *st_window_get_handle(const st_window_t *window) {
    return (void *)(uintptr_t)window->handle;
}

static unsigned st_window_get_width(const st_window_t *window) {
    return window->width;
}

static unsigned st_window_get_height(const st_window_t *window) {
    return window->height;
}
