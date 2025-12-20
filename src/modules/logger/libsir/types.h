#pragma once

#include <limits.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <sir.h>
#pragma GCC diagnostic pop

#include "dlist.h"
#include "steroids/modctx.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/events.h"

#define ST_POSTMORTEM_MSG_SIZE_MAX 131072

typedef void (*st_logcbk_t)(const char *msg, void *userdata);

typedef struct {
    char      filename[PATH_MAX];
    sirfileid file;
} st_logger_libsir_log_file_t;

typedef struct {
    st_logcbk_t func;
    void       *userdata;
    int         log_levels;
} st_logger_libsir_callback_t;

typedef struct st_loggerctx_s {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_eventsctx_t *events_ctx;
    st_dlist_t     *log_files;
    st_dlist_t     *callbacks;
    st_evtypeid_t   ev_log_output_debug;
    st_evtypeid_t   ev_log_output_info;
    st_evtypeid_t   ev_log_output_warning;
    st_evtypeid_t   ev_log_output_error;
    char            postmortem_msg[ST_POSTMORTEM_MSG_SIZE_MAX];
} st_loggerctx_t;

#define ST_LOGGERCTX_T_DEFINED
