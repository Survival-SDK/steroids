#pragma once

#include <stddef.h>

#include "steroids/consts/sound.h"
#include "steroids/modctx.h"
#include "steroids/object.h"
#include "steroids/types/pcmfmt.h"

struct st_sndstream;
struct st_sndcodecctx;

#ifndef ST_SOUNDCTX_T_DEFINED
    typedef st_modctx_t st_soundctx_t;
#endif
#ifndef ST_SOUND_T_DEFINED
    typedef st_object_t st_sound_t;
#endif
#ifndef ST_PRLDSOUND_T_DEFINED
    typedef st_sound_t st_prldsound_t;
#endif
#ifndef ST_STRMSOUND_T_DEFINED
    typedef st_sound_t st_strmsound_t;
#endif

typedef st_sound_t *(*st_sound_load_t)(st_soundctx_t *sound_ctx,
 const char *filename, st_soundtype_t type);
typedef st_sound_t *(*st_sound_import_preloaded_t)(st_soundctx_t *sound_ctx,
 size_t frames_per_channel, const st_pcmfmt_t *src_fmt, const void *pcm[],
 size_t pcm_len_per_channel);
typedef st_sound_t *(*st_sound_import_streamed_t)(st_soundctx_t *sound_ctx,
 struct st_sndstream *sndstream, size_t frames_per_channel,
 const st_pcmfmt_t *src_fmt, struct st_sndcodecctx *codec_ctx,
 const char *asset_id);
typedef const st_pcmfmt_t *(*st_sound_get_mix_fmt_t)(
 const st_soundctx_t *sound_ctx);
typedef st_soundtype_t (*st_sound_get_type_t)(const st_sound_t *sound);
typedef size_t (*st_sound_get_frames_per_channel_t)(const st_sound_t *sound);
typedef const st_pcmfmt_t *(*st_sound_get_fmt_t)(const st_sound_t *sound);

typedef struct {
    st_modctx_funcs_t;
    st_sound_load_t             load;
    st_sound_import_preloaded_t import_preloaded;
    st_sound_import_streamed_t  import_streamed;
    st_sound_get_mix_fmt_t      get_mix_fmt;
} st_soundctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_sound_get_type_t               get_type;
    st_sound_get_frames_per_channel_t get_frames_per_channel;
    st_sound_get_fmt_t                get_fmt;
} st_sound_funcs_t;

#define ST_SOUNDCTX_CALL(ctx, func, ...) \
    ((st_soundctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_SOUND_CALL(object, func, ...) \
    ((st_sound_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
