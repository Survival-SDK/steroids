#pragma once

#include "steroids/consts/mouse.h"
#include "steroids/modctx.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/events.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/dpsrvconn.h"

typedef enum {
    EV_MOUSE_PRESS = 0,
    EV_MOUSE_RELEASE,
    EV_MOUSE_WHEEL,
    EV_MOUSE_MOVE,
    EV_MOUSE_ENTER,
    EV_MOUSE_LEAVE,

    EV_MAX,
} evtype_index_t;

typedef struct {
    st_modctx_t;
    st_modsmgr_t      *modsmgr;
    st_eventsctx_t    *events_ctx;
    st_loggerctx_t    *logger_ctx;
    st_evtypeid_t      evtypes[EV_MAX];
    st_evq_t          *evq;
    unsigned           x;
    unsigned           y;
    bool               prev_mbstate[ST_MB_MAX];
    bool               curr_mbstate[ST_MB_MAX];
    int                wheel;
    bool               move;
    bool               enter;
    bool               leave;
    const st_window_t *current_window;
} st_mousectx_t;

#define ST_MOUSECTX_T_DEFINED
