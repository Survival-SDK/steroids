#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_MONITORCTX_T_DEFINED
    typedef st_modctx_t st_monitorctx_t;
#endif
#ifndef ST_MONITOR_T_DEFINED
    typedef st_object_t st_monitor_t;
#endif

typedef unsigned (*st_monitor_get_monitors_count_t)(
 const st_monitorctx_t *monitor_ctx);
typedef st_monitor_t *(*st_monitor_open_t)(st_monitorctx_t *monitor_ctx,
 unsigned index);
typedef unsigned (*st_monitor_get_width_t)(const st_monitor_t *monitor);
typedef unsigned (*st_monitor_get_height_t)(const st_monitor_t *monitor);
typedef unsigned (*st_monitor_get_index_t)(const st_monitor_t *monitor);
typedef const char *(*st_monitor_get_name_t)(const st_monitor_t *monitor);
typedef bool (*st_monitor_is_primary_t)(const st_monitor_t *monitor);
typedef void *(*st_monitor_get_handle_t)(const st_monitor_t *monitor);
typedef void (*st_monitor_set_userdata_t)(const st_monitor_t *monitor,
 const char *key, uintptr_t value);
typedef bool (*st_monitor_get_userdata_t)(const st_monitor_t *monitor,
 uintptr_t *dst, const char *key);

typedef struct {
    st_modctx_funcs_t;
    st_monitor_get_monitors_count_t get_monitors_count;
    st_monitor_open_t               open;
} st_monitorctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_monitor_get_width_t    get_width;
    st_monitor_get_height_t   get_height;
    st_monitor_get_index_t    get_index;
    st_monitor_get_name_t     get_name;
    st_monitor_is_primary_t   is_primary;
    st_monitor_get_handle_t   get_handle;
    st_monitor_set_userdata_t set_userdata;
    st_monitor_get_userdata_t get_userdata;
} st_monitor_funcs_t;

#define ST_MONITORCTX_CALL(ctx, func, ...) \
    ((st_monitorctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_MONITOR_CALL(object, func, ...) \
    ((st_monitor_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
