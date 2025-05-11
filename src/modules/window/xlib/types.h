#pragma once

#include <X11/Xlib.h>

#include "steroids/modules/events.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/monitor.h"
#include "steroids/object.h"

#include "dlist.h"

typedef enum {
    EV_MOUSE_PRESS = 0,
    EV_MOUSE_RELEASE,
    EV_MOUSE_WHEEL,
    EV_MOUSE_MOVE,
    EV_MOUSE_ENTER,
    EV_MOUSE_LEAVE,

    EV_KEY_PRESS,
    EV_KEY_RELEASE,
    EV_KEY_INPUT,

    EV_FOCUS_IN,
    EV_FOCUS_OUT,
    EV_RESIZE,
    EV_PLACE_ON_TOP,
    EV_PLACE_ON_BOTTOM,
    EV_CREATE,
    EV_DESTROY,
    // EV_MOVE,
    EV_SHOW,
    EV_HIDE,

    EV_MAX,
} evtype_index_t;

typedef struct {
    st_modctx_t              *ctx;
    st_events_register_type_t register_type;
    st_events_push_t          push;
} st_window_xlib_events_t;

typedef struct {
    st_modctx_t        *ctx;
    st_logger_debug_t   debug;
    st_logger_info_t    info;
    st_logger_warning_t warning;
    st_logger_error_t   error;
} st_window_xlib_logger_t;

typedef struct {
    st_modctx_t *ctx;
} st_window_xlib_monitor_t;

typedef struct {
    st_window_xlib_events_t  events;
    st_window_xlib_logger_t  logger;
    st_window_xlib_monitor_t monitor;
    st_dlist_t              *windows;
    st_evtypeid_t            evtypes[EV_MAX];
} st_window_xlib_t;

ST_CLASS (
    Window        handle;
    st_monitor_t *monitor;
    Atom          wm_delete_msg;
    XIM           input_method;
    XIC           input_context;
    bool          xed;
    unsigned      width;
    unsigned      height;
) st_window_t;

#define ST_WINDOW_T_DEFINED
#define ST_MONITOR_T_DEFINED
