#pragma once

#include "steroids/module.h"
#include "steroids/modules/window.h"

typedef enum {
    ST_MB_LEFT = 1,
    ST_MB_MIDDLE,
    ST_MB_RIGHT,

    ST_MB_MAX,
} st_mbutton_t;

typedef st_modctx_t *(*st_mouse_init_t)(st_modctx_t *events_ctx,
 st_modctx_t *logger_ctx);
typedef void (*st_mouse_quit_t)(st_modctx_t *mouse_ctx);

typedef void (*st_mouse_process_t)(st_modctx_t *mouse_ctx);
typedef bool (*st_mouse_press_t)(const st_modctx_t *mouse_ctx,
 st_mbutton_t button);
typedef bool (*st_mouse_release_t)(const st_modctx_t *mouse_ctx,
 st_mbutton_t button);
typedef bool (*st_mouse_pressed_t)(const st_modctx_t *mouse_ctx,
 st_mbutton_t button);
typedef int (*st_mouse_get_wheel_relative_t)(const st_modctx_t *mouse_ctx);
typedef bool (*st_mouse_moved_t)(const st_modctx_t *mouse_ctx);
typedef bool (*st_mouse_entered_t)(const st_modctx_t *mouse_ctx);
typedef bool (*st_mouse_leaved_t)(const st_modctx_t *mouse_ctx);
typedef unsigned (*st_mouse_get_x_t)(const st_modctx_t *mouse_ctx);
typedef unsigned (*st_mouse_get_y_t)(const st_modctx_t *mouse_ctx);
typedef const st_window_t *(*st_mouse_get_window_t)(
 const st_modctx_t *mouse_ctx);

typedef struct {
    st_mouse_init_t               mouse_init;
    st_mouse_quit_t               mouse_quit;
    st_mouse_process_t            mouse_process;
    st_mouse_press_t              mouse_press;
    st_mouse_release_t            mouse_release;
    st_mouse_pressed_t            mouse_pressed;
    st_mouse_get_wheel_relative_t mouse_get_wheel_relative;
    st_mouse_moved_t              mouse_moved;
    st_mouse_entered_t            mouse_entered;
    st_mouse_leaved_t             mouse_leaved;
    st_mouse_get_x_t              mouse_get_x;
    st_mouse_get_y_t              mouse_get_y;
    st_mouse_get_window_t         mouse_get_window;
} st_mouse_funcs_t;
