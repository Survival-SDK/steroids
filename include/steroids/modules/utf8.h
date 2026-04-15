#pragma once

#include <sys/types.h>
#include <stdint.h>

#include "steroids/modctx.h"

#ifndef ST_UTF8CTX_T_DEFINED
    typedef st_modctx_t st_utf8ctx_t;
#endif

typedef ssize_t (*st_utf8_str_codepoints_t)(const st_utf8ctx_t *utf8_ctx,
 const char *str, size_t codepoints_max);
typedef const char *(*st_utf8_str_advance_t)(const st_utf8ctx_t *utf8_ctx,
 const char *str, size_t codepoints_count);
typedef int64_t (*st_utf8_to_codepoint_t)(const st_utf8ctx_t *utf8_ctx,
 const char *utf8char);
typedef ssize_t (*st_utf8_str_to_codepoints_t)(const st_utf8ctx_t *utf8_ctx,
 const char *str, uint32_t *dst, size_t codepoints_max);

typedef struct {
    st_modctx_funcs_t;
    st_utf8_str_codepoints_t    str_codepoints;
    st_utf8_str_advance_t       str_advance;
    st_utf8_to_codepoint_t      to_codepoint;
    st_utf8_str_to_codepoints_t str_to_codepoints;
} st_utf8ctx_funcs_t;

#define ST_UTF8CTX_CALL(ctx, func, ...) \
    ((st_utf8ctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
