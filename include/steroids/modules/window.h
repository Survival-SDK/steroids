#pragma once

#include <stdbool.h>

#include "steroids/modctx.h"
#include "steroids/modules/monitor.h"
#include "steroids/object.h"

#ifndef ST_WINDOWCTX_T_DEFINED
    typedef st_modctx_t st_windowctx_t;
#endif
#ifndef ST_WINDOW_T_DEFINED
    typedef st_object_t st_window_t;
#endif

typedef enum {
    WMB_LEFT = 0,
    WMB_MIDDLE,
    WMB_RIGHT,
} st_winmb_t;

typedef struct {
    st_window_t *window;
} st_evwinnoargs_t;

typedef struct {
    st_window_t *window;
    unsigned hvalue;
    unsigned vvalue;
} st_evwinuvec2_t;

typedef struct {
    st_window_t *window;
    unsigned     value;
} st_evwinunsigned_t;

typedef struct {
    st_window_t *window;
    uint64_t     value;
} st_evwinu64_t;

typedef struct {
    st_window_t *window;
    int          value;
} st_evwininteger_t;

typedef struct {
    st_window_t *window;
    char         value[4];
} st_evwinsymbol_t;

typedef st_window_t *(*st_window_create_t)(st_windowctx_t *window_ctx,
 st_monitor_t *monitor, int x, int y, unsigned width, unsigned height,
 bool fullscreen, const char *title);
typedef void (*st_window_process_t)(st_windowctx_t *window_ctx);
typedef bool (*st_window_xed_t)(const st_window_t *window);
typedef st_monitor_t *(*st_window_get_monitor_t)(const st_window_t *window);
typedef void *(*st_window_get_handle_t)(const st_window_t *window);
typedef unsigned (*st_window_get_width_t)(const st_window_t *window);
typedef unsigned (*st_window_get_height_t)(const st_window_t *window);

typedef struct {
    st_modctx_funcs_t;
    st_window_create_t  create;
    st_window_process_t process;
} st_windowctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_window_xed_t         xed;
    st_window_get_monitor_t get_monitor;
    st_window_get_handle_t  get_handle;
    st_window_get_width_t   get_width;
    st_window_get_height_t  get_height;
} st_window_funcs_t;

#define ST_WINDOWCTX_CALL(ctx, func, ...) \
    ((st_windowctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_WINDOW_CALL(object, func, ...) \
    ((st_window_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
