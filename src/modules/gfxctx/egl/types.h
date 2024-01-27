#pragma once

#include <EGL/egl.h>

#include "steroids/types/modules/logger.h"
#include "steroids/types/modules/monitor.h"
#include "steroids/types/modules/window.h"

#include "dlist.h"

typedef struct {
    st_modctx_t        *ctx;
    st_logger_debug_t   debug;
    st_logger_info_t    info;
    st_logger_warning_t warning;
    st_logger_error_t   error;
} st_gfxctx_egl_logger_t;

typedef struct {
    st_modctx_t            *ctx;
    st_monitor_get_handle_t get_handle;
} st_gfxctx_egl_monitor_t;

typedef struct {
    st_modctx_t            *ctx;
    st_window_get_handle_t  get_handle;
} st_gfxctx_egl_window_t;

typedef struct {
    st_gfxctx_egl_logger_t  logger;
    st_gfxctx_egl_monitor_t monitor;
    st_gfxctx_egl_window_t  window;
} st_gfxctx_egl_t;

typedef struct {
    struct st_gfxctx_s *ctx;
    unsigned            index;
} st_gfxctx_shared_data_t;

typedef struct st_gfxctx_s {
    st_modctx_t *ctx;
    st_window_t *window;
    EGLDisplay   display;
    EGLConfig    cfg;
    EGLSurface   surface;
    EGLContext   handle;
    int          gapi;
    bool         debug;
    st_dlist_t  *shared_data;
} st_gfxctx_t;

#define ST_GFXCTX_T_DEFINED