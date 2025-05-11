#pragma once

#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    struct st_loggerctx_s *logger_ctx;
} st_mpoolctx_t;

typedef struct st_freelist_s {
    struct st_freelist_s *next;
} st_freelist_t;

typedef struct {
    st_object_t;
    size_t         chunk_size;
    size_t         block_size;
    void         **blocks;
    size_t         blocks_count;
    size_t         blocks_max;
    st_freelist_t *head;
} st_mpool_t;

#define ST_MPOOLCTX_T_DEFINED
#define ST_MPOOL_T_DEFINED
