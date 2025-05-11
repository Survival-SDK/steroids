#pragma once

#include <stddef.h>
#include <stdint.h>

#include "steroids/module.h"
#include "steroids/types/modctx.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/object.h"

#ifndef ST_FNV1ACTX_T_DEFINED
    typedef struct st_fnv1actx_s st_fnv1actx_t;
#endif

typedef uint32_t (*st_u32hashstrfunc_t)(const char *str);
typedef uint32_t (*st_u32hashbytesfunc_t)(const void *ptr, size_t size);

typedef st_fnv1actx_t *(*st_fnv1a_init_t)(struct st_loggerctx_s *logger_ctx);
typedef void (*st_fnv1a_quit_t)(st_fnv1actx_t *fnv1a_ctx);
typedef uint32_t (*st_fnv1a_u32hash_string_t)(st_fnv1actx_t *fnv1a_ctx,
 const char *str);
typedef uint32_t (*st_fnv1a_u32hash_bytes_t)(st_fnv1actx_t *fnv1a_ctx,
 const void *ptr, size_t size);
typedef st_u32hashstrfunc_t (*st_fnv1a_get_u32hashstr_func_t)(
 st_fnv1actx_t *fnv1a_ctx);
typedef st_u32hashbytesfunc_t (*st_fnv1a_get_u32hashbytes_func_t)(
 st_fnv1actx_t *fnv1a_ctx);

typedef struct {
    st_modctx_funcs_t;
    st_fnv1a_u32hash_string_t        u32hash_string;
    st_fnv1a_u32hash_bytes_t         u32hash_bytes;
    st_fnv1a_get_u32hashstr_func_t   get_u32hashstr_func;
    st_fnv1a_get_u32hashbytes_func_t get_u32hashbytes_func;
} st_fnv1actx_funcs_t;

#define ST_FNV1ACTX_CALL(ctx, func, ...) \
    ((st_fnv1actx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, ## __VA_ARGS__)
