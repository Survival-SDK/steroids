#include "simple.h"

#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/sndcodec.h"
#include "types.h"

#define CODEC_NAME_SIZE                32
#define DEFAULT_MIXFMT_RATE            48000
#define DEFAULT_MIXFMT_CHANNELS_COUNT  2
#define DEFAULT_MIXFMT_CHANNELS_LAYOUT ST_SND_CL_INTERLEAVED
#define DEFAULT_MIXFMT_SAMPLE_FMT      ST_SND_SF_F32LE

static st_soundctx_t *st_sound_init(const st_param_t params[]);
static void st_sound_quit(st_soundctx_t *sound_ctx);

static st_sound_t *st_sound_load(st_soundctx_t *sound_ctx,
 const char *filename, st_soundtype_t type);
static st_sound_t *st_sound_import_preloaded(st_soundctx_t *sound_ctx,
 const st_pcmfmt_t *src_fmt, const void *pcm[], size_t pcm_len_per_channel);
static st_sound_t *st_sound_import_streamed(st_soundctx_t *sound_ctx,
 struct st_sndstream *sndstream, const st_pcmfmt_t *src_fmt,
 struct st_sndcodecctx *codec_ctx, const char *asset_id);
static const st_pcmfmt_t *st_sound_get_mix_fmt(const st_soundctx_t *sound_ctx);

static st_soundtype_t st_sound_get_type(const st_sound_t *sound);

static size_t st_sound_get_pcm_size_per_channel(const st_prldsound_t *sound);

static const st_pcmfmt_t *st_sound_get_src_fmt(const st_strmsound_t *sound);

static st_soundctx_funcs_t soundctx_funcs = {
    ST_MODCTX_FUNCS,
    .load             = st_sound_load,
    .import_preloaded = st_sound_import_preloaded,
    .import_streamed  = st_sound_import_streamed,
    .get_mix_fmt      = st_sound_get_mix_fmt,
};

#define ST_SOUND_FUNCS             \
    ST_OBJECT_FUNCS,               \
    .get_type = st_sound_get_type

static st_sound_funcs_t sound_funcs = {
    ST_SOUND_FUNCS,
};

static st_prldsound_funcs_t prldsound_funcs = {
    ST_SOUND_FUNCS,
    .get_pcm_size_per_channel = st_sound_get_pcm_size_per_channel,
};

static st_strmsound_funcs_t strmsound_funcs = {
    ST_SOUND_FUNCS,
    .get_src_fmt = st_sound_get_src_fmt,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "pcmconv", NULL, },
    {0},
};

st_moddata_t *st_module_sound_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("sound", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_sound_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_sound_simple_init(modsmgr);
}
#endif

static void st_sound_init_codecs(st_soundctx_t *sound_ctx,
 st_modsmgr_t *modsmgr) {
    char  codecs_names[CODEC_NAME_SIZE][SND_CODECS_MAX] = {0};
    char *pcodecsnames[SND_CODECS_MAX];

    for (size_t i = 0; i < SND_CODECS_MAX; i++)
        pcodecsnames[i] = codecs_names[i];

    memset(sound_ctx->codecs, 0, sizeof(sound_ctx->codecs));
    sound_ctx->codecs_count = 0;

    ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, info,
     "sound_simple: Searching sound codecs");

    ST_MODSMGR_CALL(modsmgr, get_module_names, pcodecsnames,
     SND_CODECS_MAX, CODEC_NAME_SIZE, "sndcodec");

    for (size_t i = 0; i < SND_CODECS_MAX; i++) {
        st_ctx_ctor_t ctx_ctor;
        st_modctx_t  *ctx;
        char         *codec_name = pcodecsnames[i];

        if (!*codec_name)
            break;

        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, info,
         "sound_simple: Found module \"sndcodec_%s\"", codec_name);

        ctx_ctor = ST_MODSMGR_CALL(modsmgr, get_ctor,
         "sndcodec", codec_name);

        if (!ctx_ctor) {
            ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
             "sound_simple: Unable to get ctor from module \"sndcodec_%s\"",
             codec_name);

            continue;
        }

        ctx = ctx_ctor((st_params_t){
            {"modsmgr", (uintptr_t)modsmgr},
            {"sound_ctx", (uintptr_t)sound_ctx},
        {0},});
        if (!ctx)
            continue;

        sound_ctx->codecs[sound_ctx->codecs_count++] = ctx;
    }
}

static st_soundctx_t *st_sound_init(const st_param_t params[]) {
    st_modsmgr_t      *modsmgr;
    st_loggerctx_t    *logger_ctx;
    st_pcmconvctx_t   *pcmconv_ctx;
    st_soundctx_t     *sound_ctx;
    unsigned           mixfmt_rate;
    unsigned           mixfmt_channels_count;
    st_sndchnllayout_t mixfmt_channels_layout;
    st_sndsamplefmt_t  mixfmt_sample_fmt;

    modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    if (!modsmgr)
        return NULL;

    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    pcmconv_ctx = (st_pcmconvctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "pcmconv", NULL);
    if (!pcmconv_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "sound_simple: Unable to get initialized pcmconv context");

        return NULL;
    }

    sound_ctx = (st_soundctx_t *)st_modctx_new("sound", "simple",
     sizeof(st_soundctx_t), NULL, &soundctx_funcs,
     (st_object_dtor_t)st_sound_quit);
    if (!sound_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "sound_simple: Unable to create new sound ctx object");

        return NULL;
    }

    mixfmt_rate = st_modctx_get_param_as_uint(params, "rate");
    mixfmt_channels_count = st_modctx_get_param_as_uint(params,
     "channels_count");
    mixfmt_channels_layout = st_modctx_get_param_as_int(params,
     "channels_layout");
    mixfmt_sample_fmt = st_modctx_get_param_as_int(params, "sample_fmt");

    sound_ctx->logger_ctx = logger_ctx;
    sound_ctx->pcmconv_ctx = pcmconv_ctx;
    sound_ctx->mix_fmt    = (st_pcmfmt_t){
        .rate = mixfmt_rate ?: DEFAULT_MIXFMT_RATE,
        .channels = mixfmt_channels_count ?: DEFAULT_MIXFMT_CHANNELS_COUNT,
        .channels_layout = mixfmt_channels_layout
            ?: DEFAULT_MIXFMT_CHANNELS_LAYOUT,
        .sample_fmt = mixfmt_sample_fmt ?: DEFAULT_MIXFMT_SAMPLE_FMT,
    };

    st_sound_init_codecs(sound_ctx, modsmgr);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "sound_simple: Sounds mgr initialized.");

    return sound_ctx;
}

static void st_sound_quit(st_soundctx_t *sound_ctx) {
    for (size_t i = 0; i < sound_ctx->codecs_count; i++)
        ST_SNDCODECCTX_CALL(sound_ctx->codecs[i], destroy);

    ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, info,
     "sound_simple: Sounds mgr destroyed");
    free(sound_ctx);
}

static st_sound_t *st_sound_load(st_soundctx_t *sound_ctx,
 const char *filename, st_soundtype_t type) {
    for (size_t i = 0; i < sound_ctx->codecs_count; i++) {
        st_modctx_t *codec = sound_ctx->codecs[i];
        st_sound_t  *sound = ST_SNDCODECCTX_CALL(codec, load, filename, type);

        if (sound)
            return sound;
    }

    ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
     "sound_simple: No suitable codec for loading sound \"%s\"", filename);

    return NULL;
}

static void st_sound_destroy_preloaded(void *psound) {
    st_prldsound_t *sound = psound;
    st_soundctx_t  *sound_ctx = (st_soundctx_t *)ST_PRLDSOUND_CALL(sound,
     get_owner);

    if (sound_ctx->mix_fmt.channels_layout == ST_SND_CL_PLANAR) {
        for (size_t i = 0; i < sound_ctx->mix_fmt.channels; i++) {
            if (sound->pcm[i])
                free(sound->pcm[i]);
        }
    }
    free(sound->pcm);
    free(sound);
}

static void st_sound_destroy_streamed(void *psound) {
    st_strmsound_t *sound = psound;

    ST_SNDSTREAM_CALL(sound->handle, destroy);
    free(sound->asset_id);
    free(sound);
}

static void **convert_pcm_to_mix_fmt(st_soundctx_t *sound_ctx,
 size_t *dst_pcm_size_per_channel, const st_pcmfmt_t *src_fmt, const void *pcm[],
 size_t src_pcm_size_per_channel) {
    void        **dst_pcm;
    ssize_t       converted_size_pre_channel;
    st_pcmconv_t *converter = ST_PCMCONVCTX_CALL(sound_ctx->pcmconv_ctx,
     create_converter, src_fmt, &sound_ctx->mix_fmt);

    if (!converter) {
        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
         "sound_simple: Unable to create PCM-converter");

        return NULL;
    }

    if (!ST_PCMCONV_CALL(converter, put_pcm, pcm, src_pcm_size_per_channel)) {
        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
         "sound_simple: Unable to put src PCM data to PCM-converter");

        goto put_pcm_fail;
    }

    if (!ST_PCMCONV_CALL(converter, flush)) {
        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
         "sound_simple: Unable to flush converted PCM data to converter output"
        );

        goto flush_fail;
    }

    *dst_pcm_size_per_channel = ST_PCMCONV_CALL(converter,
     get_available_pcm_size_per_channel);
    if (*dst_pcm_size_per_channel == -1) {
        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
         "sound_simple: Unable to get size of converted PCM data");

        goto get_size_fail;
    }

    if (sound_ctx->mix_fmt.channels_layout == ST_SND_CL_INTERLEAVED) {
        if (*dst_pcm_size_per_channel > SIZE_MAX / sound_ctx->mix_fmt.channels)
        {
            ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
             "sound_simple: Expected so huge PCM data size after converting");

            goto huge_size;
        }

        dst_pcm = malloc(sizeof(void *));
        if (!dst_pcm) {
            ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
             "sound_simple: Unable to allocate memory for pcm data pointer");

            goto pcm_array_malloc_fail;
        }

        dst_pcm[0] = malloc(
         *dst_pcm_size_per_channel * sound_ctx->mix_fmt.channels);
        if (!dst_pcm[0]) {
            ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
             "sound_simple: Unable to allocate memory for loaded pcm data");

            goto pcm_malloc_fail;
        }
    } else {
        dst_pcm = malloc(sizeof(void *) * sound_ctx->mix_fmt.channels);
        if (!dst_pcm) {
            ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
             "sound_simple: Unable to allocate memory for loaded pcm data "
             "array");

            goto pcm_array_malloc_fail;
        }

        memset(dst_pcm, 0, sizeof(void *) * sound_ctx->mix_fmt.channels);

        for (size_t i = 0; i < sound_ctx->mix_fmt.channels; i++) {
            dst_pcm[i] = malloc(*dst_pcm_size_per_channel);
            if (!dst_pcm[i]) {
                ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
                 "sound_simple: Unable to allocate memory for loaded pcm "
                 "data");

                goto pcm_malloc_fail;
            }
        }
    }

    converted_size_pre_channel = ST_PCMCONV_CALL(converter, get_pcm, dst_pcm,
     *dst_pcm_size_per_channel);
    if (converted_size_pre_channel == -1) {
        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
         "sound_simple: Unable to get converted PCM data");

        goto get_pcm_fail;
    }

    *dst_pcm_size_per_channel = converted_size_pre_channel;

    ST_PCMCONV_CALL(converter, destroy);

    return dst_pcm;

get_pcm_fail:
pcm_malloc_fail:
    if (sound_ctx->mix_fmt.channels_layout == ST_SND_CL_PLANAR) {
        for (size_t i = 0; i < sound_ctx->mix_fmt.channels; i++) {
            if (dst_pcm[i])
                free(dst_pcm[i]);
        }
    } else {
        if (dst_pcm[0])
            free(dst_pcm[0]);
    }
    free(dst_pcm);

pcm_array_malloc_fail:
huge_size:
get_size_fail:
flush_fail:
put_pcm_fail:
    ST_PCMCONV_CALL(converter, destroy);

    return NULL;
}

static st_sound_t *st_sound_import_preloaded(st_soundctx_t *sound_ctx,
 const st_pcmfmt_t *src_fmt, const void *pcm[], size_t pcm_size_per_channel) {
    st_prldsound_t *sound;

    if (!is_pcmfmt_valid(src_fmt)) {
        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
         "sound_simple: Incorrect PCM format");

        return NULL;
    }

    sound = (st_prldsound_t *)st_object_new(
     sizeof(st_prldsound_t), &prldsound_funcs, st_sound_destroy_preloaded,
     (st_object_t *)sound_ctx);

    if (!sound) {
        ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
         "sound_simple: Unable to create object for preloaded sound");

        return NULL;
    }

    sound->pcm = convert_pcm_to_mix_fmt(sound_ctx, &sound->pcm_size_per_channel,
     src_fmt, pcm, pcm_size_per_channel);
    if (!sound->pcm)
        goto convert_fail;

    sound->type = ST_SND_ST_PRELOAD;

    return (st_sound_t *)sound;

convert_fail:
    free(sound);

    return NULL;
}

static st_sound_t *st_sound_import_streamed(st_soundctx_t *sound_ctx,
 struct st_sndstream *sndstream, const st_pcmfmt_t *src_fmt,
 struct st_sndcodecctx *codec_ctx, const char *asset_id) {
     st_strmsound_t *sound;

     if (!src_fmt) {
         ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
          "sound_simple: Expected non-NULL src pcm fmt");

         return NULL;
     }

     if (!asset_id) {
         ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
          "sound_simple: Expected non-NULL asset id");

         return NULL;
     }

     sound = (st_strmsound_t *)st_object_new(sizeof(st_strmsound_t),
      &strmsound_funcs, st_sound_destroy_streamed, (st_object_t *)sound_ctx);

     if (!sound) {
         ST_LOGGERCTX_CALL(sound_ctx->logger_ctx, error,
          "sound_simple: Unable to create object for streamed sound");

         return NULL;
     }

     sound->asset_id = strdup(asset_id);
     if (!sound->asset_id) {
         free(sound);

         return NULL;
     }

     sound->type = ST_SND_ST_STREAM;
     memcpy(&sound->src_fmt, src_fmt, sizeof(st_pcmfmt_t));
     sound->codec_ctx = (st_modctx_t *)codec_ctx;
     sound->handle = sndstream;

     return (st_sound_t *)sound;
}

static const st_pcmfmt_t *st_sound_get_mix_fmt(
 const st_soundctx_t *sound_ctx) {
    return sound_ctx ? &sound_ctx->mix_fmt : NULL;
}

static st_soundtype_t st_sound_get_type(const st_sound_t *sound) {
    return sound ? sound->type : ST_SND_ST_UNKNOWN;
}

static size_t st_sound_get_pcm_size_per_channel(const st_prldsound_t *sound) {
    return sound ? sound->pcm_size_per_channel : 0;
}

static const st_pcmfmt_t *st_sound_get_src_fmt(const st_strmsound_t *sound) {
    return sound ? &sound->src_fmt : NULL;
}
