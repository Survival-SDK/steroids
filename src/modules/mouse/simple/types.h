#pragma once

#include "steroids/modules/events.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/mouse.h"
#include "steroids/modules/window.h"

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
    st_modctx_t             *ctx;
    st_events_get_type_id_t  get_type_id;
    st_events_create_queue_t create_queue;
} st_mouse_simple_events_t;

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_mouse_simple_logger_t;

typedef struct {
    st_mouse_simple_events_t events;
    st_mouse_simple_logger_t logger;
    st_evtypeid_t            evtypes[EV_MAX];
    st_evq_t                *evq;
    unsigned                 x;
    unsigned                 y;
    bool                     prev_mbstate[ST_MB_MAX];
    bool                     curr_mbstate[ST_MB_MAX];
    int                      wheel;
    bool                     move;
    bool                     enter;
    bool                     leave;
    const st_window_t       *current_window;
} st_mouse_simple_t;
