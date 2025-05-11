#include "simple.h"

#include <stdio.h>

#include "steroids/modules/window.h"

#define EVQ_POOL_SIZE 1024

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

ST_MODULE_DEF_GET_FUNC(keyboard_simple)
ST_MODULE_DEF_INIT_FUNC(keyboard_simple)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_keyboard_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_keyboard_import_functions(st_modctx_t *keyboard_ctx,
 st_modctx_t *events_ctx, st_modctx_t *logger_ctx) {
    st_keyboard_simple_t *module = keyboard_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "keyboard_simple: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION("keyboard_simple", htable, NULL, create);
    ST_LOAD_FUNCTION("keyboard_simple", htable, NULL, init);
    ST_LOAD_FUNCTION("keyboard_simple", htable, NULL, quit);

    ST_LOAD_FUNCTION_FROM_CTX("keyboard_simple", events, get_type_id);
    ST_LOAD_FUNCTION_FROM_CTX("keyboard_simple", events, create_queue);

    ST_LOAD_FUNCTION_FROM_CTX("keyboard_simple", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("keyboard_simple", logger, info);

    return true;
}

static bool st_keyeqfunc(const void *left, const void *right) {
    return (uint32_t)(uintptr_t)left == (uint32_t)(uintptr_t)right;
}

static uint32_t st_keyboard_hash_key(const void *key) {
    return (uint32_t)(uintptr_t)key;
}

static st_modctx_t *st_keyboard_init(st_modctx_t *events_ctx,
 st_modctx_t *logger_ctx) {
    st_modctx_t          *keyboard_ctx;
    st_keyboard_simple_t *module;

    keyboard_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_keyboard_simple_data, sizeof(st_keyboard_simple_t));

    if (!keyboard_ctx)
        return NULL;

    keyboard_ctx->funcs = &st_keyboard_simple_funcs;

    module = keyboard_ctx->data;
    module->events.ctx = events_ctx;
    module->logger.ctx = logger_ctx;

    if (!st_keyboard_import_functions(keyboard_ctx, events_ctx, logger_ctx))
        goto import_fail;

    module->htable.ctx = module->htable.init(module->logger.ctx);
    if (!module->htable.ctx)
        goto ht_ctx_init_fail;

    module->evtypes[EV_KEY_PRESS] = module->events.get_type_id(events_ctx,
     "window_key_press");
    module->evtypes[EV_KEY_RELEASE] = module->events.get_type_id(events_ctx,
     "window_key_release");
    module->evtypes[EV_KEY_INPUT] = module->events.get_type_id(events_ctx,
     "window_key_input");

    module->prev_state = module->htable.create(module->htable.ctx,
     st_keyboard_hash_key, st_keyeqfunc, NULL, NULL);
    if (!module->prev_state)
        goto prev_state_fail;

    module->cur_state = module->htable.create(module->htable.ctx,
     st_keyboard_hash_key, st_keyeqfunc, NULL, NULL);
    if (!module->cur_state)
        goto cur_state_fail;

    module->evq = module->events.create_queue(events_ctx, EVQ_POOL_SIZE);
    if (!module->evq)
        goto create_queue_fail;

    ST_EVQ_CALL(module->evq, subscribe, module->evtypes[EV_KEY_PRESS]);
    ST_EVQ_CALL(module->evq, subscribe, module->evtypes[EV_KEY_RELEASE]);
    ST_EVQ_CALL(module->evq, subscribe, module->evtypes[EV_KEY_INPUT]);
    memset(module->input, 0, INPUT_SIZE);

    module->logger.info(module->logger.ctx,
     "keyboard_simple: Keyboard initialized");

    return keyboard_ctx;

create_queue_fail:
    ST_HTABLE_CALL(module->cur_state, destroy);
cur_state_fail:
    ST_HTABLE_CALL(module->prev_state, destroy);
prev_state_fail:
    module->htable.quit(module->htable.ctx);
ht_ctx_init_fail:
import_fail:
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, keyboard_ctx);

    return NULL;
}

static void st_keyboard_quit(st_modctx_t *keyboard_ctx) {
    st_keyboard_simple_t *module = keyboard_ctx->data;

    ST_EVQ_CALL(module->evq, destroy_queue);
    ST_HTABLE_CALL(module->cur_state, destroy);
    ST_HTABLE_CALL(module->prev_state, destroy);
    module->htable.quit(module->htable.ctx);

    module->logger.info(module->logger.ctx,
     "keyboard_simple: Keyboard destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, keyboard_ctx);
}

static void st_keyboard_process_press(st_modctx_t *keyboard_ctx) {
    st_keyboard_simple_t *module = keyboard_ctx->data;
    st_evwinu64_t         event;

    ST_EVQ_CALL(module->evq, pop, &event);
    ST_HTABLE_CALL(module->cur_state, insert, NULL, (const void *)event.value,
     (void *)1ull);
}

static void st_keyboard_process_release(st_modctx_t *keyboard_ctx) {
    st_keyboard_simple_t *module = keyboard_ctx->data;
    st_evwinu64_t         event;

    ST_EVQ_CALL(module->evq, pop, &event);

    ST_HTABLE_CALL(module->cur_state, insert, NULL, (const void *)event.value,
     (void *)0ull);
}

static void st_keyboard_process_input(st_modctx_t *keyboard_ctx) {
    st_keyboard_simple_t *module = keyboard_ctx->data;
    st_evwinsymbol_t      event;

    ST_EVQ_CALL(module->evq, pop, &event);

    memcpy(module->input, event.value, INPUT_SIZE);
}

static void (*procfuncs[])(st_modctx_t *keyboard_ctx) = {
    [EV_KEY_PRESS] = st_keyboard_process_press,
    [EV_KEY_RELEASE] = st_keyboard_process_release,
    [EV_KEY_INPUT] = st_keyboard_process_input,
};

static void st_keyboard_process(st_modctx_t *keyboard_ctx) {
    st_keyboard_simple_t *module = keyboard_ctx->data;
    st_htiter_t           it;

    if (ST_HTABLE_CALL(module->cur_state, get_first, &it)) { ;
        do {
            const void *key   = ST_HTITER_CALL(&it, get_key);
            void       *value = ST_HTITER_CALL(&it, get_value);

            ST_HTABLE_CALL(module->prev_state, insert, NULL, key, value);
        } while (ST_HTITER_CALL(&it, get_next, &it));
    }

    memset(module->input, 0, INPUT_SIZE);

    while (!ST_EVQ_CALL(module->evq, is_empty)) {
        st_evtypeid_t evtype = ST_EVQ_CALL(module->evq, peek_type);

        for (evtype_index_t evt = 0; evt < EV_MAX; evt++) {
            if (evtype == module->evtypes[evt]) {
                procfuncs[evt](keyboard_ctx);

                break;
            }
        }
    }
}

static bool st_keyboard_press(const st_modctx_t *keyboard_ctx, st_key_t key) {
    st_keyboard_simple_t *module = keyboard_ctx->data;
    return ST_HTABLE_CALL(module->cur_state, get, (const void *)key)
     && !ST_HTABLE_CALL(module->prev_state, get, (const void *)key);
}

static bool st_keyboard_release(const st_modctx_t *keyboard_ctx, st_key_t key) {
    st_keyboard_simple_t *module = keyboard_ctx->data;

    return !ST_HTABLE_CALL(module->cur_state, get, (const void *)key)
     && ST_HTABLE_CALL(module->prev_state, get, (const void *)key);
}

static bool st_keyboard_pressed(const st_modctx_t *keyboard_ctx, st_key_t key) {
    st_keyboard_simple_t *module = keyboard_ctx->data;

    return ST_HTABLE_CALL(module->cur_state, get, (const void *)key)
     && ST_HTABLE_CALL(module->prev_state, get, (const void *)key);
}

static const char *st_keyboard_get_input(const st_modctx_t *keyboard_ctx) {
    st_keyboard_simple_t *module = keyboard_ctx->data;

    return *module->input ? module->input : NULL;
}
