#pragma once

typedef enum {
    ST_SND_ST_UNKNOWN,
    ST_SND_ST_PRELOAD,
    ST_SND_ST_STREAM,
} st_soundtype_t;

typedef enum {
    ST_SND_CL_UNKNOWN = 0,
    ST_SND_CL_PLANAR,
    ST_SND_CL_INTERLEAVED,
} st_sndchnllayout_t;

typedef enum {
    ST_SND_SF_UNKNOWN = 0,
    ST_SND_SF_U8,
    ST_SND_SF_S8,
    ST_SND_SF_S16LE,
    ST_SND_SF_S16BE,
    ST_SND_SF_S32LE,
    ST_SND_SF_S32BE,
    ST_SND_SF_F32LE,
    ST_SND_SF_F32BE,
} st_sndsamplefmt_t;
