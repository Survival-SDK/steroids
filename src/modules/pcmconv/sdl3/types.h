#pragma once

#include <stddef.h>

#include <SDL3/SDL.h>

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"
#include "steroids/types/pcmfmt.h"

typedef struct {
    SDL_AudioStream *(*create_audio_stream)(const SDL_AudioSpec *src_spec,
     const SDL_AudioSpec *dst_spec);
    bool (*put_audio_stream_data)(SDL_AudioStream *stream, const void *buf,
     int len);
    bool (*put_audio_stream_planar_data)(SDL_AudioStream *stream,
     const void * const *channel_buffers, int num_channels, int num_samples);
    int (*get_audio_stream_data)(SDL_AudioStream *stream, void *buf, int len);
    int (*get_audio_stream_available)(SDL_AudioStream *stream);
    bool (*flush_audio_stream)(SDL_AudioStream *stream);
    void (*destroy_audio_stream)(SDL_AudioStream *stream);

    const char *(*get_error)(void);
} sdl3_funcs_t;

typedef struct {
    st_object_t;
    st_pcmfmt_t      input_fmt;
    st_pcmfmt_t      output_fmt;
    SDL_AudioStream *stream;
} st_pcmconv_t;

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
    sdl3_funcs_t    sdl3;
} st_pcmconvctx_t;

#define ST_PCMCONVCTX_T_DEFINED
#define ST_PCMCONV_T_DEFINED
