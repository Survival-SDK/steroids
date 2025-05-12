#include "hash_table.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hash_table.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ITERS_COUNT_MAX 128

static st_htablectx_t *st_htable_init(const st_ctxctorparam_t params[]);
static void st_htable_quit(st_htablectx_t *htable_ctx);

static st_htable_t *st_htable_create(st_htablectx_t *htable_ctx,
 st_u32hashfunc_t hashfunc, st_keyeqfunc_t keyeqfunc, st_freefunc_t keydelfunc,
 st_freefunc_t valdelfunc);
static void st_htable_destroy(st_htable_t *htable);
static bool st_htable_insert(st_htable_t *htable, st_htiter_t *iter,
 const void *key, void *value);
static void *st_htable_get(st_htable_t *htable, const void *key);
static bool st_htable_remove(st_htable_t *htable, const void *key);
static void st_htable_clear(st_htable_t *htable);
static bool st_htable_contains(st_htable_t *htable, const void *key);
static bool st_htable_find(st_htable_t *htable, st_htiter_t *dst,
 const void *key);
static bool st_htable_first(st_htable_t *htable, st_htiter_t *dst);
static bool st_htable_next(st_htiter_t *current, st_htiter_t *dst);
static const void *st_htable_get_iter_key(const st_htiter_t *iter);
static void *st_htable_get_iter_value(const st_htiter_t *iter);

static st_htablectx_funcs_t htablectx_funcs = {
    st_modctx_funcs,
    .create = st_htable_create,
};

static st_htable_funcs_t htable_funcs = {
    st_object_funcs,
    .insert    = st_htable_insert,
    .get       = st_htable_get,
    .remove    = st_htable_remove,
    .clear     = st_htable_clear,
    .contains  = st_htable_contains,
    .find      = st_htable_find,
    .get_first = st_htable_first,
};

static st_htiter_funcs_t htiter_funcs = {
    st_object_funcs,
    .get_next  = st_htable_next,
    .get_key   = st_htable_get_iter_key,
    .get_value = st_htable_get_iter_value,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_htable_hash_table_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("htable", "hash_table", ST_MODULE_TYPE, mod_prereqs,
     st_htable_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_htable_hash_table_init(modsmgr, modsmgr_funcs);
}
#endif

static st_htablectx_t *st_htable_init(const st_ctxctorparam_t params[]) {
    st_modsmgr_t          *modsmgr = st_modctx_get_param_as_ptr(params,
     "modsmgr");
    struct st_loggerctx_s *logger_ctx = (
     struct st_loggerctx_s *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    st_htablectx_t        *htable_ctx = (st_htablectx_t *)st_modctx_new(
     "htable", "hash_table", sizeof(st_htablectx_t), NULL, &htablectx_funcs,
     (st_object_dtor_t)st_htable_quit);

    if (!htable_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "htable_hash_table: unable to create new htable ctx object");

        return NULL;
    }

    htable_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "htable_hash_table: hash tables manipulation module context initialized");

    return htable_ctx;
}

static void st_htable_quit(st_htablectx_t *htable_ctx) {
    ST_LOGGERCTX_CALL(htable_ctx->logger_ctx, info,
     "htable_hash_table: hash tables manipulation module context destroyed");
    free(htable_ctx);
}

static st_htable_t *st_htable_create(st_htablectx_t *htable_ctx,
 st_u32hashfunc_t hashfunc, st_keyeqfunc_t keyeqfunc, st_freefunc_t keydelfunc,
 st_freefunc_t valdelfunc) {
    struct hash_table *handle = hash_table_create(hashfunc, keyeqfunc);
    st_htable_t       *htable;

    if (!handle) {
        ST_LOGGERCTX_CALL(htable_ctx->logger_ctx, error,
         "htable_hash_table: Unable to create hash table handle");

        return NULL;
    }

    htable = (st_htable_t *)st_object_new(sizeof(st_htable_t), &htable_funcs,
     (st_object_dtor_t)st_htable_destroy, (st_object_t *)htable_ctx);
    if (!htable) {
        ST_LOGGERCTX_CALL(htable_ctx->logger_ctx, error,
         "htable_hash_table: Unable allocate memory for hash_table");
        hash_table_destroy(handle, NULL);

        return NULL;
    }

    htable->handle = handle;
    htable->keydelfunc = keydelfunc;
    htable->valdelfunc = valdelfunc;

    return htable;
}

static void st_htable_destroy(st_htable_t *htable) {
    if (htable) {
        ST_HTABLE_CALL(htable, clear);
        hash_table_destroy(htable->handle, NULL);
        free(htable);
    }
}

static bool st_htable_insert(st_htable_t *htable, st_htiter_t *iter,
 const void *key, void *value) {
    bool               delete_old;
    const void        *old_key;
    void              *old_value;
    struct hash_entry *entry = hash_table_search(htable->handle, key);

    if (entry) {
        delete_old = true;
        old_key = entry->key;
        old_value = entry->data;
    }

    entry = hash_table_insert(htable->handle, key, value);
    if (!entry)
        return false;

    if (iter) {
        st_object_placement_new(iter, &htiter_funcs, st_object_fake_dtor,
         (st_object_t *)htable);
        iter->st_userdata = (uintptr_t)entry;
    }

    if (delete_old) {
        if (htable->keydelfunc) { /* keydelfunc fails without braces */
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wcast-qual"
            htable->keydelfunc((void *)old_key);
            #pragma GCC diagnostic pop
        }
        if (htable->valdelfunc)
            htable->valdelfunc(old_value);
    }

    return true;
}

static void *st_htable_get(st_htable_t *htable, const void *key) {
    st_htiter_t iter;

    if (ST_HTABLE_CALL(htable, find, &iter, key))
        return ST_HTITER_CALL(&iter, get_value);

    return NULL;
}

static bool st_htable_remove(st_htable_t *htable, const void *key) {
    struct hash_entry *entry = hash_table_search(htable->handle, key);

    if (entry) {
        if (htable->keydelfunc) { /* keydelfunc fails without braces */
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wcast-qual"
            htable->keydelfunc((void *)entry->key);
            #pragma GCC diagnostic pop
        }
        if (htable->valdelfunc)
            htable->valdelfunc(entry->data);

        hash_table_remove_entry(htable->handle, entry);
    }

    return !!entry;
}

static void st_htable_clear(st_htable_t *htable) {
    struct hash_entry *entry = hash_table_next_entry(htable->handle, NULL);

    while (entry) {
        if (htable->keydelfunc) { /* keydelfunc fails without braces */
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wcast-qual"
            htable->keydelfunc((void *)entry->key);
            #pragma GCC diagnostic pop
        }
        if (htable->valdelfunc)
            htable->valdelfunc(entry->data);
        hash_table_remove_entry(htable->handle, entry);
        entry = hash_table_next_entry(htable->handle, entry);
    }
}

static bool st_htable_contains(st_htable_t *htable, const void *key) {
    return !!hash_table_search(htable->handle, key);
}

static bool st_htable_find(st_htable_t *htable, st_htiter_t *dst,
 const void *key) {
    struct hash_entry *handle;

    if (!dst)
        return false;

    handle = hash_table_search(htable->handle, key);
    if (!handle)
        return false;

    st_object_placement_new(dst, &htiter_funcs, st_object_fake_dtor,
     (st_object_t *)htable);
    dst->st_userdata = (uintptr_t)handle;

    return true;
}

static bool st_htable_first_or_next(st_htable_t *htable, st_htiter_t *current,
 st_htiter_t *dst) {
    struct hash_entry *entry;

    if (!htable && !current)
        return false;

    entry = hash_table_next_entry(htable->handle,
     current ? (struct hash_entry *)current->st_userdata : NULL);

    if (!entry)
        return false;

    st_object_placement_new(dst, &htiter_funcs, st_object_fake_dtor,
     (st_object_t *)htable);
    dst->st_userdata = (uintptr_t)entry;

    return true;
}

static bool st_htable_first(st_htable_t *htable, st_htiter_t *dst) {
    return htable ? st_htable_first_or_next(htable, NULL, dst) : false;
}

static bool st_htable_next(st_htiter_t *current, st_htiter_t *dst) {
    return current
        ? st_htable_first_or_next((st_htable_t *)st_object_get_owner(current),
           current, dst)
        : false;
}

static const void *st_htable_get_iter_key(const st_htiter_t *iter) {
    return ((struct hash_entry *)iter->st_userdata)->key;
}

static void *st_htable_get_iter_value(const st_htiter_t *iter) {
    return ((struct hash_entry *)iter->st_userdata)->data;
}
