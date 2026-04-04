#pragma once

#include <stdbool.h>

#include "steroids/modctx.h"
#include "steroids/modules/dpsrvconn.h"
#include "steroids/object.h"

#ifndef ST_GFXCTXCTX_T_DEFINED
    typedef st_modctx_t st_gfxctxctx_t;
#endif
#ifndef ST_GFXCTX_T_DEFINED
    typedef st_object_t st_gfxctx_t;
#endif

typedef enum {
    ST_GAPI_GL1,
    ST_GAPI_GL11,
    ST_GAPI_GL12,
    ST_GAPI_GL13,
    ST_GAPI_GL14,
    ST_GAPI_GL15,
    ST_GAPI_GL2,
    ST_GAPI_GL21,
    ST_GAPI_GL3,
    ST_GAPI_GL31,
    ST_GAPI_GL32,
    ST_GAPI_GL33,
    ST_GAPI_GL4,
    ST_GAPI_GL41,
    ST_GAPI_GL42,
    ST_GAPI_GL43,
    ST_GAPI_GL44,
    ST_GAPI_GL45,
    ST_GAPI_GL46,
    ST_GAPI_ES1,
    ST_GAPI_ES11,
    ST_GAPI_ES2,
    ST_GAPI_ES3,
    ST_GAPI_ES31,
    ST_GAPI_ES32,

    ST_GAPI_MAX,
    ST_GAPI_UNKNOWN = ST_GAPI_MAX,
} st_gapi_t;

typedef st_gfxctx_t *(*st_gfxctx_create_t)(st_gfxctxctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, st_gapi_t api);
typedef st_gfxctx_t *(*st_gfxctx_create_shared_t)(st_gfxctxctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, st_gfxctx_t *other);
typedef bool (*st_gfxctx_make_current_t)(st_gfxctx_t *gfxctx);
typedef bool (*st_gfxctx_swap_buffers_t)(st_gfxctx_t *gfxctx);
typedef st_window_t *(*st_gfxctx_get_window_t)(st_gfxctx_t *gfxctx);
typedef st_gapi_t (*st_gfxctx_get_api_t)(const st_gfxctx_t *gfxctx);
typedef unsigned (*st_gfxctx_get_shared_index_t)(const st_gfxctx_t *gfxctx);
typedef bool (*st_gfxctx_debug_enabled_t)(const st_gfxctx_t *gfxctx);
typedef void (*st_gfxctx_set_userdata_t)(const st_gfxctx_t *gfxctx,
 const char *key, uintptr_t value);
typedef bool (*st_gfxctx_get_userdata_t)(const st_gfxctx_t *gfxctx,
 uintptr_t *dst, const char *key);

typedef struct {
    st_modctx_funcs_t;
    st_gfxctx_create_t        create;
    st_gfxctx_create_shared_t create_shared;
} st_gfxctxctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_gfxctx_make_current_t     make_current;
    st_gfxctx_swap_buffers_t     swap_buffers;
    st_gfxctx_get_window_t       get_window;
    st_gfxctx_get_api_t          get_api;
    st_gfxctx_get_shared_index_t get_shared_index;
    st_gfxctx_debug_enabled_t    debug_enabled;
    st_gfxctx_set_userdata_t     set_userdata;
    st_gfxctx_get_userdata_t     get_userdata;
} st_gfxctx_funcs_t;

#define ST_GFXCTXCTX_CALL(ctx, func, ...) \
    ((st_gfxctxctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_GFXCTX_CALL(object, func, ...) \
    ((st_gfxctx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
