#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#define ST_OPTS_SHORT_UNSPEC       '\0'
#define ST_OPTS_LONG_INDEX_UNSPEC  -1

#ifndef ST_OPTSCTX_T_DEFINED
    typedef struct st_optsctx_s st_optsctx_t;
#endif

typedef enum {
    ST_OA_NO       = 0,
    ST_OA_REQUIRED = 1,
    ST_OA_OPTIONAL = 2,
} st_opt_arg_t;

typedef bool (*st_opts_add_option_t)(st_optsctx_t *opts_ctx, char short_option,
 const char *long_option, st_opt_arg_t arg, const char *arg_fmt,
 const char *option_descr);
typedef bool (*st_opts_get_str_t)(st_optsctx_t *opts_ctx, const char *opt,
 char *dst, size_t dstsize);
typedef bool (*st_opts_get_help_t)(st_optsctx_t *opts_ctx, char *dst,
 size_t dstsize, size_t columns);

typedef struct {
    st_modctx_funcs_t;
    st_opts_add_option_t add_option;
    st_opts_get_str_t    get_str;
    st_opts_get_help_t   get_help;
} st_optsctx_funcs_t;

#define ST_OPTSCTX_CALL(object, func, ...) \
    ((st_optsctx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
