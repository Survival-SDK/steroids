#pragma once

#include <X11/Xlib.h>

#include "steroids/modctx.h"
#include "steroids/modules/fnv1a.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_fnv1actx_t  *fnv1a_ctx;
    st_htablectx_t *htable_ctx;
    st_loggerctx_t *logger_ctx;
    Display        *display; /* Owned */
} st_monitorctx_t;

typedef struct {
    st_object_t;
    Display     *handle; /* Copy of display pointer from monitorctx */
    unsigned     index;
    // bool         is_primary;
    char        *name;
    unsigned     width;
    unsigned     height;
    st_htable_t *userdata;
} st_monitor_t;

#define ST_MONITORCTX_T_DEFINED
#define ST_MONITOR_T_DEFINED
