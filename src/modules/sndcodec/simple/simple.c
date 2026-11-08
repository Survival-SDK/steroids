#include "simple.h"

#if !defined(__BYTE_ORDER__) || __BYTE_ORDER__ != __LITTLE_ENDIAN
    #error "Unknown or unsupported byte order"
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define WAV_AUDIO_FORMAT_INTEGER 1
#define WAV_AUDIO_FORMAT_FLOAT   3

static st_sndcodecctx_t *st_sndcodec_init(const st_param_t params[]);
static void st_sndcodec_quit(st_sndcodecctx_t *sndcodec_ctx);

static st_sound_t *st_sndcodec_load(st_sndcodecctx_t *sndcodec_ctx,
 const char *filename, st_soundtype_t type);

static size_t st_sndcodec_stream_read_pcm(st_sndstream_t *sndstream,
 void *buf, size_t len);
static void st_sndcodec_stream_close(st_sndstream_t *sndstream);

static st_sndcodecctx_funcs_t sndcodecctx_funcs = {
    ST_MODCTX_FUNCS,
    .load = st_sndcodec_load,
};

static st_sndstream_funcs_t sndstream_funcs = {
    ST_OBJECT_FUNCS,
    .read_pcm = st_sndcodec_stream_read_pcm,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "sound", NULL, },
    {0},
};

st_moddata_t *st_module_sndcodec_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("sndcodec", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_sndcodec_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_sndcodec_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "sndcodec";
static const char *st_module_name = "simple";

static st_sndcodecctx_t *st_sndcodec_init(const st_param_t params[]) {
    st_modsmgr_t     *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t   *logger_ctx;
    st_soundctx_t    *sound_ctx;
    st_sndcodecctx_t *sndcodec_ctx;

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

    sndcodec_ctx = (st_sndcodecctx_t *)st_modctx_new(
     "sndcodec", "simple", sizeof(st_sndcodecctx_t), NULL, &sndcodecctx_funcs,
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

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Simple WAV audio codec initialized", st_module_subsystem,
     st_module_name);

    return sndcodec_ctx;
}

static void st_sndcodec_quit(st_sndcodecctx_t *sndcodec_ctx) {
    ST_LOGGERCTX_CALL(sndcodec_ctx->logger_ctx, info,
     "%s_%s: Simple WAV audio codec destroyed", st_module_subsystem,
     st_module_name);
    free(sndcodec_ctx);
}

static st_sound_t *st_sndcodec_load_preloaded(
 __attribute__((unused)) st_sndcodecctx_t *sndcodec_ctx,
 __attribute__((unused)) const char *filename) {
     return NULL; /* not implemented */
}

static st_sndsamplefmt_t wav_to_samplefmt(bool float_pcm, int bits_per_sample) {
    if (float_pcm)
        return ST_SND_SF_F32LE;

    switch (bits_per_sample) {
        case 8:
            return ST_SND_SF_U8; /* WAV 8-bit = unsigned */
        case 16:
            return ST_SND_SF_S16LE;
        case 32:
            return ST_SND_SF_S32LE;
        default:
            return ST_SND_SF_UNKNOWN; /* 24-bit etc */
    }
}

static bool st_sndcodec_stream_open(st_sndstream_t *sndstream,
 st_pcmfmt_t *src_fmt, const char *path) {
    uint16_t block_align = 0;
    char     header_id[4];
    bool     have_fmt = false;
    bool     have_data = false;

    // SDL_memset(sndstream, 0, sizeof(st_sndstream_t));

    sndstream->fstream = fopen(path, "rb");
    if (!sndstream->fstream)
        return false;

    if (fread(header_id, 1, 4, sndstream->fstream) != 4
     || strncmp(header_id, "RIFF", 4) != 0)
        goto fail; /* Not a RIFF file */

    if (fseek(sndstream->fstream, 4, SEEK_CUR) != 0)
        goto fail;

    if (fread(header_id, 1, 4, sndstream->fstream) != 4
     || strncmp(header_id, "WAVE", 4) != 0)
        goto fail; /* Not a WAVE file */

    while (!have_data) {
        char     chunk_id[4];
        uint32_t chunk_size;

        if (fread(chunk_id, 1, 4, sndstream->fstream) != 4)
            goto fail; /* Unexpected end of file */

        if (fread(&chunk_size, 4, 1, sndstream->fstream) != 1)
            goto fail;

        if (strncmp(chunk_id, "fmt ", 4) == 0) {
            uint16_t format_tag = 0;
            uint16_t channels_count;
            uint32_t rate;
            uint16_t bps;

            if (chunk_size < 16)
                goto fail; /* fmt chunk too short */

            if (fread(&format_tag, 2, 1, sndstream->fstream) != 1
             || (format_tag != WAV_AUDIO_FORMAT_INTEGER
              && format_tag != WAV_AUDIO_FORMAT_FLOAT))
                goto fail; /* Unsupported WAV format tag */

            if (fread(&channels_count, 2, 1, sndstream->fstream) != 1)
                goto fail;

            if (fread(&rate, 4, 1, sndstream->fstream) != 1)
                goto fail;

            if (fseek(sndstream->fstream, 4, SEEK_CUR) != 0) /* byte_rate */
                goto fail;

            if (fread(&block_align, 2, 1, sndstream->fstream) != 1
             || block_align == 0)
                goto fail; /* Zero block align valu */

            if (fread(&bps, 2, 1, sndstream->fstream) != 1)
                goto fail;

            src_fmt->sample_fmt = wav_to_samplefmt(
             format_tag == WAV_AUDIO_FORMAT_FLOAT, bps);
            if (src_fmt->sample_fmt == ST_SND_SF_UNKNOWN)
                goto fail; /* Unsupported bits per sample */

            src_fmt->channels = channels_count;
            src_fmt->rate = rate;
            src_fmt->channels_layout = ST_SND_CL_INTERLEAVED;

            /* доп. байты fmt-чанка (нестандартные расширения) */
            if (chunk_size > 16
             && fseek(sndstream->fstream, chunk_size - 16 + (chunk_size & 1),
              SEEK_CUR) != 0)
                goto fail;

            have_fmt = true;
        } else if (strncmp(chunk_id, "data", 4) == 0) {
            /* ---- data-чанк ---- */
            if (!have_fmt)
                goto fail; /* data chunk before fmt chunk */

            sndstream->data_start = ftell(sndstream->fstream);
            sndstream->data_size  = chunk_size;
            have_data = true;

            if (sndstream->data_size % block_align != 0)
                goto fail; /* Incorrect data size */

            /* данные не читаем — только запоминаем позицию */
        } else {
            /* неизвестный чанк — пропускаем */
            if (fseek(sndstream->fstream, chunk_size + (chunk_size & 1),
             SEEK_CUR) != 0)
                goto fail;
        }
    }
    /* Устанавливаем позицию на начало PCM-данных */
    if (fseek(sndstream->fstream, sndstream->data_start, SEEK_SET) != 0)
        goto fail;

    return true;

fail:
    fclose(sndstream->fstream);
    sndstream->fstream = NULL;

    return false;
}

static void st_sndcodec_stream_close(st_sndstream_t *sndstream) {
    if (sndstream->fstream)
        fclose(sndstream->fstream);

    free(sndstream);
}

static size_t st_sndcodec_stream_read_pcm(st_sndstream_t *sndstream, void *buf,
 size_t len) {
     ssize_t remaining;

     if (!sndstream->fstream)
        return 0;

    remaining = (ssize_t)sndstream->data_size
     - (ftell(sndstream->fstream) - sndstream->data_start);
    if (remaining <= 0)
        return 0;

    if (len > remaining)
        len = remaining;

    return fread(buf, 1, len, sndstream->fstream);
}

static st_sound_t *st_sndcodec_load_streamed(st_sndcodecctx_t *sndcodec_ctx,
 const char *filename) {
    st_sound_t      *result;
    st_pcmfmt_t      src_fmt;
    st_sndstream_t  *sndstream = (st_sndstream_t *)st_object_new(
     sizeof(st_sndstream_t), &sndstream_funcs,
     (st_object_dtor_t)st_sndcodec_stream_close, (st_object_t *)sndcodec_ctx);

    if (!sndstream)
        return NULL;

    if (!st_sndcodec_stream_open(sndstream, &src_fmt, filename))
         goto stream_open_fail;

    result = ST_SOUNDCTX_CALL(sndcodec_ctx->sound_ctx, import_streamed,
     sndstream, &src_fmt, sndcodec_ctx, filename);
    if (!result)
        goto import_streamed_fail;

    return result;

import_streamed_fail:
stream_open_fail:
    st_sndcodec_stream_close(sndstream);

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
