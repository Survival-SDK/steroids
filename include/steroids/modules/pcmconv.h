#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "steroids/modctx.h"
#include "steroids/object.h"
#include "steroids/types/pcmfmt.h"

#ifndef ST_PCMCONVCTX_T_DEFINED
    typedef st_modctx_t st_pcmconvctx_t;
#endif
#ifndef ST_PCMCONV_T_DEFINED
    typedef st_object_t st_pcmconv_t;
#endif

typedef st_pcmconv_t *(*st_pcmconv_create_converter_t)(
 st_pcmconvctx_t *pcmconv_ctx, const st_pcmfmt_t *input_fmt,
 const st_pcmfmt_t *output_fmt);

typedef bool (*st_pcmconv_put_pcm_t)(st_pcmconv_t *converter, const void **pcm,
 size_t pcm_size_per_channel);
typedef ssize_t (*st_pcmconv_get_available_pcm_size_per_channel_t)(
 st_pcmconv_t *converter);
typedef ssize_t (*st_pcmconv_get_pcm_t)(st_pcmconv_t *converter, void **dst,
 size_t dst_size_per_channel);
typedef bool (*st_pcmconv_flush_t)(st_pcmconv_t *converter);

typedef struct {
    st_modctx_funcs_t;
    st_pcmconv_create_converter_t create_converter;
} st_pcmconvctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_pcmconv_put_pcm_t                            put_pcm;
    st_pcmconv_get_available_pcm_size_per_channel_t
     get_available_pcm_size_per_channel;
    st_pcmconv_get_pcm_t                            get_pcm;
    st_pcmconv_flush_t                              flush;
} st_pcmconv_funcs_t;

#define ST_PCMCONVCTX_CALL(ctx, func, ...) \
    ((st_pcmconvctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_PCMCONV_CALL(object, func, ...) \
    ((st_pcmconv_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
