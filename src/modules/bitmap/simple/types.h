#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

#include "dlist.h"

typedef struct {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
    st_dlist_t     *codecs;
} st_bitmapctx_t;

typedef struct {
    st_object_t;
    unsigned width;
    unsigned height;
    int      pixel_format;
    char     data[];
} st_bitmap_t;

#define ST_BITMAPCTX_T_DEFINED
#define ST_BITMAP_T_DEFINED
