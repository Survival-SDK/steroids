#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/modctx.h"
#include "steroids/types/object.h"

#define ST_EVTYPE_ID_NONE -1

typedef int st_evtypeid_t;

typedef struct st_evq_s st_evq_t;

struct st_eventsctx_s;
struct st_loggerctx_s;

typedef struct st_eventsctx_s *(*st_events_init_t)(
 struct st_loggerctx_s *logger_ctx);

typedef st_evtypeid_t (*st_events_register_type_t)(
 struct st_eventsctx_s *events_ctx, const char *type_name, size_t size);
typedef st_evtypeid_t (*st_events_get_type_id_t)(
 struct st_eventsctx_s *events_ctx, const char *type_name);
typedef struct st_evq_s *(*st_events_create_queue_t)(
 struct st_eventsctx_s *events_ctx, size_t pool_size);
typedef void (*st_events_destroy_queue_t)(struct st_evq_s *queue);
typedef bool (*st_events_subscribe_t)(struct st_evq_s *queue,
 st_evtypeid_t type_id);
typedef void (*st_events_unsubscribe_t)(struct st_evq_s *queue,
 st_evtypeid_t type_id);
typedef void (*st_events_unsubscribe_all_t)(struct st_evq_s *queue);
typedef void (*st_events_suspend_t)(struct st_evq_s *queue, bool clear);
typedef void (*st_events_resume_t)(struct st_evq_s *queue);
typedef void (*st_events_push_t)(struct st_eventsctx_s *events_ctx,
 st_evtypeid_t type_id, const void *data);
typedef bool (*st_events_is_empty_t)(const struct st_evq_s *queue);
typedef st_evtypeid_t (*st_events_peek_type_t)(const struct st_evq_s *queue);
typedef bool (*st_events_pop_t)(struct st_evq_s *queue, void *data);
typedef bool (*st_events_drop_t)(struct st_evq_s *queue);
typedef bool (*st_events_clear_t)(struct st_evq_s *queue);

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
    ((st_eventsctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, ## __VA_ARGS__)
#define ST_EVQ_CALL(object, func, ...) \
    ((st_evq_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
