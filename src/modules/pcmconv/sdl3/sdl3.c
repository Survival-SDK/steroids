#include "sdl3.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include <SDL3/SDL.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/sdl3loader.h"

static st_pcmconvctx_t *st_pcmconv_init(const st_param_t params[]);
static void st_pcmconv_quit(st_pcmconvctx_t *pcmconv_ctx);

static st_pcmconv_t *st_pcmconv_create_converter(st_pcmconvctx_t *pcmconv_ctx,
 const st_pcmfmt_t *input_fmt, const st_pcmfmt_t *output_fmt);

static void st_pcmconv_destroy_converter(st_pcmconv_t *converter);

static bool st_pcmconv_put_pcm(st_pcmconv_t *converter, const void **pcm,
 size_t pcm_len_per_channel);
static ssize_t st_pcmconv_get_available_pcm_size_per_channel(
 st_pcmconv_t *converter);
static ssize_t st_pcmconv_get_pcm(st_pcmconv_t *converter, void **dst,
 size_t dst_size_per_channel);
static bool st_pcmconv_flush(st_pcmconv_t *converter);

static st_pcmconvctx_funcs_t pcmconvctx_funcs = {
    ST_MODCTX_FUNCS,
    .create_converter = st_pcmconv_create_converter,
};

static st_pcmconv_funcs_t pcmconv_funcs = {
    ST_OBJECT_FUNCS,
    .put_pcm                            = st_pcmconv_put_pcm,
    .get_available_pcm_size_per_channel
     = st_pcmconv_get_available_pcm_size_per_channel,
    .get_pcm                            = st_pcmconv_get_pcm,
    .flush                          = st_pcmconv_flush,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "sdl3loader", "sdl3", },
    {0},
};

st_moddata_t *st_module_pcmconv_sdl3_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("pcmconv", "sdl3", ST_MODULE_TYPE, mod_prereqs,
     st_pcmconv_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_pcmconv_sdl3_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "pcmconv";
static const char *st_module_name = "sdl3";

static void *load_sdl3_func(st_pcmconvctx_t *pcmconv_ctx,
 st_sdl3loaderctx_t *sdl3loader_ctx, bool *all_funcs_loaded,
 const char *func_name) {
    void *func = ST_SDL3LOADERCTX_CALL(sdl3loader_ctx,
     get_proc_address, func_name);

    if (!func)
        ST_LOGGERCTX_CALL(pcmconv_ctx->logger_ctx, error,
         "%s_%s: Unable to load function \"%s\"", st_module_subsystem,
         st_module_name, func_name);

    *all_funcs_loaded &= !!func;

    return func;
}

static bool load_sdl3_funcs(st_pcmconvctx_t *pcmconv_ctx,
 st_sdl3loaderctx_t *sdl3loader_ctx) {
    bool all_funcs_loaded = true;

    /* audio */
    pcmconv_ctx->sdl3.create_audio_stream = load_sdl3_func(pcmconv_ctx,
     sdl3loader_ctx, &all_funcs_loaded, "SDL_CreateAudioStream");
    pcmconv_ctx->sdl3.put_audio_stream_data = load_sdl3_func(pcmconv_ctx,
     sdl3loader_ctx, &all_funcs_loaded, "SDL_PutAudioStreamData");
    pcmconv_ctx->sdl3.put_audio_stream_planar_data = load_sdl3_func(
     pcmconv_ctx, sdl3loader_ctx, &all_funcs_loaded,
     "SDL_PutAudioStreamPlanarData");
    pcmconv_ctx->sdl3.get_audio_stream_data = load_sdl3_func(pcmconv_ctx,
     sdl3loader_ctx, &all_funcs_loaded, "SDL_GetAudioStreamData");
    pcmconv_ctx->sdl3.get_audio_stream_available = load_sdl3_func(pcmconv_ctx,
     sdl3loader_ctx, &all_funcs_loaded, "SDL_GetAudioStreamAvailable");
    pcmconv_ctx->sdl3.flush_audio_stream = load_sdl3_func(pcmconv_ctx,
     sdl3loader_ctx, &all_funcs_loaded, "SDL_FlushAudioStream");
    pcmconv_ctx->sdl3.destroy_audio_stream = load_sdl3_func(pcmconv_ctx,
     sdl3loader_ctx, &all_funcs_loaded, "SDL_DestroyAudioStream");

    /* misc */
    pcmconv_ctx->sdl3.get_error = load_sdl3_func(pcmconv_ctx, sdl3loader_ctx,
     &all_funcs_loaded, "SDL_GetError");

    return all_funcs_loaded;
}

static st_pcmconvctx_t *st_pcmconv_init(const st_param_t params[]) {
    st_sdl3loaderctx_t *sdl3loader_ctx;
    st_loggerctx_t     *logger_ctx;
    st_pcmconvctx_t    *pcmconv_ctx;
    st_modsmgr_t       *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");

    if (!modsmgr)
        return NULL;

    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

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

    pcmconv_ctx = (st_pcmconvctx_t *)st_modctx_new("pcmconv", "sdl3",
     sizeof(st_pcmconvctx_t), NULL, &pcmconvctx_funcs,
     (st_object_dtor_t)st_pcmconv_quit);

    if (!pcmconv_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: unable to create new PCM-converter ctx object",
         st_module_subsystem, st_module_name);

        return NULL;
    }

    pcmconv_ctx->logger_ctx = logger_ctx;
    if (!load_sdl3_funcs(pcmconv_ctx, sdl3loader_ctx))
        goto load_funcs_fail;

    ST_LOGGERCTX_CALL(logger_ctx, info, "%s_%s: PCM-converter ctx initialized.",
     st_module_subsystem, st_module_name);

    return pcmconv_ctx;

load_funcs_fail:
    free(pcmconv_ctx);

    return NULL;
}

static void st_pcmconv_quit(st_pcmconvctx_t *pcmconv_ctx) {
    ST_LOGGERCTX_CALL(pcmconv_ctx->logger_ctx, info,
     "%s_%s: PCM-converter ctx destroyed", st_module_subsystem, st_module_name);
    free(pcmconv_ctx);
}

static SDL_AudioFormat samplefmt_to_audiofmt(st_sndsamplefmt_t fmt) {
    switch (fmt) {
        case ST_SND_SF_UNKNOWN :
            return SDL_AUDIO_UNKNOWN;
        case ST_SND_SF_U8:
            return SDL_AUDIO_U8;
        case ST_SND_SF_S8:
            return SDL_AUDIO_S8;
        case ST_SND_SF_S16LE:
            return SDL_AUDIO_S16LE;
        case ST_SND_SF_S16BE:
            return SDL_AUDIO_S16BE;
        case ST_SND_SF_S32LE:
            return SDL_AUDIO_S32LE;
        case ST_SND_SF_S32BE:
            return SDL_AUDIO_S32BE;
        case ST_SND_SF_F32LE:
            return SDL_AUDIO_F32LE;
        case ST_SND_SF_F32BE:
            return SDL_AUDIO_F32BE;
        default:
            return SDL_AUDIO_UNKNOWN;
    }
}

static void pcmfmt_to_sdl3audiospec(SDL_AudioSpec *dst,
 const st_pcmfmt_t *pmcfmt) {
    *dst = (struct SDL_AudioSpec){
        .format = samplefmt_to_audiofmt(pmcfmt->sample_fmt),
        .channels = pmcfmt->channels,
        .freq = pmcfmt->rate,
    };
}

static st_pcmconv_t *st_pcmconv_create_converter(st_pcmconvctx_t *pcmconv_ctx,
 const st_pcmfmt_t *input_fmt, const st_pcmfmt_t *output_fmt) {
    SDL_AudioSpec input_spec;
    SDL_AudioSpec output_spec;
    st_pcmconv_t *converter;

    if (!is_pcmfmt_valid(input_fmt) || !is_pcmfmt_valid(output_fmt)) {
        ST_LOGGERCTX_CALL(pcmconv_ctx->logger_ctx, error,
         "%s_%s: Input or output PCM-format is not valid", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    if (output_fmt->channels_layout == ST_SND_CL_PLANAR) {
        ST_LOGGERCTX_CALL(pcmconv_ctx->logger_ctx, error,
         "%s_%s: planar channel layout for PCM-converter output is not "
         "supported", st_module_subsystem, st_module_name);

        return NULL;
    }

    converter = (st_pcmconv_t *)st_object_new(sizeof(st_pcmconv_t),
     &pcmconv_funcs, (st_object_dtor_t)st_pcmconv_destroy_converter,
     (st_object_t *)pcmconv_ctx);

    if (!converter) {
        ST_LOGGERCTX_CALL(pcmconv_ctx->logger_ctx, error,
         "%s_%s: Unable to create PCM-converter object", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    pcmfmt_to_sdl3audiospec(&input_spec, input_fmt);
    pcmfmt_to_sdl3audiospec(&output_spec, output_fmt);

    converter->stream = pcmconv_ctx->sdl3.create_audio_stream(&input_spec,
     &output_spec);
    if (!converter->stream) {
        ST_LOGGERCTX_CALL(pcmconv_ctx->logger_ctx, error,
         "%s_%s: Unable to create SDL3 Audio Stream for PCM-converter: "
         "%s", st_module_subsystem, st_module_name,
         pcmconv_ctx->sdl3.get_error());

        goto stream_create_fail;
    }

    converter->input_fmt = *input_fmt;
    converter->output_fmt = *output_fmt;

    return converter;

stream_create_fail:
    free(converter);

    return NULL;
}

static void st_pcmconv_destroy_converter(st_pcmconv_t *converter) {
    st_pcmconvctx_t *pcmconv_ctx = (st_pcmconvctx_t *)ST_OBJECT_CALL(converter,
     get_owner);

    pcmconv_ctx->sdl3.destroy_audio_stream(converter->stream);
    free(converter);
}

static size_t pcm_size_to_samples_count(size_t pcm_size,
 st_sndsamplefmt_t sample_fmt) {
    switch (sample_fmt) {
        case ST_SND_SF_U8:
        case ST_SND_SF_S8:
            return pcm_size;
        case ST_SND_SF_S16LE:
        case ST_SND_SF_S16BE:
            return pcm_size % 2 == 0
                ? pcm_size / 2
                : 0;
        case ST_SND_SF_S32LE:
        case ST_SND_SF_S32BE:
        case ST_SND_SF_F32LE:
        case ST_SND_SF_F32BE:
            return pcm_size % 4 == 0
                ? pcm_size / 4
                : 0;
        case ST_SND_SF_UNKNOWN :
        default:
            return 0;
    }
}

static bool st_pcmconv_put_pcm(st_pcmconv_t *converter, const void **pcm,
 size_t pcm_size_per_channel) {
    st_pcmconvctx_t *pcmconv_ctx = (st_pcmconvctx_t *)ST_OBJECT_CALL(converter,
     get_owner);

    if (converter->input_fmt.channels_layout == ST_SND_CL_PLANAR) {
        size_t samples_count = pcm_size_to_samples_count(pcm_size_per_channel,
         converter->input_fmt.sample_fmt);

        if (samples_count == 0 || samples_count > INT_MAX)
            return false;

        return pcmconv_ctx->sdl3.put_audio_stream_planar_data(converter->stream,
         pcm, converter->input_fmt.channels, samples_count);
    } else if (converter->input_fmt.channels_layout == ST_SND_CL_INTERLEAVED) {
        return pcmconv_ctx->sdl3.put_audio_stream_data(converter->stream,
         pcm[0], pcm_size_per_channel * converter->input_fmt.channels);
    } else {
        return false;
    }
}

static ssize_t st_pcmconv_get_available_pcm_size_per_channel(
 st_pcmconv_t *converter) {
    st_pcmconvctx_t *pcmconv_ctx = (st_pcmconvctx_t *)ST_OBJECT_CALL(converter,
     get_owner);
    int              available = pcmconv_ctx->sdl3.get_audio_stream_available(
     converter->stream);

    return available == -1
        ? -1
        : available / converter->output_fmt.channels;
}

static ssize_t st_pcmconv_get_pcm(st_pcmconv_t *converter, void **dst,
 size_t dst_size_per_channel) {
    st_pcmconvctx_t *pcmconv_ctx = (st_pcmconvctx_t *)ST_OBJECT_CALL(converter,
     get_owner);
    int              get_data_size;

    if (dst_size_per_channel > INT_MAX / converter->output_fmt.channels)
        return -1;

    get_data_size = pcmconv_ctx->sdl3.get_audio_stream_data(converter->stream,
     dst[0], dst_size_per_channel * converter->output_fmt.channels);

    return get_data_size == -1
        ? -1
        : get_data_size / converter->output_fmt.channels;
}

static bool st_pcmconv_flush(st_pcmconv_t *converter) {
    st_pcmconvctx_t *pcmconv_ctx = (st_pcmconvctx_t *)ST_OBJECT_CALL(converter,
     get_owner);

    return pcmconv_ctx->sdl3.flush_audio_stream(converter->stream);
}
