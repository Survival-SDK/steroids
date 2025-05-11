#pragma once

#include <X11/Xlib.h>

#include "steroids/modules/fnv1a.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_monitor_xlib_logger_t;

typedef struct {
    st_fnv1a_get_u32hashstr_func_t get_u32hashstr_func;
} st_monitor_xlib_fnv1a_t;

typedef struct {
    st_modctx_t       *ctx;
    st_htable_init_t   init;
    st_htable_quit_t   quit;
    st_htable_create_t create;
} st_monitor_xlib_htable_t;

typedef struct {
    st_monitor_xlib_logger_t logger;
    st_monitor_xlib_fnv1a_t  fnv1a;
    st_monitor_xlib_htable_t htable;
} st_monitor_xlib_t;

ST_CLASS (
    Display *handle;
    unsigned index;
    st_htable_t *userdata;
) st_monitor_t;

#define ST_MONITOR_T_DEFINED
