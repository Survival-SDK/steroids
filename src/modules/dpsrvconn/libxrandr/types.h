#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>

#include "steroids/modctx.h"
#include "steroids/modules/events.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

#define MONITORS_MAX 8

typedef enum {
    EV_MONITOR_CONNECTED = 0,
    EV_MONITOR_DISCONNECTED,
    EV_MONITOR_REINDEX,
    EV_MONITOR_RESIZE,

    EV_MOUSE_PRESS,
    EV_MOUSE_RELEASE,
    EV_MOUSE_WHEEL,
    EV_MOUSE_MOVE,
    EV_MOUSE_ENTER,
    EV_MOUSE_LEAVE,

    EV_KEY_PRESS,
    EV_KEY_RELEASE,
    EV_KEY_INPUT,

    EV_WIN_FOCUS_IN,
    EV_WIN_FOCUS_OUT,
    EV_WIN_RESIZE,
    EV_WIN_PLACE_ON_TOP,
    EV_WIN_PLACE_ON_BOTTOM,
    EV_WIN_CREATE,
    EV_WIN_DESTROY,
    // EV_MOVE,
    EV_WIN_SHOW,
    EV_WIN_HIDE,
    EV_WIN_MONITOR_CHANGED,

    EV_MAX,
} evtype_index_t;

typedef struct {
    st_object_t;
    unsigned  index;
    RROutput  output_id;
    char     *name;
    bool      is_primary;
    int       x;
    int       y;
    unsigned  width;
    unsigned  height;
} st_monitor_t;

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_eventsctx_t *events_ctx;
    Display        *display;
    Window          root_window;
    bool            dri2_available;
    bool            dri3_available;
    XContext        xcontext;
    st_monitor_t    monitors[MONITORS_MAX];
    size_t          monitors_count;
    int             randr_event_base;
    st_evtypeid_t   evtypes[EV_MAX];
} st_dpsrvconnctx_t;

typedef struct {
    st_object_t;
    Window        handle;
    Atom          wm_delete_msg;
    XIM           input_method;
    XIC           input_context;
    bool          xed;
    unsigned      width;
    unsigned      height;
    st_monitor_t *monitor;
} st_window_t;

#define ST_DPSRVCONNCTX_T_DEFINED
#define ST_MONITOR_T_DEFINED
#define ST_WINDOW_T_DEFINED
