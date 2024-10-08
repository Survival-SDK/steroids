#include "simple.h"

#include <stdio.h>

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_evq_funcs_t evq_funcs = {
    .destroy_queue   = st_events_destroy_queue,
    .subscribe       = st_events_subscribe,
    .unsubscribe     = st_events_unsubscribe,
    .unsubscribe_all = st_events_unsubscribe_all,
    .suspend         = st_events_suspend,
    .resume          = st_events_resume,
    .is_empty        = st_events_is_empty,
    .peek_type       = st_events_peek_type,
    .pop             = st_events_pop,
    .drop            = st_events_drop,
    .clear           = st_events_clear,
};

ST_MODULE_DEF_GET_FUNC(events_simple)
ST_MODULE_DEF_INIT_FUNC(events_simple)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_events_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_events_import_functions(st_modctx_t *events_ctx,
 st_modctx_t *logger_ctx) {
    st_events_simple_t *module = events_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "events_simple: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("events_simple", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("events_simple", logger, info);

    ST_LOAD_FUNCTION("events_simple", rbuf, NULL, create);

    return true;
}

static st_modctx_t *st_events_init(st_modctx_t *logger_ctx) {
    st_modctx_t        *events_ctx;
    st_events_simple_t *module;

    events_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_events_simple_data, sizeof(st_events_simple_t));

    if (events_ctx == NULL)
        return NULL;

    module = events_ctx->data;

    module->rbuf.init = global_modsmgr_funcs.get_function(global_modsmgr,
     "rbuf", NULL, "init");
    if (!module->rbuf.init) {
        fprintf(stderr,
         "events_simple: Unable to load function \"init\" from module "
         "\"rbuf\"\n");

        goto get_func_fail;
    }

    module->rbuf.quit = global_modsmgr_funcs.get_function(global_modsmgr,
     "rbuf", NULL, "quit");
    if (!module->rbuf.quit) {
        fprintf(stderr,
         "events_simple: Unable to load function \"quit\" from module "
         "\"rbuf\"\n");

        goto get_func_fail;
    }

    events_ctx->funcs = &st_events_simple_funcs;

    module->logger.ctx = logger_ctx;
    module->rbuf.ctx = module->rbuf.init(logger_ctx);
    if (!module->rbuf.ctx)
        goto rbuf_init_fail;

    if (!st_events_import_functions(events_ctx, logger_ctx))
        goto import_funcs_fail;

    module->types_count = 0;

    module->logger.info(module->logger.ctx,
     "events_simple: Event subsystem initialized.");

    return events_ctx;

import_funcs_fail:
    module->rbuf.quit(module->rbuf.ctx);
rbuf_init_fail:
get_func_fail:
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, events_ctx);

    return NULL;
}

static void st_events_quit(st_modctx_t *events_ctx) {
    st_events_simple_t *module = events_ctx->data;

    // TODO(edomin):
    // Unsubscribe all queues
    // Delete queues

    module->rbuf.quit(module->rbuf.ctx);

    module->logger.info(module->logger.ctx,
     "events_simple: Event subsystem destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, events_ctx);
}

static st_evtypeid_t st_events_register_type(st_modctx_t *events_ctx,
 const char *type_name, size_t size) {
    st_events_simple_t *module = events_ctx->data;
    st_evtype_t        *evtype = &module->types[module->types_count];
    int                 ret = snprintf(evtype->name, EVENT_TYPE_NAME_SIZE, "%s",
     type_name);

    if (ret < 0 || ret == EVENT_TYPE_NAME_SIZE) {
        module->logger.error(module->logger.ctx,
         "events_simple: Unable to copy event type name while registering "
         "event type \"%s\"", type_name);

        return ST_EVTYPE_ID_NONE;
    }

    memset(evtype->subscribers, 0, sizeof(st_evq_t *) * SUBSCRIBERS_MAX);
    evtype->data_size = size;
    evtype->subscribers_count = 0;

    return (st_evtypeid_t)(module->types_count++);
}

static st_evtypeid_t st_events_get_type_id(st_modctx_t *events_ctx,
 const char *type_name) {
    st_events_simple_t *module = events_ctx->data;

    for (size_t i = 0; i < module->types_count; i++) {
        if (strcmp(module->types[i].name, type_name) == 0)
            return (st_evtypeid_t)i;
    }

    return ST_EVTYPE_ID_NONE;
}

static st_evq_t *st_events_create_queue(st_modctx_t *events_ctx,
 size_t pool_size) {
    st_events_simple_t *module = events_ctx->data;
    st_evq_t           *queue;
    st_rbuf_t          *handle = module->rbuf.create(module->rbuf.ctx,
     pool_size);

    if (!handle)
        return NULL;

    queue = malloc(sizeof(st_evq_t));
    if (!queue) {
        ST_RBUF_CALL(handle, destroy);

        return NULL;
    }

    st_object_make(queue, events_ctx, &evq_funcs);
    queue->handle = handle;
    queue->active = true;

    return queue;
}

static void st_events_destroy_queue(st_evq_t *queue) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;

    st_events_unsubscribe_all(queue);

    ST_RBUF_CALL(queue->handle, destroy);
    free(queue);
}

static bool st_events_subscribe(st_evq_t *queue, st_evtypeid_t type_id) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;
    st_evtype_t        *evtype;

    if (type_id >= (st_evtypeid_t)module->types_count)
        return false;

    evtype = &module->types[type_id];

    for (size_t i = 0; i < evtype->subscribers_count; i++) {
        if (evtype->subscribers[i] == queue)
            return true;
    }

    if (evtype->subscribers_count >= SUBSCRIBERS_MAX)
        return false;

    evtype->subscribers[evtype->subscribers_count++] = queue;

    return true;
}

static void st_events_unsubscribe(st_evq_t *queue, st_evtypeid_t type_id) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;
    st_evtype_t        *evtype;

    if (type_id >= (st_evtypeid_t)module->types_count)
        return;

    evtype = &module->types[type_id];

    for (size_t i = 0; i < evtype->subscribers_count; i++) {
        if (evtype->subscribers[i] == queue) {
            size_t block_size = sizeof(st_evq_t *) *
             (evtype->subscribers_count - i - 1);

            if (i <= evtype->subscribers_count - 1) {
                memmove(&evtype->subscribers[i], &evtype->subscribers[i + 1],
                 block_size);
                evtype->subscribers_count--;
            }

            return;
        }
    }
}

static void st_events_unsubscribe_all(st_evq_t *queue) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;

    for (int i = 0; i < (st_evtypeid_t)module->types_count; i++)
        st_events_unsubscribe(queue, i);
}

static void st_events_suspend(st_evq_t *queue, bool clear) {
    if (clear)
        st_events_clear(queue);

    queue->active = false;
}

static void st_events_resume(st_evq_t *queue) {
    queue->active = true;
}

static void st_events_push(st_modctx_t *events_ctx, st_evtypeid_t type_id,
 const void *data) {
    st_events_simple_t *module = events_ctx->data;
    st_evtype_t        *evtype;

    if (type_id >= (st_evtypeid_t)module->types_count)
        return;

    evtype = &module->types[type_id];

    for (size_t i = 0; i < evtype->subscribers_count; i++) {
        if (!evtype->subscribers[i]->active)
            continue;

        if (ST_RBUF_CALL(evtype->subscribers[i]->handle, get_free_space) >=
         evtype->data_size + sizeof(st_evtypeid_t)) {
            ST_RBUF_CALL(evtype->subscribers[i]->handle, push, &type_id,
             sizeof(st_evtypeid_t));
            ST_RBUF_CALL(evtype->subscribers[i]->handle, push, data,
             evtype->data_size);
        }
    }
}

static bool st_events_is_empty(const st_evq_t *queue) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;

    return ST_RBUF_CALL(queue->handle, is_empty);
}

static st_evtypeid_t st_events_peek_type(const st_evq_t *queue) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;
    st_evtypeid_t       type_id;
    bool                success = ST_RBUF_CALL(queue->handle, peek, &type_id,
     sizeof(st_evtypeid_t));

    return success ? type_id : ST_EVTYPE_ID_NONE;
}

static bool st_events_pop(st_evq_t *queue, void *data) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;
    st_evtypeid_t       type_id;

    if (!ST_RBUF_CALL(queue->handle, pop, &type_id, sizeof(st_evtypeid_t)))
        return false;

    return ST_RBUF_CALL(queue->handle, pop, data,
     module->types[type_id].data_size);
}

static bool st_events_drop(st_evq_t *queue) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;
    st_evtypeid_t       type_id;

    if (!ST_RBUF_CALL(queue->handle, pop, &type_id, sizeof(st_evtypeid_t)))
        return false;

    return ST_RBUF_CALL(queue->handle, drop, module->types[type_id].data_size);
}

static bool st_events_clear(st_evq_t *queue) {
    st_events_simple_t *module = ((st_modctx_t *)st_object_get_owner(queue))->data;

    return ST_RBUF_CALL(queue->handle, clear);
}
