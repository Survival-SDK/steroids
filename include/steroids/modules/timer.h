#pragma once

#include <stdint.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_TIMERCTX_T_DEFINED
    typedef st_modctx_t st_timerctx_t;
#endif

typedef uint64_t (*st_timer_start_t)(const st_timerctx_t *timer_ctx);
typedef unsigned (*st_timer_get_elapsed_t)(const st_timerctx_t *timer_ctx,
 uint64_t start);
typedef void (*st_timer_sleep_t)(const st_timerctx_t *timer_ctx, unsigned ms);
typedef void (*st_timer_sleep_for_fps_t)(const st_timerctx_t *timer_ctx, 
 unsigned fps);

typedef struct {
    st_modctx_funcs_t;
    st_timer_start_t         start;
    st_timer_get_elapsed_t   get_elapsed;
    st_timer_sleep_t         sleep;
    st_timer_sleep_for_fps_t sleep_for_fps;
} st_timerctx_funcs_t;

#define ST_TIMERCTX_CALL(ctx, func, ...) \
    ((st_timerctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
