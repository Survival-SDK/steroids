#include "simple.h"

#include <errno.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/dpsrvconn.h"

#define EVQ_POOL_SIZE  1024
#define ERRMSGBUF_SIZE 128

static st_keyboardctx_t *st_keyboard_init(const st_param_t params[]);
static void st_keyboard_quit(st_keyboardctx_t *keyboard_ctx);

static void st_keyboard_process(st_keyboardctx_t *keyboard_ctx);
static bool st_keyboard_press(const st_keyboardctx_t *keyboard_ctx, 
 st_key_t key);
static bool st_keyboard_release(const st_keyboardctx_t *keyboard_ctx, 
 st_key_t key);
static bool st_keyboard_pressed(const st_keyboardctx_t *keyboard_ctx,
 st_key_t key);
static const char *st_keyboard_get_input(const st_keyboardctx_t *keyboard_ctx);

static st_keyboardctx_funcs_t keyboardctx_funcs = {
    st_modctx_funcs,
    .process   = st_keyboard_process,
    .press     = st_keyboard_press,
    .release   = st_keyboard_release,
    .pressed   = st_keyboard_pressed,
    .get_input = st_keyboard_get_input,
};

static const st_modprerq_t mod_prereqs[] = {
    { "events", NULL, },
    { "htable", NULL, },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_keyboard_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("keyboard", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_keyboard_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_keyboard_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "keyboard";
static const char *st_module_name = "simple";

static bool st_keyeqfunc(const void *left, const void *right) {
    return (uint32_t)(uintptr_t)left == (uint32_t)(uintptr_t)right;
}

static uint32_t st_keyboard_hash_key(const void *key) {
    return (uint32_t)(uintptr_t)key;
}

static st_keyboardctx_t *st_keyboard_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx;
    st_eventsctx_t *events_ctx;
    st_htablectx_t *htable_ctx;
    st_keyboardctx_t *keyboard_ctx;

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

    htable_ctx = (st_htablectx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "htable", NULL);
    if (!htable_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get htable context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    keyboard_ctx = (st_keyboardctx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_keyboardctx_t), NULL, &keyboardctx_funcs,
     (st_object_dtor_t)st_keyboard_quit);
    if (!keyboard_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create keyboard context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    keyboard_ctx->modsmgr = modsmgr;
    keyboard_ctx->events_ctx = events_ctx;
    keyboard_ctx->logger_ctx = logger_ctx;
    keyboard_ctx->htable_ctx = htable_ctx;

    keyboard_ctx->evtypes[EV_KEY_PRESS] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "key_press");
    keyboard_ctx->evtypes[EV_KEY_RELEASE] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "key_release");
    keyboard_ctx->evtypes[EV_KEY_INPUT] = ST_EVENTSCTX_CALL(events_ctx,
     get_type_id, "key_input");

    keyboard_ctx->prev_state = ST_HTABLECTX_CALL(htable_ctx, create,
     st_keyboard_hash_key, st_keyeqfunc, NULL, NULL);
    if (!keyboard_ctx->prev_state) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create prev_state hash table", st_module_subsystem,
         st_module_name);

        goto prev_state_fail;
    }

    keyboard_ctx->cur_state = ST_HTABLECTX_CALL(htable_ctx, create,
     st_keyboard_hash_key, st_keyeqfunc, NULL, NULL);
    if (!keyboard_ctx->cur_state) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create cur_state hash table", st_module_subsystem,
         st_module_name);

        goto cur_state_fail;
    }

    keyboard_ctx->evq = ST_EVENTSCTX_CALL(events_ctx, create_queue, 
     EVQ_POOL_SIZE);
    if (!keyboard_ctx->evq) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create event queue", st_module_subsystem,
         st_module_name);

        goto create_queue_fail;
    }

    ST_EVQ_CALL(keyboard_ctx->evq, subscribe, 
     keyboard_ctx->evtypes[EV_KEY_PRESS]);
    ST_EVQ_CALL(keyboard_ctx->evq, subscribe, 
     keyboard_ctx->evtypes[EV_KEY_RELEASE]);
    ST_EVQ_CALL(keyboard_ctx->evq, subscribe, 
     keyboard_ctx->evtypes[EV_KEY_INPUT]);

    memset(keyboard_ctx->input, 0, INPUT_SIZE);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Keyboard context initialized", st_module_subsystem, 
     st_module_name);

    return keyboard_ctx;

create_queue_fail:
    ST_HTABLE_CALL(keyboard_ctx->cur_state, destroy);
cur_state_fail:
    ST_HTABLE_CALL(keyboard_ctx->prev_state, destroy);
prev_state_fail:
    free(keyboard_ctx);
    
    return NULL;
}

static void st_keyboard_quit(st_keyboardctx_t *keyboard_ctx) {
    ST_LOGGERCTX_CALL(keyboard_ctx->logger_ctx, info,
     "%s_%s: Keyboard context destroyed", st_module_subsystem, st_module_name);

    ST_EVQ_CALL(keyboard_ctx->evq, destroy);
    ST_HTABLE_CALL(keyboard_ctx->cur_state, destroy);
    ST_HTABLE_CALL(keyboard_ctx->prev_state, destroy);
    free(keyboard_ctx);
}

static void st_keyboard_process_press(st_keyboardctx_t *keyboard_ctx) {
    st_evwinu64_t event;

    ST_EVQ_CALL(keyboard_ctx->evq, pop, &event);
    ST_HTABLE_CALL(keyboard_ctx->cur_state, insert, NULL, 
     (const void *)event.value, (void *)1ull);
}

static void st_keyboard_process_release(st_keyboardctx_t *keyboard_ctx) {
    st_evwinu64_t event;

    ST_EVQ_CALL(keyboard_ctx->evq, pop, &event);
    ST_HTABLE_CALL(keyboard_ctx->cur_state, insert, NULL, 
     (const void *)event.value, (void *)0ull);
}

static void st_keyboard_process_input(st_keyboardctx_t *keyboard_ctx) {
    st_evwinsymbol_t event;

    ST_EVQ_CALL(keyboard_ctx->evq, pop, &event);
    memcpy(keyboard_ctx->input, event.value, INPUT_SIZE);
}

static void (*procfuncs[])(st_keyboardctx_t *keyboard_ctx) = {
    [EV_KEY_PRESS] = st_keyboard_process_press,
    [EV_KEY_RELEASE] = st_keyboard_process_release,
    [EV_KEY_INPUT] = st_keyboard_process_input,
};

static void st_keyboard_process(st_keyboardctx_t *keyboard_ctx) {
    st_htiter_t it;

    if (ST_HTABLE_CALL(keyboard_ctx->cur_state, get_first, &it)) {
        do {
            const void *key   = ST_HTITER_CALL(&it, get_key);
            void       *value = ST_HTITER_CALL(&it, get_value);

            ST_HTABLE_CALL(keyboard_ctx->prev_state, insert, NULL, key, value);
        } while (ST_HTITER_CALL(&it, get_next, &it));
    }

    memset(keyboard_ctx->input, 0, INPUT_SIZE);

    while (!ST_EVQ_CALL(keyboard_ctx->evq, is_empty)) {
        st_evtypeid_t evtype = ST_EVQ_CALL(keyboard_ctx->evq, peek_type);

        for (evtype_index_t evt = 0; evt < EV_MAX; evt++) {
            if (evtype == keyboard_ctx->evtypes[evt]) {
                procfuncs[evt](keyboard_ctx);

                break;
            }
        }
    }
}

static bool st_keyboard_press(const st_keyboardctx_t *keyboard_ctx, 
 st_key_t key) {
    return ST_HTABLE_CALL(keyboard_ctx->cur_state, get, (const void *)key)
     && !ST_HTABLE_CALL(keyboard_ctx->prev_state, get, (const void *)key);
}

static bool st_keyboard_release(const st_keyboardctx_t *keyboard_ctx, 
 st_key_t key) {
    return !ST_HTABLE_CALL(keyboard_ctx->cur_state, get, (const void *)key)
     && ST_HTABLE_CALL(keyboard_ctx->prev_state, get, (const void *)key);
}

static bool st_keyboard_pressed(const st_keyboardctx_t *keyboard_ctx, 
 st_key_t key) {
    return ST_HTABLE_CALL(keyboard_ctx->cur_state, get, (const void *)key)
     && ST_HTABLE_CALL(keyboard_ctx->prev_state, get, (const void *)key);
}

static const char *st_keyboard_get_input(const st_keyboardctx_t *keyboard_ctx) {
    return *keyboard_ctx->input ? keyboard_ctx->input : NULL;
}
