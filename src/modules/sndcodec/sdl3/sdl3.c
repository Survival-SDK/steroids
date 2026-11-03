#include "sdl3.h"

#include <stdbool.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#include <SDL3/SDL.h>

#define WAV_AUDIO_FORMAT_INTEGER 1
#define WAV_AUDIO_FORMAT_FLOAT   3

static st_sndcodecctx_t *st_sndcodec_init(const st_param_t params[]);
static void st_sndcodec_quit(st_sndcodecctx_t *sndcodec_ctx);

static st_sound_t *st_sndcodec_load(st_sndcodecctx_t *sndcodec_ctx,
 const char *filename, st_soundtype_t type);

static st_sndcodecctx_funcs_t sndcodecctx_funcs = {
    ST_MODCTX_FUNCS,
    .load = st_sndcodec_load,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "sdl3loader", "sdl3", },
    { "sound", NULL, },
    {0},
};

st_moddata_t *st_module_sndcodec_sdl3_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("sndcodec", "sdl3", ST_MODULE_TYPE, mod_prereqs,
     st_sndcodec_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_sndcodec_sdl3_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "sndcodec";
static const char *st_module_name = "sdl3";

static void *load_sdl3_func(st_sndcodecctx_t *sndcodec_ctx,
 bool *all_funcs_loaded, const char *func_name) {
    void *func = ST_SDL3LOADERCTX_CALL(sndcodec_ctx->sdl3loader_ctx,
     get_proc_address, func_name);

    if (!func)
        ST_LOGGERCTX_CALL(sndcodec_ctx->logger_ctx, error,
         "%s_%s: Unable to load function \"%s\"", st_module_subsystem,
         st_module_name, func_name);

    *all_funcs_loaded &= !!func;

    return func;
}

static bool load_sdl3_funcs(st_sndcodecctx_t *sndcodec_ctx) {
    bool all_funcs_loaded = true;

    /* audio */
    sndcodec_ctx->sdl3.load_wav = load_sdl3_func(sndcodec_ctx,
     &all_funcs_loaded, "SDL_LoadWAV");

    /* misc */
    sndcodec_ctx->sdl3.free = load_sdl3_func(sndcodec_ctx, &all_funcs_loaded,
     "SDL_free");

    return all_funcs_loaded;
}

static st_sndcodecctx_t *st_sndcodec_init(const st_param_t params[]) {
    st_modsmgr_t       *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t     *logger_ctx;
    st_soundctx_t      *sound_ctx;
    st_sndcodecctx_t   *sndcodec_ctx;
    st_sdl3loaderctx_t *sdl3loader_ctx;

    if (!modsmgr)
        return NULL;

    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    /* We need get sound_ctx via params because sound_ctx is not available yet
     * as singleton when this context is created */
    sound_ctx = st_modctx_get_param_as_ptr(params, "sound_ctx");
    if (!sound_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get sound context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    if (ST_MODSMGR_CALL(modsmgr, have_singleton, "sdl3loader", "sdl3")) {
        sdl3loader_ctx = (st_sdl3loaderctx_t *)ST_MODSMGR_CALL(modsmgr,
         get_singleton, "sdl3loader", NULL);
        if (!sdl3loader_ctx) {
            ST_LOGGERCTX_CALL(logger_ctx, error,
             "%s_%s: Unable to get sdl3loader context", st_module_subsystem,
             st_module_name);

            return NULL;
        }
    } else {
        sdl3loader_ctx = (st_sdl3loaderctx_t *)ST_MODSMGR_CALL(modsmgr,
         create_singleton, "sdl3loader", NULL, (st_params_t){{0}});
        if (!sdl3loader_ctx) {
            ST_LOGGERCTX_CALL(logger_ctx, error,
             "%s_%s: Unable to create sdl3loader context", st_module_subsystem,
             st_module_name);

            return NULL;
        }
    }

    sndcodec_ctx = (st_sndcodecctx_t *)st_modctx_new(
     "sndcodec", "sdl3", sizeof(st_sndcodecctx_t), NULL, &sndcodecctx_funcs,
     (st_object_dtor_t)st_sndcodec_quit);
    if (!sndcodec_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: unable to create new sndcodec ctx object", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    sndcodec_ctx->modsmgr    = modsmgr;
    sndcodec_ctx->logger_ctx = logger_ctx;
    sndcodec_ctx->sound_ctx  = sound_ctx;
    sndcodec_ctx->sdl3loader_ctx = sdl3loader_ctx;
    if (!load_sdl3_funcs(sndcodec_ctx))
        goto load_funcs_fail;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: SDL3 audio codec initialized", st_module_subsystem, st_module_name);

    return sndcodec_ctx;

load_funcs_fail:
    free(sndcodec_ctx);

    return NULL;
}

static void st_sndcodec_quit(st_sndcodecctx_t *sndcodec_ctx) {
    ST_LOGGERCTX_CALL(sndcodec_ctx->logger_ctx, info,
     "%s_%s: SDL3 audio codec codec destroyed", st_module_subsystem,
     st_module_name);
    free(sndcodec_ctx);
}

static st_sndsamplefmt_t audiofmt_to_samplefmt(SDL_AudioFormat fmt) {
    switch (fmt) {
        case SDL_AUDIO_UNKNOWN:
            return ST_SND_SF_UNKNOWN;
        case SDL_AUDIO_U8:
            return ST_SND_SF_U8;
        case SDL_AUDIO_S8:
            return ST_SND_SF_S8;
        case SDL_AUDIO_S16LE:
            return ST_SND_SF_S16LE;
        case SDL_AUDIO_S16BE:
            return ST_SND_SF_S16BE;
        case SDL_AUDIO_S32LE:
            return ST_SND_SF_S32LE;
        case SDL_AUDIO_S32BE:
            return ST_SND_SF_S32BE;
        case SDL_AUDIO_F32LE:
            return ST_SND_SF_F32LE;
        case SDL_AUDIO_F32BE:
            return ST_SND_SF_F32BE;
        default:
            return ST_SND_SF_UNKNOWN;
    }
}

static void audiospec_to_pcmfmt(st_pcmfmt_t *dst_fmt,
 const SDL_AudioSpec *spec) {
     *dst_fmt = (st_pcmfmt_t) {
         .rate = spec->freq,
         .channels = spec->channels,
         .channels_layout = ST_SND_CL_INTERLEAVED,
         .sample_fmt = audiofmt_to_samplefmt(spec->format),
     };
}

static st_sound_t *st_sndcodec_load_preloaded(st_sndcodecctx_t *sndcodec_ctx,
 const char *filename) {
    st_sound_t *result = NULL;

    SDL_AudioSpec spec;          /* заполнится: format, channels, freq */
    Uint8        *data = NULL;   /* сырые PCM-сэмплы */
    Uint32        len = 0;       /* длина в байтах */
    st_pcmfmt_t   src_fmt;

    if (!sndcodec_ctx->sdl3.load_wav(filename, &spec, &data, &len))
        return NULL;

    audiospec_to_pcmfmt(&src_fmt, &spec);

    if (is_pcmfmt_valid(&src_fmt))
        result = ST_SOUNDCTX_CALL(sndcodec_ctx->sound_ctx, import_preloaded,
         len / SDL_AUDIO_FRAMESIZE(spec), &src_fmt, (const void *[]){data},
         len / src_fmt.channels);

    sndcodec_ctx->sdl3.free(data);

    return result;
}

static st_sound_t *st_sndcodec_load_streamed(
 __attribute__((unused)) st_sndcodecctx_t *sndcodec_ctx,
 __attribute__((unused)) const char *filename) {
    /* not implemented */

    return NULL;
}

static st_sound_t *st_sndcodec_load(st_sndcodecctx_t *sndcodec_ctx,
 const char *filename, st_soundtype_t type) {
    if (type == ST_SND_ST_PRELOAD)
        return st_sndcodec_load_preloaded(sndcodec_ctx, filename);
    else if (type == ST_SND_ST_STREAM)
        return st_sndcodec_load_streamed(sndcodec_ctx, filename);

    return NULL;
}
