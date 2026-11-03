#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/sound.h"
#include "steroids/modules/logger.h"

#include <stdio.h>

typedef struct st_sndstream {
    st_object_t;
    size_t data_start;
    size_t data_size;
    FILE  *fstream;
} st_sndstream_t;

typedef struct st_sndcodecctx {
    st_modctx_t;
    st_modsmgr_t   *modsmgr;
    st_loggerctx_t *logger_ctx;
    st_soundctx_t  *sound_ctx;
} st_sndcodecctx_t;

#define ST_SNDCODECCTX_T_DEFINED
#define ST_SNDSTREAM_T_DEFINED
