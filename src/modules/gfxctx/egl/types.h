#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "steroids/modctx.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/fnv1a.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/monitor.h"
#include "steroids/modules/window.h"
#include "steroids/object.h"

#include "dlist.h"

typedef struct {
    st_modctx_t;
    st_modsmgr_t     *modsmgr;
    st_loggerctx_t   *logger_ctx;
    st_monitorctx_t  *monitor_ctx;
    st_windowctx_t   *window_ctx;
    st_fnv1actx_t    *fnv1a_ctx;
    st_htablectx_t   *htable_ctx;
    bool              debug_enabled;
    EGLint          (*egl_debug_message_control_khr)(
     EGLDEBUGPROCKHR callback, const EGLAttrib *attrib_list);
    EGLint          (*egl_label_object_khr)(EGLDisplay display,
     EGLenum objectType, EGLObjectKHR object, EGLLabelKHR label);
    bool              must_quit;
    size_t            gfxctxs_count;
} st_gfxctxctx_t;

typedef struct {
    st_object_t;
    st_window_t *window;
    EGLDisplay   display;
    EGLConfig    cfg;
    EGLSurface   surface;
    EGLContext   handle;
    int          gapi;
    bool         debug;
    st_dlist_t  *shared_data;
    st_htable_t *userdata;
} st_gfxctx_t;

typedef struct {
    st_gfxctx_t *ctx;
    unsigned     index;
} st_gfxctx_shared_data_t;

#define ST_GFXCTXCTX_T_DEFINED
#define ST_GFXCTX_T_DEFINED
