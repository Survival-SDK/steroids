#pragma once

#include <stdbool.h>

#include "steroids/module.h"
#include "steroids/types/modules/events.h"
#include "steroids/types/object.h"

#define ST_EV_LOG_MSG_SIZE 1000

struct st_loggerctx_s;
struct st_eventsctx_s;

typedef enum {
    ST_LL_NONE    = 0x0,
    ST_LL_ERROR   = 0x8,
    ST_LL_WARNING = 0x10,
    ST_LL_INFO    = 0x40,
    ST_LL_DEBUG   = 0x80,
    ST_LL_ALL     = 0xFF,
} st_loglvl_t;

typedef void (*st_logcbk_t)(const char *msg, void *userdata);
typedef void (*st_logger_generic_msg_t)(const struct st_loggerctx_s *logger_ctx,
 const char* format, ...);

typedef struct st_loggerctx_s *(*st_logger_init_t)(
 struct st_eventsctx_s *events_ctx);

typedef bool (*st_logger_enable_events_t)(struct st_loggerctx_s *logger_ctx,
 struct st_eventsctx_s *events_ctx);
typedef bool (*st_logger_set_stdout_levels_t)(struct st_loggerctx_s *logger_ctx,
 st_loglvl_t levels);
typedef bool (*st_logger_set_stderr_levels_t)(struct st_loggerctx_s *logger_ctx,
 st_loglvl_t levels);
typedef bool (*st_logger_set_log_file_t)(struct st_loggerctx_s *logger_ctx,
 const char *filename, st_loglvl_t levels);
typedef bool (*st_logger_set_callback_t)(struct st_loggerctx_s *logger_ctx,
 st_logcbk_t callback, void *userdata, st_loglvl_t levels);
typedef st_logger_generic_msg_t st_logger_debug_t;
typedef st_logger_generic_msg_t st_logger_info_t;
typedef st_logger_generic_msg_t st_logger_warning_t;
typedef st_logger_generic_msg_t st_logger_error_t;
typedef void (*st_logger_set_postmortem_msg_t)(
 struct st_loggerctx_s *logger_ctx, const char *msg);

typedef struct {
    st_modctx_funcs_t;
    st_logger_enable_events_t      enable_events;
    st_logger_set_stdout_levels_t  set_stdout_levels;
    st_logger_set_stderr_levels_t  set_stderr_levels;
    st_logger_set_log_file_t       set_log_file;
    st_logger_set_callback_t       set_callback;
    st_logger_debug_t              debug;
    st_logger_info_t               info;
    st_logger_warning_t            warning;
    st_logger_error_t              error;
    st_logger_set_postmortem_msg_t set_postmortem_msg;
} st_loggerctx_funcs_t;

#define ST_LOGGERCTX_CALL(ctx, func, ...) \
    ((st_loggerctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, ## __VA_ARGS__)

