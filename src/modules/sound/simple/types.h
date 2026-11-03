#pragma once

#include <stddef.h>

#include "steroids/consts/sound.h"
#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"
#include "steroids/types/pcmfmt.h"

#define SND_CODECS_MAX 32

typedef struct {
    st_object_t;
    st_soundtype_t type;
    size_t         frames_per_channel;
    st_pcmfmt_t    src_fmt;
} st_sound_t;

typedef struct {
    st_sound_t;
    void **pcm;  /* 1 plane if interleaved, channels planes if planar */
} st_prldsound_t;

typedef struct {
    st_sound_t;
    st_modctx_t *codec_ctx;
    char        *asset_id;
    void        *handle;
} st_strmsound_t;

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    st_pcmfmt_t     mix_fmt;
    st_modctx_t    *codecs[SND_CODECS_MAX];
    size_t          codecs_count;
} st_soundctx_t;

#define ST_SOUNDCTX_T_DEFINED
#define ST_SOUND_T_DEFINED
#define ST_PRLDSOUND_T_DEFINED
#define ST_STRMSOUND_T_DEFINED
