#pragma once

#include "config.h" // IWYU pragma: keep
#include "steroids/timer.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_timer_simple_logger_t;

typedef struct {
    st_timer_simple_logger_t logger;
} st_timer_simple_t;

static st_timer_funcs_t st_timer_simple_funcs = {
    .timer_init          = st_timer_init,
    .timer_quit          = st_timer_quit,
    .timer_start         = st_timer_start,
    .timer_get_elapsed   = st_timer_get_elapsed,
    .timer_sleep         = st_timer_sleep,
    .timer_sleep_for_fps = st_timer_sleep_for_fps,
};

static st_modfuncentry_t st_module_timer_simple_funcs[] = {
    {"init"         , st_timer_init},
    {"quit"         , st_timer_quit},
    {"start"        , st_timer_start},
    {"get_elapsed"  , st_timer_get_elapsed},
    {"sleep"        , st_timer_sleep},
    {"sleep_for_fps", st_timer_sleep_for_fps},
    {NULL, NULL},
};
