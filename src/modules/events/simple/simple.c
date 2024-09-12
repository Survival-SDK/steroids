#include "simple.h"

#include <stdio.h>

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_eventsctx_funcs_t eventsctx_funcs = {
    .quit          = st_events_quit,
    .register_type = st_events_register_type,
    .get_type_id   = st_events_get_type_id,
    .create_queue  = st_events_create_queue,
    .push          = st_events_push,
};

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

static const char *st_module_subsystem = "events";
static const char *st_module_name = "simple";

static st_eventsctx_t *st_events_init(struct st_loggerctx_s *logger_ctx) {
    st_rbuf_init_t  rbuf_init;
    st_eventsctx_t *events_ctx = st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_eventsctx_t), NULL, &eventsctx_funcs);

    if (events_ctx == NULL)
        return NULL;

    rbuf_init = global_modsmgr_funcs.get_function(global_modsmgr,
     "rbuf", NULL, "init");
    if (!rbuf_init) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "events_simple: Unable to load function \"init\" from module "
         "\"rbuf\"");

        goto get_func_fail;
    }

    events_ctx->rbuf_ctx = rbuf_init(logger_ctx);
    if (!events_ctx->rbuf_ctx)
        goto rbuf_init_fail;

    events_ctx->logger_ctx = logger_ctx;
    events_ctx->types_count = 0;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "events_simple: Event subsystem initialized.");

    return events_ctx;

rbuf_init_fail:
get_func_fail:
    free(events_ctx);

    return NULL;
}

static void st_events_quit(st_eventsctx_t *events_ctx) {
    // TODO(edomin):
    // Unsubscribe all queues
    // Delete queues

    ST_RBUFCTX_CALL(events_ctx->rbuf_ctx, quit);
    ST_LOGGERCTX_CALL(events_ctx->logger_ctx, info,
     "events_simple: Event subsystem destroyed");
    free(events_ctx);
}

static st_evtypeid_t st_events_register_type(st_eventsctx_t *events_ctx,
 const char *type_name, size_t size) {
    st_evtype_t *evtype = &events_ctx->types[events_ctx->types_count];
    int          ret = snprintf(evtype->name, EVENT_TYPE_NAME_SIZE, "%s",
     type_name);

    if (ret < 0 || ret == EVENT_TYPE_NAME_SIZE) {
        ST_LOGGERCTX_CALL(events_ctx->logger_ctx, error,
         "events_simple: Unable to copy event type name while registering "
         "event type \"%s\"", type_name);

        return ST_EVTYPE_ID_NONE;
    }

    memset(evtype->subscribers, 0, sizeof(st_evq_t *) * SUBSCRIBERS_MAX);
    evtype->data_size = size;
    evtype->subscribers_count = 0;

    return (st_evtypeid_t)(events_ctx->types_count++);
}

static st_evtypeid_t st_events_get_type_id(st_eventsctx_t *events_ctx,
 const char *type_name) {
    for (size_t i = 0; i < events_ctx->types_count; i++) {
        if (strcmp(events_ctx->types[i].name, type_name) == 0)
            return (st_evtypeid_t)i;
    }

    return ST_EVTYPE_ID_NONE;
}

static st_evq_t *st_events_create_queue(st_eventsctx_t *events_ctx,
 size_t pool_size) {
    st_evq_t  *queue;
    st_rbuf_t *handle = ST_RBUFCTX_CALL(events_ctx->rbuf_ctx, create, pool_size);

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
    st_events_unsubscribe_all(queue);

    ST_RBUF_CALL(queue->handle, destroy);
    free(queue);
}

static bool st_events_subscribe(st_evq_t *queue, st_evtypeid_t type_id) {
    st_eventsctx_t *events_ctx = st_object_get_owner(queue);
    st_evtype_t    *evtype;

    if (type_id >= (st_evtypeid_t)events_ctx->types_count)
        return false;

    evtype = &events_ctx->types[type_id];

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
    st_eventsctx_t *events_ctx = st_object_get_owner(queue);
    st_evtype_t    *evtype;

    if (type_id >= (st_evtypeid_t)events_ctx->types_count)
        return;

    evtype = &events_ctx->types[type_id];

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
    st_eventsctx_t *events_ctx = st_object_get_owner(queue);

    for (int i = 0; i < (st_evtypeid_t)events_ctx->types_count; i++)
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

static void st_events_push(st_eventsctx_t *events_ctx, st_evtypeid_t type_id,
 const void *data) {
    st_evtype_t *evtype;

    if (type_id >= (st_evtypeid_t)events_ctx->types_count)
        return;

    evtype = &events_ctx->types[type_id];

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
    return ST_RBUF_CALL(queue->handle, is_empty);
}

static st_evtypeid_t st_events_peek_type(const st_evq_t *queue) {
    st_evtypeid_t type_id;
    bool          success = ST_RBUF_CALL(queue->handle, peek, &type_id,
     sizeof(st_evtypeid_t));

    return success ? type_id : ST_EVTYPE_ID_NONE;
}

static bool st_events_pop(st_evq_t *queue, void *data) {
    st_eventsctx_t *events_ctx = st_object_get_owner(queue);
    st_evtypeid_t   type_id;

    if (!ST_RBUF_CALL(queue->handle, pop, &type_id, sizeof(st_evtypeid_t)))
        return false;

    return ST_RBUF_CALL(queue->handle, pop, data,
     events_ctx->types[type_id].data_size);
}

static bool st_events_drop(st_evq_t *queue) {
    st_eventsctx_t *events_ctx = st_object_get_owner(queue);
    st_evtypeid_t   type_id;

    if (!ST_RBUF_CALL(queue->handle, pop, &type_id, sizeof(st_evtypeid_t)))
        return false;

    return ST_RBUF_CALL(queue->handle, drop, events_ctx->types[type_id].data_size);
}

static bool st_events_clear(st_evq_t *queue) {
    return ST_RBUF_CALL(queue->handle, clear);
}
