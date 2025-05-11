#pragma once

#include "steroids/module.h"
#include "steroids/object.h"

#ifndef ST_MONITOR_T_DEFINED
    typedef struct st_monitor_s st_monitor_t;
#endif

typedef st_modctx_t *(*st_monitor_init_t)(st_modctx_t *logger_ctx);
typedef void (*st_monitor_quit_t)(st_modctx_t *monitor_ctx);

typedef unsigned (*st_monitor_get_monitors_count_t)(st_modctx_t *monitor_ctx);
typedef st_monitor_t *(*st_monitor_open_t)(st_modctx_t *monitor_ctx,
 unsigned index);
typedef void (*st_monitor_release_t)(st_monitor_t *monitor);
typedef unsigned (*st_monitor_get_width_t)(st_monitor_t *monitor);
typedef unsigned (*st_monitor_get_height_t)(st_monitor_t *monitor);
typedef void *(*st_monitor_get_handle_t)(st_monitor_t *monitor);
typedef void (*st_monitor_set_userdata_t)(const st_monitor_t *monitor,
 const char *key, uintptr_t value);
typedef bool (*st_monitor_get_userdata_t)(const st_monitor_t *monitor,
 uintptr_t *dst, const char *key);

typedef struct {
    st_monitor_init_t               monitor_init;
    st_monitor_quit_t               monitor_quit;
    st_monitor_get_monitors_count_t monitor_get_monitors_count;
    st_monitor_open_t               monitor_open;
} st_monitorctx_funcs_t;

typedef struct {
    st_monitor_release_t      release;
    st_monitor_get_width_t    get_width;
    st_monitor_get_height_t   get_height;
    st_monitor_get_handle_t   get_handle;
    st_monitor_set_userdata_t set_userdata;
    st_monitor_get_userdata_t get_userdata;
} st_monitor_funcs_t;

#define ST_MONITOR_CALL(object, func, ...) \
    ((st_monitor_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
