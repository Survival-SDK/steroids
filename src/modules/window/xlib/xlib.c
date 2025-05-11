#include "xlib.h"

#include <stdio.h>

#include <X11/Xatom.h> // NOLINT(llvm-include-order)
#include <X11/XKBlib.h> // NOLINT(llvm-include-order)
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "steroids/modules/monitor.h"

#define ATOM_BITS 32

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_window_funcs_t window_funcs = {
    .destroy     = st_window_destroy,
    .xed         = st_window_xed,
    .get_monitor = st_window_get_monitor,
    .get_handle  = st_window_get_handle,
    .get_width   = st_window_get_width,
    .get_height  = st_window_get_height,
};

ST_MODULE_DEF_GET_FUNC(window_xlib)
ST_MODULE_DEF_INIT_FUNC(window_xlib)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_window_xlib_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_window_import_functions(st_modctx_t *window_ctx,
 st_modctx_t *events_ctx, st_modctx_t *logger_ctx) {
    st_window_xlib_t *module = window_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "window_xlib: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("window_xlib", events, register_type);
    ST_LOAD_FUNCTION_FROM_CTX("window_xlib", events, push);

    ST_LOAD_FUNCTION_FROM_CTX("window_xlib", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("window_xlib", logger, info);
    ST_LOAD_FUNCTION_FROM_CTX("window_xlib", logger, warning);

    return true;
}

static void st_window_free(void *window) {
    XCloseIM(((st_window_t *)window)->input_method);
    XDestroyWindow(
     ST_MONITOR_CALL(((st_window_t *)window)->monitor, get_handle),
     ((st_window_t *)window)->handle);
}

static st_modctx_t *st_window_init(st_modctx_t *events_ctx,
 st_modctx_t *logger_ctx, st_modctx_t *monitor_ctx) {
    st_modctx_t      *window_ctx;
    st_window_xlib_t *module;

    window_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_window_xlib_data, sizeof(st_window_xlib_t));

    if (!window_ctx)
        return NULL;

    window_ctx->funcs = &st_window_xlib_funcs;

    module = window_ctx->data;
    module->events.ctx = events_ctx;
    module->logger.ctx = logger_ctx;
    module->monitor.ctx = monitor_ctx;

    if (!st_window_import_functions(window_ctx, events_ctx, logger_ctx))
        goto fail;

    module->windows = st_dlist_create(sizeof(st_window_t), st_window_free);
    if (!module->windows) {
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to create list for windows entries");

        goto fail;
    }

    module->evtypes[EV_MOUSE_PRESS] = module->events.register_type(events_ctx,
     "window_mouse_press", sizeof(st_evwinunsigned_t));
    module->evtypes[EV_MOUSE_RELEASE] = module->events.register_type(events_ctx,
     "window_mouse_release", sizeof(st_evwinunsigned_t));
    module->evtypes[EV_MOUSE_WHEEL] = module->events.register_type(events_ctx,
     "window_mouse_wheel", sizeof(st_evwininteger_t));
    module->evtypes[EV_MOUSE_MOVE] = module->events.register_type(events_ctx,
     "window_mouse_move", sizeof(st_evwinuvec2_t));
    module->evtypes[EV_MOUSE_ENTER] = module->events.register_type(events_ctx,
     "window_mouse_enter", sizeof(st_evwinnoargs_t));
    module->evtypes[EV_MOUSE_LEAVE] = module->events.register_type(events_ctx,
     "window_mouse_leave", sizeof(st_evwinnoargs_t));

    module->evtypes[EV_KEY_PRESS] = module->events.register_type(events_ctx,
     "window_key_press", sizeof(st_evwinu64_t));
    module->evtypes[EV_KEY_RELEASE] = module->events.register_type(events_ctx,
     "window_key_release", sizeof(st_evwinu64_t));
    module->evtypes[EV_KEY_INPUT] = module->events.register_type(events_ctx,
     "window_key_input", sizeof(st_evwinsymbol_t));

    module->evtypes[EV_FOCUS_IN] = module->events.register_type(events_ctx,
     "window_focus_in", sizeof(st_evwinnoargs_t));
    module->evtypes[EV_FOCUS_OUT] = module->events.register_type(events_ctx,
     "window_focus_in", sizeof(st_evwinnoargs_t));
    module->evtypes[EV_RESIZE] = module->events.register_type(events_ctx,
     "window_resize", sizeof(st_evwinuvec2_t));
    module->evtypes[EV_PLACE_ON_TOP] = module->events.register_type(events_ctx,
     "window_place_on_top", sizeof(st_evwinnoargs_t));
    module->evtypes[EV_PLACE_ON_BOTTOM] = module->events.register_type(
     events_ctx, "window_place_on_bottom", sizeof(st_evwinnoargs_t));
    module->evtypes[EV_CREATE] = module->events.register_type(events_ctx,
     "window_create", sizeof(st_evwinnoargs_t));
    module->evtypes[EV_DESTROY] = module->events.register_type(events_ctx,
     "window_destroy", sizeof(st_evwinnoargs_t));
    // module->evtypes[EV_MOVE] = module->events.register_type(events_ctx,
    //  "window_move", sizeof(st_evwinmove_t));
    module->evtypes[EV_SHOW] = module->events.register_type(events_ctx,
     "window_show", sizeof(st_evwinnoargs_t));
    module->evtypes[EV_HIDE] = module->events.register_type(events_ctx,
     "window_hide", sizeof(st_evwinnoargs_t));

    module->logger.info(module->logger.ctx,
     "window_xlib: Windows mgr initialized");

    return window_ctx;

fail:
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, window_ctx);

    return NULL;
}

static void st_window_quit(st_modctx_t *window_ctx) {
    st_window_xlib_t *module = window_ctx->data;

    st_dlist_destroy(module->windows);
    module->windows = NULL;

    module->logger.info(module->logger.ctx,
     "window_xlib: Windows mgr destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, window_ctx);
}

void fullscreen_window(Display* display, Window window) {
    XWindowAttributes attrs;
    XEvent            event = {0};
    XGetWindowAttributes(display, window, &attrs);


    event.xclient.type = ClientMessage;
    event.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", false);
    event.xclient.display = display;
    event.xclient.window = window;
    event.xclient.format = ATOM_BITS;
    event.xclient.data.l[0] = 1;
    event.xclient.data.l[1] = (long)XInternAtom(display,
     "_NET_WM_STATE_FULLSCREEN", false);
    XSendEvent(display, attrs.root, false,
     SubstructureNotifyMask | SubstructureRedirectMask, &event); // NOLINT(hicpp-signed-bitwise)
}

static st_window_t *st_window_create(st_modctx_t *window_ctx,
 st_monitor_t *monitor, int x, int y, unsigned width, unsigned height,
 bool fullscreen, const char *title) {
    st_window_xlib_t    *module = window_ctx->data;
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
    st_window_t          window;
    st_dlnode_t         *node;
    uintptr_t            root_window;

    if (!ST_MONITOR_CALL(monitor, get_userdata, &root_window, "root_window")) {
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to get root window from monitor");

        return NULL;
    }

    window.handle = XCreateWindow(ST_MONITOR_CALL(monitor, get_handle),
     root_window, x, y, width, height, 0, CopyFromParent, InputOutput,
     CopyFromParent, CWEventMask, &event_attrs); // NOLINT(hicpp-signed-bitwise)
    if (!window.handle) {
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to create window");

        return NULL;
    }

    XChangeWindowAttributes(ST_MONITOR_CALL(monitor, get_handle), window.handle,
     CWOverrideRedirect, &override_redirect_attrs);  // NOLINT(hicpp-signed-bitwise)

    window.wm_delete_msg = XInternAtom(ST_MONITOR_CALL(monitor, get_handle),
     "WM_DELETE_WINDOW", False);
    XSetWMProtocols(ST_MONITOR_CALL(monitor, get_handle), window.handle,
     &window.wm_delete_msg, 1);
    window.xed = false;

    XSetWMHints(ST_MONITOR_CALL(monitor, get_handle), window.handle, &hints);
    XMapWindow(ST_MONITOR_CALL(monitor, get_handle), window.handle);
    XStoreName(ST_MONITOR_CALL(monitor, get_handle), window.handle, title);

    if (fullscreen) {
        fullscreen_window(ST_MONITOR_CALL(monitor, get_handle), window.handle);
    } else {
        XChangeProperty(ST_MONITOR_CALL(monitor, get_handle), window.handle,
         XInternAtom(ST_MONITOR_CALL(monitor, get_handle),
         "_HILDON_NON_COMPOSITED_WINDOW", False),
         XA_INTEGER, ATOM_BITS, PropModeReplace, (unsigned char*)(int[]){1}, 1);
    }

    window.input_method = XOpenIM(ST_MONITOR_CALL(monitor, get_handle), NULL,
     NULL, NULL);
    if (!window.input_method) {
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to open X input method");

        goto open_im_fail;
    }

    if (XGetIMValues(window.input_method, XNQueryInputStyle, &im_styles, NULL)
     != NULL || !im_styles) {
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to open X input method");

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
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to get best input method style");

        goto best_match_fail;
    }

    window.input_context = XCreateIC(window.input_method, XNInputStyle,
     im_best_match_style, XNClientWindow, window.handle, XNFocusWindow,
     window.handle, NULL);
    if (!window.input_context) {
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to create input context");

        goto create_ic_fail;;
    }

    XkbSetDetectableAutoRepeat(ST_MONITOR_CALL(monitor, get_handle), true,
     NULL);

    st_object_make(&window, window_ctx, &window_funcs);
    window.monitor = monitor;
    window.width = width;
    window.height = height;
    node = st_dlist_push_back(module->windows, &window);
    if (!node) {
        module->logger.error(module->logger.ctx,
         "window_xlib: Unable to create list entry for window");

        goto dlist_push_back_fail;
    }

    return st_dlist_get_data(node);

dlist_push_back_fail:
create_ic_fail:
best_match_fail:
get_im_values_fail:
    XCloseIM(window.input_method);
open_im_fail:
    XDestroyWindow(ST_MONITOR_CALL(window.monitor, get_handle), window.handle);

    return NULL;
}

static void st_window_destroy(st_window_t *window) {
    st_window_xlib_t *module = ((st_modctx_t *)st_object_get_owner(window))->data;
    st_dlnode_t      *node = st_dlist_get_head(module->windows);

    while (node) {
        if (st_dlist_get_data(node) == window) {
            st_dlist_remove(node);

            break;
        }

        node = st_dlist_get_next(node);
    }
}

static st_window_t *get_window_by_xwindow(st_modctx_t *window_ctx,
 Window xwindow) {
    st_window_xlib_t *module = window_ctx->data;
    st_dlnode_t      *node = st_dlist_get_head(module->windows);

    while (node) {
        st_window_t *window = st_dlist_get_data(node);

        if (window->handle == xwindow)
            return window;

        node = st_dlist_get_next(node);
    }

    return NULL;
}

static void st_window_process(st_modctx_t *window_ctx) {
    st_window_xlib_t *module = window_ctx->data;
    st_dlnode_t      *node = st_dlist_get_head(module->windows);

    while (node) {
        st_window_t *window = st_dlist_get_data(node);

        while (XPending(ST_MONITOR_CALL(window->monitor, get_handle))) {
            XEvent xevent;

            XNextEvent(ST_MONITOR_CALL(window->monitor, get_handle), &xevent);
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
                        module->events.push(module->events.ctx,
                         module->evtypes[EV_MOUSE_WHEEL], &event);
                    } else {
                        st_evwinunsigned_t event = {
                            .window = get_window_by_xwindow(window_ctx,
                             xevent.xbutton.window),
                            .value = xevent.xbutton.button - 1,
                        };
                        module->events.push(module->events.ctx,
                         module->evtypes[EV_MOUSE_PRESS], &event);
                    }

                    break;
                }
                case ButtonRelease: {
                    st_evwinunsigned_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xbutton.window),
                        .value = xevent.xbutton.button - 1,
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_MOUSE_RELEASE], &event);

                    break;
                }
                case MotionNotify: {
                    st_evwinuvec2_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xmotion.window),
                        .hvalue = (unsigned)xevent.xmotion.x,
                        .vvalue = (unsigned)xevent.xmotion.y,
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_MOUSE_MOVE], &event);
                    break;
                }
                case EnterNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcrossing.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_MOUSE_ENTER], &event);
                    break;
                }
                case LeaveNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcrossing.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_MOUSE_LEAVE], &event);
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
                        .value = XkbKeycodeToKeysym(
                         ST_MONITOR_CALL(window->monitor, get_handle),
                         (unsigned char)xevent.xkey.keycode, 0, 0),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_KEY_PRESS], &press_event);

                    Xutf8LookupString(window->input_context, &xevent.xkey,
                     input_event.value, 4, 0, &status);
                    if (status == XBufferOverflow)
                        module->logger.warning(module->logger.ctx,
                         "window_xlib: Buffer owerflow on lookup inputted "
                         "UTF-8 character");
                    else if(status == XLookupChars)
                        module->events.push(module->events.ctx,
                         module->evtypes[EV_KEY_INPUT], &input_event);

                    break;
                }
                case KeyRelease: {
                    st_evwinu64_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xkey.window),
                        .value = XkbKeycodeToKeysym(
                         ST_MONITOR_CALL(window->monitor, get_handle),
                         (unsigned char)xevent.xkey.keycode, 0, 0),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_KEY_RELEASE], &event);

                    break;
                }
                case FocusIn: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xfocus.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_FOCUS_IN], &event);
                    break;
                }
                case FocusOut: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xfocus.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_FOCUS_OUT], &event);
                    break;
                }
                case ResizeRequest: {
                    st_evwinuvec2_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xresizerequest.window),
                        .hvalue = (unsigned)xevent.xresizerequest.width,
                        .vvalue = (unsigned)xevent.xresizerequest.height,
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_RESIZE], &event);
                    break;
                }
                case CirculateNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcirculate.window),
                    };
                    if (xevent.xcirculate.place == PlaceOnTop)
                        module->events.push(module->events.ctx,
                         module->evtypes[EV_PLACE_ON_TOP], &event);
                    else
                        module->events.push(module->events.ctx,
                         module->evtypes[EV_PLACE_ON_BOTTOM], &event);
                    break;
                }
                case CreateNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xcreatewindow.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_CREATE], &event);
                    break;
                }
                case DestroyNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xdestroywindow.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_DESTROY], &event);
                    break;
                }
                case MapNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xmap.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_SHOW], &event);
                    break;
                }
                case UnmapNotify: {
                    st_evwinnoargs_t event = {
                        .window = get_window_by_xwindow(window_ctx,
                         xevent.xunmap.window),
                    };
                    module->events.push(module->events.ctx,
                     module->evtypes[EV_HIDE], &event);
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

static st_monitor_t *st_window_get_monitor(st_window_t *window) {
    return window->monitor;
}

static void *st_window_get_handle(st_window_t *window) {
    return (void *)(uintptr_t)window->handle;
}

static unsigned st_window_get_width(const st_window_t *window) {
    return window->width;
}

static unsigned st_window_get_height(const st_window_t *window) {
    return window->height;
}
