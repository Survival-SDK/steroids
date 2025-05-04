#include "simple.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#define ERRMSGBUF_SIZE 128
#define DEFAULT_CHUNKS_COUNT 32

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static void st_mpool_quit(st_mpoolctx_t *mpool_ctx);
static st_mpool_t *st_mpool_create(st_mpoolctx_t *mpool_ctx, size_t chunk_size, 
 size_t chunks_count, size_t max_blocks);
static void st_mpool_destroy(st_mpool_t *mpool);
static void *st_mpool_alloc(st_mpool_t *mpool);
static void st_mpool_free(st_mpool_t *mpool, void *ptr);

static st_mpoolctx_funcs_t mpoolctx_funcs = {
    st_modctx_funcs,
    .create = st_mpool_create,
};

static st_mpool_funcs_t mpool_funcs = {
    st_object_funcs,
    .alloc = st_mpool_alloc,
    .free  = st_mpool_free,
};

static st_moddata_t st_module_mpool_simple_data = {
    .name = "simple",
    .type = ST_MODULE_TYPE,
    .subsystem = "mpool",
    .prereqs = (st_modprerq_t[]){ 
        { "logger", NULL, },
        {0}, 
    },
    .ctor = st_mpool_init,
};

ST_MODULE_DEF_INIT_FUNC(mpool_simple)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_mpool_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static st_mpoolctx_t *st_mpool_init(struct st_loggerctx_s *logger_ctx) {
    st_mpoolctx_t *mpool_ctx = (st_mpoolctx_t *)st_modctx_new("mpool", "simple",
     sizeof(st_mpoolctx_t), NULL, &mpoolctx_funcs, 
     (st_object_dtor_t)st_mpool_quit);

    if (!mpool_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "mpool_simple: unable to create new mpool ctx object");

        return NULL;
    }

    mpool_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info, "mpool_simple: Module initialized");

    return mpool_ctx;
}

static void st_mpool_quit(st_mpoolctx_t *mpool_ctx) {
    ST_LOGGERCTX_CALL(mpool_ctx->logger_ctx, info,
     "mpool_simple: Module destroyed");
    free(mpool_ctx);
}

static bool mpool_grow(st_mpool_t *mpool) {
    size_t         new_block_num;
    st_freelist_t *freelist;
    size_t         chunks_count;

    if (!mpool || mpool->head != NULL || 
     mpool->blocks_count == mpool->blocks_max)
        return false;

    new_block_num = mpool->blocks_count++;
    mpool->blocks[new_block_num] = malloc(mpool->block_size);
    if (!mpool->blocks[new_block_num])
        return false;

    freelist = mpool->blocks[new_block_num];
    chunks_count = mpool->block_size / mpool->blocks_count;
    for (size_t i = 0; i < chunks_count; i++) {
        freelist->next = i == chunks_count - 1
            ? NULL
            : (st_freelist_t *)((char *)freelist + mpool->chunk_size);
        freelist = freelist->next;
    }

    mpool->head = mpool->blocks[new_block_num];

    return true;
}

static st_mpool_t *st_mpool_create(st_mpoolctx_t *mpool_ctx, size_t chunk_size, 
 size_t chunks_count, size_t max_blocks) {
    char           errbuf[ERRMSGBUF_SIZE];
    size_t         real_chunks_count = chunks_count ?: DEFAULT_CHUNKS_COUNT;
    st_mpool_t    *mpool = (st_mpool_t *)st_object_new(sizeof(st_mpool_t), 
     &mpool_funcs, (st_object_dtor_t)st_mpool_destroy, 
     (st_object_t *)mpool_ctx);

    if (!mpool) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(mpool_ctx->logger_ctx, error,
             "mpool_simple: Unable to allocate memory for memory pool object: "
             "%s", errbuf);

        return NULL;
    }

    mpool->blocks_max = max_blocks ?: 1;
    mpool->blocks = malloc(sizeof(void *) * mpool->blocks_max);
    if (mpool->blocks) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(mpool_ctx->logger_ctx, error,
             "mpool_simple: Unable to allocate memory for blocks array: %s", 
             errbuf);

        st_object_destroy((st_object_t *)mpool);

        return NULL;    
    }

    mpool->chunk_size = chunk_size < sizeof(uintptr_t)
        ? sizeof(uintptr_t)
        : chunk_size;
    mpool->block_size = chunk_size * real_chunks_count;
    mpool->blocks_count = 0;
    mpool->head = NULL;

    return mpool;
}

static void st_mpool_destroy(st_mpool_t *mpool) {
    for (size_t i = 0; i < mpool->blocks_count; i++)
        free(mpool->blocks[i]);
    free(mpool->blocks);
    free(mpool);
}

static void *st_mpool_alloc(st_mpool_t *mpool) {
    void *result;
    
    if (mpool->head == NULL && !mpool_grow(mpool))
        return NULL;

    result = mpool->head;
    mpool->head = mpool->head->next;

    return result;
}

static void st_mpool_free(st_mpool_t *mpool, void *ptr) {
    st_freelist_t *old_head = mpool->head;
    
    mpool->head = ptr;
    mpool->head->next = old_head;
}
