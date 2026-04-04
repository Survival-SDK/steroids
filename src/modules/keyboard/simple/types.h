#pragma once

#include "steroids/modctx.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/events.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"

#define INPUT_SIZE 4

typedef enum {
    EV_KEY_PRESS = 0,
    EV_KEY_RELEASE,
    EV_KEY_INPUT,

    EV_MAX,
} evtype_index_t;

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_eventsctx_t *events_ctx;
    st_loggerctx_t *logger_ctx;
    st_htablectx_t *htable_ctx;
    st_evtypeid_t   evtypes[EV_MAX];
    st_evq_t       *evq;
    st_htable_t    *prev_state;
    st_htable_t    *cur_state;
    char            input[INPUT_SIZE];
} st_keyboardctx_t;

#define ST_KEYBOARDCTX_T_DEFINED
