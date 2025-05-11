#pragma once

#include "steroids/modules/events.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/keyboard.h"

#define INPUT_SIZE 4

typedef enum {
    EV_KEY_PRESS = 0,
    EV_KEY_RELEASE,
    EV_KEY_INPUT,

    EV_MAX,
} evtype_index_t;

typedef struct {
    st_modctx_t             *ctx;
    st_events_get_type_id_t  get_type_id;
    st_events_create_queue_t create_queue;
} st_keyboard_simple_events_t;

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_keyboard_simple_logger_t;

typedef struct {
    st_modctx_t       *ctx;
    st_htable_init_t   init;
    st_htable_quit_t   quit;
    st_htable_create_t create;
} st_keyboard_simple_htable_t;

typedef struct {
    st_keyboard_simple_events_t events;
    st_keyboard_simple_logger_t logger;
    st_keyboard_simple_htable_t htable;
    st_evtypeid_t               evtypes[EV_MAX];
    st_evq_t                   *evq;
    st_htable_t                *prev_state;
    st_htable_t                *cur_state;
    char                        input[INPUT_SIZE];
} st_keyboard_simple_t;
