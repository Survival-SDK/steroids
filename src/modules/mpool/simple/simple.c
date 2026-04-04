#include "simple.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ERRMSGBUF_SIZE 128
#define DEFAULT_CHUNKS_COUNT 32

static st_mpoolctx_t *st_mpool_init(const st_param_t params[]);
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

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_mpool_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("mpool", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_mpool_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_mpool_simple_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "mpool";
static const char *st_module_name = "simple";

static st_mpoolctx_t *st_mpool_init(const st_param_t params[]) {
    st_modsmgr_t    *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t  *logger_ctx;
    st_mpoolctx_t   *mpool_ctx;

    if (!modsmgr)
        return NULL;
    
    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    mpool_ctx = (st_mpoolctx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_mpoolctx_t), NULL, &mpoolctx_funcs, 
     (st_object_dtor_t)st_mpool_quit);
    if (!mpool_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create mpool context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    mpool_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Memory pool context initialized", st_module_subsystem, 
     st_module_name);

    return mpool_ctx;
}

static void st_mpool_quit(st_mpoolctx_t *mpool_ctx) {
    ST_LOGGERCTX_CALL(mpool_ctx->logger_ctx, info,
     "%s_%s: Memory pool context destroyed", st_module_subsystem, 
     st_module_name);
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
    chunks_count = mpool->block_size / mpool->chunk_size;
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
             "%s_%s: Unable to allocate memory for memory pool object: %s",
             st_module_subsystem, st_module_name, errbuf);

        return NULL;
    }

    mpool->blocks_max = max_blocks ?: 1;
    mpool->blocks = malloc(sizeof(void *) * mpool->blocks_max);
    if (!mpool->blocks) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(mpool_ctx->logger_ctx, error,
             "%s_%s: Unable to allocate memory for blocks array: %s", 
             st_module_subsystem, st_module_name, errbuf);

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
