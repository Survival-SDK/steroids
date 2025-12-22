#pragma once

#include <X11/Xlib.h>

#include "steroids/modctx.h"
#include "steroids/modsmgr.h"
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
    st_modctx_t;
    st_modsmgr_t    *modsmgr;
    st_eventsctx_t  *events_ctx;
    st_loggerctx_t  *logger_ctx;
    st_monitorctx_t *monitor_ctx;
    st_dlist_t      *windows;
    st_evtypeid_t    evtypes[EV_MAX];
} st_windowctx_t;

typedef struct {
    st_object_t;
    Window        handle;
    st_monitor_t *monitor;
    Display      *display;
    Atom          wm_delete_msg;
    XIM           input_method;
    XIC           input_context;
    bool          xed;
    unsigned      width;
    unsigned      height;
} st_window_t;

#define ST_WINDOWCTX_T_DEFINED
#define ST_WINDOW_T_DEFINED
