#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/sound.h"
#include "steroids/object.h"

#include <stddef.h>

#ifndef ST_SNDCODECCTX_T_DEFINED
    typedef st_modctx_t st_sndcodecctx_t;
#endif

#ifndef ST_SNDSTREAM_T_DEFINED
    typedef st_object_t st_sndstream_t;
#endif

typedef st_sound_t *(*st_sndcodec_load_t)(st_sndcodecctx_t *sndcodec_ctx,
 const char *filename, st_soundtype_t type);

typedef size_t (*st_sndcodec_stream_read_pcm_t)(st_sndstream_t *sndstream, void *buf,
 size_t len);

typedef struct {
    st_modctx_funcs_t;
    st_sndcodec_load_t load;
} st_sndcodecctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_sndcodec_stream_read_pcm_t read_pcm;
} st_sndstream_funcs_t;

#define ST_SNDCODECCTX_CALL(ctx, func, ...) \
    ((st_sndcodecctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_SNDSTREAM_CALL(ctx, func, ...) \
    ((st_sndstream_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
