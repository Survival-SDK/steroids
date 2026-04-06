#include "simple.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define EVQ_POOL_SIZE 1024

static st_mousectx_t *st_mouse_init(const st_param_t params[]);
static void st_mouse_quit(st_mousectx_t *mouse_ctx);

static void st_mouse_process(st_mousectx_t *mouse_ctx);
static bool st_mouse_press(const st_mousectx_t *mouse_ctx, st_mbutton_t button);
static bool st_mouse_release(const st_mousectx_t *mouse_ctx,
 st_mbutton_t button);
static bool st_mouse_pressed(const st_mousectx_t *mouse_ctx,
 st_mbutton_t button);
static int st_mouse_get_wheel_relative(const st_mousectx_t *mouse_ctx);
static bool st_mouse_moved(const st_mousectx_t *mouse_ctx);
static bool st_mouse_entered(const st_mousectx_t *mouse_ctx);
static bool st_mouse_leaved(const st_mousectx_t *mouse_ctx);
static unsigned st_mouse_get_x(const st_mousectx_t *mouse_ctx);
static unsigned st_mouse_get_y(const st_mousectx_t *mouse_ctx);
static const st_window_t *st_mouse_get_window(const st_mousectx_t *mouse_ctx);

static st_mousectx_funcs_t mousectx_funcs = {
    ST_MODCTX_FUNCS,
    .process            = st_mouse_process,
    .press              = st_mouse_press,
    .release            = st_mouse_release,
    .pressed            = st_mouse_pressed,
    .get_wheel_relative = st_mouse_get_wheel_relative,
    .moved              = st_mouse_moved,
    .entered            = st_mouse_entered,
    .leaved             = st_mouse_leaved,
    .get_x              = st_mouse_get_x,
    .get_y              = st_mouse_get_y,
    .get_window         = st_mouse_get_window,
};

static const st_modprerq_t mod_prereqs[] = {
    { "events", NULL, },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_mouse_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("mouse", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_mouse_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_mouse_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "mouse";
static const char *st_module_name = "simple";

static st_mousectx_t *st_mouse_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx;
    st_eventsctx_t *events_ctx;
    st_mousectx_t  *mouse_ctx;

    if (!modsmgr)
        return NULL;

    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    if (!logger_ctx) {
        fprintf(stderr,
         "%s_%s: Unable to get logger context\n", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    events_ctx = (st_eventsctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "events", NULL);
    if (!events_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get events context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    mouse_ctx = (st_mousectx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_mousectx_t), NULL, &mousectx_funcs,
     (st_object_dtor_t)st_mouse_quit);
    if (!mouse_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create mouse context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    mouse_ctx->modsmgr = modsmgr;
    mouse_ctx->logger_ctx = logger_ctx;
    mouse_ctx->events_ctx = events_ctx;

    mouse_ctx->evtypes[EV_MOUSE_PRESS] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "mouse_press");
    mouse_ctx->evtypes[EV_MOUSE_RELEASE] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "mouse_release");
    mouse_ctx->evtypes[EV_MOUSE_WHEEL] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "mouse_wheel");
    mouse_ctx->evtypes[EV_MOUSE_MOVE] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "mouse_move");
    mouse_ctx->evtypes[EV_MOUSE_ENTER] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "mouse_enter");
    mouse_ctx->evtypes[EV_MOUSE_LEAVE] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "mouse_leave");

    mouse_ctx->evq = ST_EVENTSCTX_CALL(events_ctx, create_queue, EVQ_POOL_SIZE);
    ST_EVQ_CALL(mouse_ctx->evq, subscribe, mouse_ctx->evtypes[EV_MOUSE_PRESS]);
    ST_EVQ_CALL(mouse_ctx->evq, subscribe, 
     mouse_ctx->evtypes[EV_MOUSE_RELEASE]);
    ST_EVQ_CALL(mouse_ctx->evq, subscribe, mouse_ctx->evtypes[EV_MOUSE_WHEEL]);
    ST_EVQ_CALL(mouse_ctx->evq, subscribe, mouse_ctx->evtypes[EV_MOUSE_MOVE]);
    ST_EVQ_CALL(mouse_ctx->evq, subscribe, mouse_ctx->evtypes[EV_MOUSE_ENTER]);
    ST_EVQ_CALL(mouse_ctx->evq, subscribe, mouse_ctx->evtypes[EV_MOUSE_LEAVE]);

    mouse_ctx->x = 0;
    mouse_ctx->y = 0;
    memset(mouse_ctx->prev_mbstate, 0, sizeof(bool) * ST_MB_MAX);
    memset(mouse_ctx->curr_mbstate, 0, sizeof(bool) * ST_MB_MAX);
    mouse_ctx->wheel = 0;
    mouse_ctx->move = false;
    mouse_ctx->enter = false;
    mouse_ctx->leave = false;
    mouse_ctx->current_window = NULL;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Mouse initialized", st_module_subsystem, st_module_name);

    return mouse_ctx;
}

static void st_mouse_quit(st_mousectx_t *mouse_ctx) {
    ST_EVQ_CALL(mouse_ctx->evq, destroy);

    ST_LOGGERCTX_CALL(mouse_ctx->logger_ctx, info,
     "%s_%s: Mouse destroyed", st_module_subsystem, st_module_name);
    free(mouse_ctx);
}

static void st_mouse_process_press(st_mousectx_t *mouse_ctx) {
    st_evwinunsigned_t event;

    ST_EVQ_CALL(mouse_ctx->evq, pop, &event);

    mouse_ctx->curr_mbstate[event.value] = true;
}

static void st_mouse_process_release(st_mousectx_t *mouse_ctx) {
    st_evwinunsigned_t event;

    ST_EVQ_CALL(mouse_ctx->evq, pop, &event);

    mouse_ctx->curr_mbstate[event.value] = false;
}

static void st_mouse_process_wheel(st_mousectx_t *mouse_ctx) {
    st_evwininteger_t event;

    ST_EVQ_CALL(mouse_ctx->evq, pop, &event);

    mouse_ctx->wheel = event.value;
}

static void st_mouse_process_move(st_mousectx_t *mouse_ctx) {
    st_evwinuvec2_t event;

    ST_EVQ_CALL(mouse_ctx->evq, pop, &event);

    mouse_ctx->x = event.hvalue;
    mouse_ctx->y = event.vvalue;
    mouse_ctx->move = true;
    mouse_ctx->current_window = event.window;
}

static void st_mouse_process_enter(st_mousectx_t *mouse_ctx) {
    st_evwinnoargs_t event;

    ST_EVQ_CALL(mouse_ctx->evq, pop, &event);

    mouse_ctx->enter = true;
    mouse_ctx->current_window = event.window;
}

static void st_mouse_process_leave(st_mousectx_t *mouse_ctx) {
    st_evwinnoargs_t event;

    ST_EVQ_CALL(mouse_ctx->evq, pop, &event);

    mouse_ctx->leave = true;
    mouse_ctx->current_window = NULL;
}

static void (*procfuncs[])(st_mousectx_t *mouse_ctx) = {
    [EV_MOUSE_PRESS]   = st_mouse_process_press,
    [EV_MOUSE_RELEASE] = st_mouse_process_release,
    [EV_MOUSE_WHEEL]   = st_mouse_process_wheel,
    [EV_MOUSE_MOVE]    = st_mouse_process_move,
    [EV_MOUSE_ENTER]   = st_mouse_process_enter,
    [EV_MOUSE_LEAVE]   = st_mouse_process_leave,
};

static void st_mouse_process(st_mousectx_t *mouse_ctx) {
    for (unsigned i = 0; i < ST_MB_MAX; i++)
        mouse_ctx->prev_mbstate[i] = mouse_ctx->curr_mbstate[i];

    mouse_ctx->wheel = 0;
    mouse_ctx->move = false;
    mouse_ctx->enter = false;
    mouse_ctx->leave = false;

    while (!ST_EVQ_CALL(mouse_ctx->evq, is_empty)) {
        st_evtypeid_t evtype = ST_EVQ_CALL(mouse_ctx->evq, peek_type);

        for (evtype_index_t evt = 0; evt < EV_MAX; evt++) {
            if (evtype == mouse_ctx->evtypes[evt]) {
                procfuncs[evt](mouse_ctx);

                break;
            }
        }
    }
}

static bool st_mouse_press(const st_mousectx_t *mouse_ctx, st_mbutton_t button) {
    return !mouse_ctx->prev_mbstate[button] && mouse_ctx->curr_mbstate[button];
}

static bool st_mouse_release(const st_mousectx_t *mouse_ctx,
 st_mbutton_t button) {
    return mouse_ctx->prev_mbstate[button] && !mouse_ctx->curr_mbstate[button];
}

static bool st_mouse_pressed(const st_mousectx_t *mouse_ctx,
 st_mbutton_t button) {
    return mouse_ctx->curr_mbstate[button];
}

static int st_mouse_get_wheel_relative(const st_mousectx_t *mouse_ctx) {
    return mouse_ctx->wheel;
}

static bool st_mouse_moved(const st_mousectx_t *mouse_ctx) {
    return mouse_ctx->move;
}

static bool st_mouse_entered(const st_mousectx_t *mouse_ctx) {
    return mouse_ctx->enter;
}

static bool st_mouse_leaved(const st_mousectx_t *mouse_ctx) {
    return mouse_ctx->leave;
}

static unsigned st_mouse_get_x(const st_mousectx_t *mouse_ctx) {
    return mouse_ctx->x;
}

static unsigned st_mouse_get_y(const st_mousectx_t *mouse_ctx) {
    return mouse_ctx->y;
}

static const st_window_t *st_mouse_get_window(const st_mousectx_t *mouse_ctx) {
    return mouse_ctx->current_window;
}
