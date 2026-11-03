#pragma once

#include "steroids/modsmgr.h"
#include "steroids/modules/sdl3loader.h"
#include "steroids/modules/sound.h"
#include "steroids/modules/logger.h"

#include <SDL3/SDL.h>

typedef struct {
    bool (*load_wav)(const char *path, SDL_AudioSpec *spec, Uint8 **audio_buf,
     Uint32 *audio_len);
    void (*free)(void *ptr);
} sdl3_funcs_t;

typedef struct st_sndcodecctx {
    st_modctx_t;
    st_modsmgr_t       *modsmgr;
    st_loggerctx_t     *logger_ctx;
    st_soundctx_t      *sound_ctx;
    st_sdl3loaderctx_t *sdl3loader_ctx;
    sdl3_funcs_t        sdl3;
} st_sndcodecctx_t;

#define ST_SNDCODECCTX_T_DEFINED
