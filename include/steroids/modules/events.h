#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#define ST_EVTYPE_ID_NONE -1

typedef int st_evtypeid_t;

#ifndef ST_EVENTSCTX_T_DEFINED
    typedef st_modctx_t st_eventsctx_t;
#endif
#ifndef ST_EVQ_T_DEFINED
    typedef st_object_t st_evq_t;
#endif

typedef st_evtypeid_t (*st_events_register_type_t)(st_eventsctx_t *events_ctx,
 const char *type_name, size_t size);
typedef st_evtypeid_t (*st_events_get_type_id_t)(st_eventsctx_t *events_ctx,
 const char *type_name);
typedef st_evq_t *(*st_events_create_queue_t)(st_eventsctx_t *events_ctx,
 size_t pool_size);
typedef void (*st_events_destroy_queue_t)(st_evq_t *queue);
typedef bool (*st_events_subscribe_t)(st_evq_t *queue, st_evtypeid_t type_id);
typedef void (*st_events_unsubscribe_t)(st_evq_t *queue, st_evtypeid_t type_id);
typedef void (*st_events_unsubscribe_all_t)(st_evq_t *queue);
typedef void (*st_events_suspend_t)(st_evq_t *queue, bool clear);
typedef void (*st_events_resume_t)(st_evq_t *queue);
typedef void (*st_events_push_t)(st_eventsctx_t *events_ctx,
 st_evtypeid_t type_id, const void *data);
typedef bool (*st_events_is_empty_t)(const st_evq_t *queue);
typedef st_evtypeid_t (*st_events_peek_type_t)(const st_evq_t *queue);
typedef bool (*st_events_pop_t)(st_evq_t *queue, void *data);
typedef bool (*st_events_drop_t)(st_evq_t *queue);
typedef bool (*st_events_clear_t)(st_evq_t *queue);

typedef struct {
    st_modctx_funcs_t;
    st_events_register_type_t register_type;
    st_events_get_type_id_t   get_type_id;
    st_events_create_queue_t  create_queue;
    st_events_push_t          push;
} st_eventsctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_events_subscribe_t       subscribe;
    st_events_unsubscribe_t     unsubscribe;
    st_events_unsubscribe_all_t unsubscribe_all;
    st_events_suspend_t         suspend;
    st_events_resume_t          resume;
    st_events_is_empty_t        is_empty;
    st_events_peek_type_t       peek_type;
    st_events_pop_t             pop;
    st_events_drop_t            drop;
    st_events_clear_t           clear;
} st_evq_funcs_t;

#define ST_EVENTSCTX_CALL(ctx, func, ...) \
    ((st_eventsctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_EVQ_CALL(object, func, ...) \
    ((st_evq_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
