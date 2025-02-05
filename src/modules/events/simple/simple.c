#include "simple.h"

#include <errno.h>
#include <stdio.h>

#define ERRMSGBUF_SIZE 128

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static void st_events_quit(st_eventsctx_t *events_ctx);
static st_evtypeid_t st_events_register_type(st_eventsctx_t *events_ctx,
 const char *type_name, size_t size);
static st_evtypeid_t st_events_get_type_id(st_eventsctx_t *events_ctx,
 const char *type_name);
static st_evq_t *st_events_create_queue(st_eventsctx_t *events_ctx,
 size_t pool_size);
static void st_events_destroy_queue(st_evq_t *queue);
static bool st_events_subscribe(st_evq_t *queue, st_evtypeid_t type_id);
static void st_events_unsubscribe(st_evq_t *queue, st_evtypeid_t type_id);
static void st_events_unsubscribe_all(st_evq_t *queue);
static void st_events_suspend(st_evq_t *queue, bool clear);
static void st_events_resume(st_evq_t *queue);
static void st_events_push(st_eventsctx_t *events_ctx, st_evtypeid_t type_id,
 const void *data);
static bool st_events_is_empty(const st_evq_t *queue);
static st_evtypeid_t st_events_peek_type(const st_evq_t *queue);
static bool st_events_pop(st_evq_t *queue, void *data);
static bool st_events_drop(st_evq_t *queue);
static bool st_events_clear(st_evq_t *queue);

static st_eventsctx_funcs_t eventsctx_funcs = {
    st_modctx_funcs,
    .register_type = st_events_register_type,
    .get_type_id   = st_events_get_type_id,
    .create_queue  = st_events_create_queue,
    .push          = st_events_push,
};

static st_evq_funcs_t evq_funcs = {
    st_object_funcs,
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

static st_moddata_t st_module_events_simple_data = {
    .name = "events",
    .type = ST_MODULE_TYPE,
    .subsystem = "simple",
    .prereqs = (st_modprerq_t[]){ 
        { "logger", NULL, },
        { "rbuf", NULL, },
        {0}, 
    },
    .ctor = st_events_init,
};

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
    st_eventsctx_t *events_ctx = (st_eventsctx_t *)st_modctx_new("events", 
     "simple", sizeof(st_eventsctx_t), NULL, (st_object_dtor_t)st_events_quit, 
     &eventsctx_funcs);

    if (!events_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "events_simple: unable to create new rbuf ctx object");

        return NULL;
    }

    rbuf_init = global_modsmgr_funcs.get_ctor(global_modsmgr, "rbuf", NULL);
    if (!rbuf_init) {
        ST_LOGGERCTX_CALL(logger_ctx, error, 
         "events_simple: unable to get rbuf ctor");

        goto get_ctor_fail;
    }

    events_ctx->rbuf_ctx = rbuf_init(logger_ctx);
    if (!events_ctx->rbuf_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error, 
         "events_simple: unable to create rbuf");
        
        goto rbuf_init_fail;
    }

    events_ctx->logger_ctx = ST_LOGGERCTX_CALL(logger_ctx, grab);
    events_ctx->types_count = 0;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "events_simple: Event subsystem initialized.");

    return events_ctx;

rbuf_init_fail:
get_ctor_fail:
    free(events_ctx);

    return NULL;
}

static void st_events_quit(st_eventsctx_t *events_ctx) {
    // TODO(edomin):
    // Unsubscribe all queues
    // Delete queues

    ST_RBUFCTX_CALL(events_ctx->rbuf_ctx, release);
    ST_LOGGERCTX_CALL(events_ctx->logger_ctx, info,
     "events_simple: Event subsystem destroyed");
    ST_LOGGERCTX_CALL(events_ctx->logger_ctx, release);
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
    char       errbuf[ERRMSGBUF_SIZE];
    st_evq_t  *queue;
    st_rbuf_t *handle = ST_RBUFCTX_CALL(events_ctx->rbuf_ctx, create, 
     pool_size);

    if (!handle)
        return NULL;

    queue = (st_evq_t *)st_object_new(
     sizeof(st_evq_t *), (st_object_dtor_t)st_events_destroy_queue, 
     &evq_funcs, (st_object_t *)events_ctx);

    if (!queue) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(events_ctx->logger_ctx, error,
             "events_simple: Unable to allocate memory for events queue "
             "object: %s", errbuf);

        ST_RBUF_CALL(handle, release);
        st_object_destroy((st_object_t *)queue);

        return NULL;
    }

    queue->handle = handle;
    queue->active = true;

    return queue;
}

static void st_events_destroy_queue(st_evq_t *queue) {
    st_events_unsubscribe_all(queue);

    ST_RBUF_CALL(queue->handle, release);
    free(queue);
}

static bool st_events_subscribe(st_evq_t *queue, st_evtypeid_t type_id) {
    st_eventsctx_t *events_ctx = st_weakptr_grab(st_object_get_owner(queue));
    st_evtype_t    *evtype;
    bool            result = false;

    if (events_ctx)
        return false;

    if (type_id >= (st_evtypeid_t)events_ctx->types_count)
        goto error;

    evtype = &events_ctx->types[type_id];

    for (size_t i = 0; i < evtype->subscribers_count; i++) {
        if (evtype->subscribers[i] == queue)
            goto success;
    }

    if (evtype->subscribers_count >= SUBSCRIBERS_MAX)
        goto error;

    evtype->subscribers[evtype->subscribers_count++] = st_object_grab(queue);

success:
    result = true;
error:
    st_object_release(events_ctx);

    return result;
}

static void st_events_unsubscribe(st_evq_t *queue, st_evtypeid_t type_id) {
    st_eventsctx_t *events_ctx = st_weakptr_grab(st_object_get_owner(queue));
    st_evtype_t    *evtype;

    if (events_ctx)
        return;
    
    if (type_id >= (st_evtypeid_t)events_ctx->types_count)
        goto release;

    evtype = &events_ctx->types[type_id];

    for (size_t i = 0; i < evtype->subscribers_count; i++) {
        if (evtype->subscribers[i] == queue) {
            st_object_release(evtype->subscribers[i]);

            if (i <= evtype->subscribers_count - 1) {
                memmove(&evtype->subscribers[i], 
                 &evtype->subscribers[evtype->subscribers_count - 1],
                 sizeof(st_evq_t *));
            }
            evtype->subscribers_count--;

            goto release;
        }
    }

release:
    st_object_release(events_ctx);
}

static void st_events_unsubscribe_all(st_evq_t *queue) {
    st_eventsctx_t *events_ctx = st_weakptr_grab(st_object_get_owner(queue));

    if (events_ctx)
        return;

    for (int i = 0; i < (st_evtypeid_t)events_ctx->types_count; i++)
        st_events_unsubscribe(queue, i);

    st_object_release(events_ctx);
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
    st_eventsctx_t *events_ctx = st_weakptr_grab(st_object_get_owner(queue));
    st_evtypeid_t   type_id;
    bool            result;

    if (events_ctx)
        return false;

    result = ST_RBUF_CALL(queue->handle, pop, &type_id, sizeof(st_evtypeid_t))
     && ST_RBUF_CALL(queue->handle, pop, data,
      events_ctx->types[type_id].data_size);

    st_object_release(events_ctx);

    return result;
}

static bool st_events_drop(st_evq_t *queue) {
    st_eventsctx_t *events_ctx = st_weakptr_grab(st_object_get_owner(queue));
    st_evtypeid_t   type_id;
    bool            result;

    if (events_ctx)
        return false;

    result = ST_RBUF_CALL(queue->handle, pop, &type_id, sizeof(st_evtypeid_t))
     && ST_RBUF_CALL(queue->handle, drop, events_ctx->types[type_id].data_size);

    st_object_release(events_ctx);

    return result;
}

static bool st_events_clear(st_evq_t *queue) {
    return ST_RBUF_CALL(queue->handle, clear);
}
