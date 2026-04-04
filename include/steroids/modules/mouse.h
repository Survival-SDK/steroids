#pragma once

#include "steroids/consts/mouse.h"
#include "steroids/modctx.h"
#include "steroids/modules/dpsrvconn.h"
#include "steroids/object.h"

#ifndef ST_MOUSECTX_T_DEFINED
    typedef st_modctx_t st_mousectx_t;
#endif

typedef void (*st_mouse_process_t)(st_mousectx_t *mouse_ctx);
typedef bool (*st_mouse_press_t)(const st_mousectx_t *mouse_ctx,
 st_mbutton_t button);
typedef bool (*st_mouse_release_t)(const st_mousectx_t *mouse_ctx,
 st_mbutton_t button);
typedef bool (*st_mouse_pressed_t)(const st_mousectx_t *mouse_ctx,
 st_mbutton_t button);
typedef int (*st_mouse_get_wheel_relative_t)(const st_mousectx_t *mouse_ctx);
typedef bool (*st_mouse_moved_t)(const st_mousectx_t *mouse_ctx);
typedef bool (*st_mouse_entered_t)(const st_mousectx_t *mouse_ctx);
typedef bool (*st_mouse_leaved_t)(const st_mousectx_t *mouse_ctx);
typedef unsigned (*st_mouse_get_x_t)(const st_mousectx_t *mouse_ctx);
typedef unsigned (*st_mouse_get_y_t)(const st_mousectx_t *mouse_ctx);
typedef const st_window_t *(*st_mouse_get_window_t)(
 const st_mousectx_t *mouse_ctx);

typedef struct {
    st_modctx_funcs_t;
    st_mouse_process_t            process;
    st_mouse_press_t              press;
    st_mouse_release_t            release;
    st_mouse_pressed_t            pressed;
    st_mouse_get_wheel_relative_t get_wheel_relative;
    st_mouse_moved_t              moved;
    st_mouse_entered_t            entered;
    st_mouse_leaved_t             leaved;
    st_mouse_get_x_t              get_x;
    st_mouse_get_y_t              get_y;
    st_mouse_get_window_t         get_window;
} st_mousectx_funcs_t;

#define ST_MOUSECTX_CALL(ctx, func, ...) \
    ((st_mousectx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
