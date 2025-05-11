#pragma once

#include "steroids/module.h"

typedef st_modctx_t *(*st_timer_init_t)(st_modctx_t *logger_ctx);
typedef void (*st_timer_quit_t)(st_modctx_t *timer_ctx);

typedef uint64_t (*st_timer_start_t)(st_modctx_t *timer_ctx);
typedef unsigned (*st_timer_get_elapsed_t)(st_modctx_t *timer_ctx,
 uint64_t start);
typedef void (*st_timer_sleep_t)(st_modctx_t *timer_ctx, unsigned ms);
typedef void (*st_timer_sleep_for_fps_t)(st_modctx_t *timer_ctx, unsigned fps);

typedef struct {
    st_timer_init_t          timer_init;
    st_timer_quit_t          timer_quit;
    st_timer_start_t         timer_start;
    st_timer_get_elapsed_t   timer_get_elapsed;
    st_timer_sleep_t         timer_sleep;
    st_timer_sleep_for_fps_t timer_sleep_for_fps;
} st_timer_funcs_t;
