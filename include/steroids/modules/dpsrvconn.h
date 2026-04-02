#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_DPSRVCONNCTX_T_DEFINED
    typedef st_modctx_t st_dpsrvconnctx_t;
#endif
#ifndef ST_MONITOR_T_DEFINED
    typedef st_object_t st_monitor_t;
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
    st_monitor_t *monitor;
    uintptr_t     id;
    unsigned      index;
} st_evmonconn_t;

typedef struct {
    uintptr_t id;
    unsigned  index;
} st_evmondisc_t;

typedef struct {
    st_monitor_t *monitor;
    unsigned      new_index;
} st_evmonreidx_t;

typedef struct {
    st_monitor_t *monitor;
    unsigned      new_width;
    unsigned      new_height;
} st_evmonresize_t;

typedef struct {
    st_window_t *window;
} st_evwinnoargs_t;

typedef struct {
    st_window_t *window;
    void        *ptr;
} st_evwinptr_t;

typedef struct {
    st_window_t *window;
    unsigned     hvalue;
    unsigned     vvalue;
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

typedef int (*st_dpsrvconn_get_monitors_count_t)(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
typedef int (*st_dpsrvconn_get_primary_monitor_index_t)(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
typedef const st_monitor_t *(*st_dpsrvconn_get_monitor_by_index_t)(
 const st_dpsrvconnctx_t *dpsrvconn_ctx, unsigned index);
typedef st_monitor_t *(*st_dpsrvconn_get_monitor_by_id_t)(
 st_dpsrvconnctx_t *dpsrvconn_ctx, uintptr_t id);
typedef const st_monitor_t *(*st_dpsrvconn_get_primary_monitor_t)(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
typedef st_window_t *(*st_dpsrvconn_open_window_t)(
 st_dpsrvconnctx_t *dpsrvconn_ctx, st_monitor_t *monitor, int x, int y, 
 unsigned width, unsigned height, bool fullscreen, const char *title);
typedef void (*st_dpsrvconn_process_t)(st_dpsrvconnctx_t *dpsrvconn_ctx);

typedef unsigned (*st_dpsrvconn_get_monitor_width_t)(
 const st_monitor_t *monitor);
typedef unsigned (*st_dpsrvconn_get_monitor_height_t)(
 const st_monitor_t *monitor);
typedef int (*st_dpsrvconn_get_monitor_index_t)(const st_monitor_t *monitor);
typedef const char *(*st_dpsrvconn_get_monitor_name_t)(
 const st_monitor_t *monitor);
typedef bool (*st_dpsrvconn_is_monitor_primary_t)(const st_monitor_t *monitor);
typedef void *(*st_dpsrvconn_get_monitor_device_handle_t)(
 const st_monitor_t *monitor);
typedef void *(*st_dpsrvconn_get_monitor_native_device_handle_t)(
 const st_monitor_t *monitor);

typedef bool (*st_dpsrvconn_is_window_xed_t)(const st_window_t *window);
typedef const st_monitor_t *(*st_dpsrvconn_get_window_monitor_t)(
 const st_window_t *window);
typedef void *(*st_dpsrvconn_get_window_handle_t)(const st_window_t *window);
typedef void *(*st_dpsrvconn_get_window_native_handle_t)(const st_window_t *window);
typedef unsigned (*st_dpsrvconn_get_window_width_t)(const st_window_t *window);
typedef unsigned (*st_dpsrvconn_get_window_height_t)(const st_window_t *window);

typedef struct {
    st_modctx_funcs_t;
    st_dpsrvconn_get_monitors_count_t        get_monitors_count;
    st_dpsrvconn_get_primary_monitor_index_t get_primary_monitor_index;
    st_dpsrvconn_get_monitor_by_index_t      get_monitor_by_index;
    st_dpsrvconn_get_monitor_by_id_t         get_monitor_by_id;
    st_dpsrvconn_get_primary_monitor_t       get_primary_monitor;
    st_dpsrvconn_open_window_t               open_window;
    st_dpsrvconn_process_t                   process;
} st_dpsrvconnctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_dpsrvconn_get_monitor_width_t                get_width;
    st_dpsrvconn_get_monitor_height_t               get_height;
    st_dpsrvconn_get_monitor_index_t                get_index;
    st_dpsrvconn_get_monitor_name_t                 get_name;
    st_dpsrvconn_is_monitor_primary_t               is_primary;
    st_dpsrvconn_get_monitor_device_handle_t        get_device_handle;
    st_dpsrvconn_get_monitor_native_device_handle_t get_native_device_handle;
} st_monitor_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_dpsrvconn_is_window_xed_t            xed;
    st_dpsrvconn_get_window_monitor_t       get_monitor;
    st_dpsrvconn_get_window_handle_t        get_handle;
    st_dpsrvconn_get_window_native_handle_t get_native_handle;
    st_dpsrvconn_get_window_width_t         get_width;
    st_dpsrvconn_get_window_height_t        get_height;
} st_window_funcs_t;

#define ST_DPSRVCONNCTX_CALL(ctx, func, ...) \
    ((st_dpsrvconnctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_MONITOR_CALL(object, func, ...) \
    ((st_monitor_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
#define ST_WINDOW_CALL(object, func, ...) \
    ((st_window_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
