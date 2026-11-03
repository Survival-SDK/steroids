#pragma once

#include "steroids/consts/sound.h"

#include <stdbool.h>

typedef struct {
    unsigned           rate;
    unsigned           channels;
    st_sndchnllayout_t channels_layout;
    st_sndsamplefmt_t  sample_fmt;
} st_pcmfmt_t;

static inline bool is_pcmfmt_valid(const st_pcmfmt_t* fmt) {
    return fmt
        && fmt->rate > 0
        && fmt->channels > 0
        && fmt->channels_layout != ST_SND_CL_UNKNOWN
        && fmt->sample_fmt != ST_SND_SF_UNKNOWN;
}
