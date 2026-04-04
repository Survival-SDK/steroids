#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_MPOOLCTX_T_DEFINED
    typedef struct st_mpoolctx_s st_mpoolctx_t;
#endif
#ifndef ST_MPOOL_T_DEFINED
    typedef struct st_mpool_s st_mpool_t;
#endif

typedef st_mpool_t *(*st_mpool_create_t)(st_mpoolctx_t *mpool_ctx,
 size_t chunk_size, size_t chunks_count, size_t max_blocks);
typedef void *(*st_mpool_alloc_t)(st_mpool_t *mpool);
typedef void (*st_mpool_free_t)(st_mpool_t *mpool, void *ptr);

typedef struct {
    st_modctx_funcs_t;
    st_mpool_create_t create;
} st_mpoolctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_mpool_alloc_t alloc;
    st_mpool_free_t  free;
} st_mpool_funcs_t;

#define ST_MPOOLCTX_CALL(ctx, func, ...) \
    ((st_mpoolctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_MPOOL_CALL(object, func, ...) \
    ((st_mpool_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
