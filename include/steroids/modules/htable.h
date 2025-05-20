#pragma once

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

#ifndef ST_HTABLECTX_T_DEFINED
    typedef struct st_htablectx_s st_htablectx_t;
#endif
#ifndef ST_HTABLE_T_DEFINED
    typedef struct st_htable_s st_htable_t;
#endif
#ifndef ST_HTITER_T_DEFINED
    typedef st_object_t st_htiter_t;
#endif

typedef uint32_t (*st_u32hashfunc_t)(const void *str);
typedef bool (*st_keyeqfunc_t)(const void *left, const void *right);
#ifndef ST_FREEFUNC_T_DEFINED
    typedef void (*st_freefunc_t)(void *ptr);
#endif

typedef st_htable_t *(*st_htable_create_t)(st_htablectx_t *htable_ctx,
 st_u32hashfunc_t hashfunc, st_keyeqfunc_t keyeqfunc, st_freefunc_t keydelfunc,
 st_freefunc_t valdelfunc);
typedef bool (*st_htable_insert_t)(st_htable_t *htable, st_htiter_t *iter,
 const void *key, void *value);
typedef void *(*st_htable_get_t)(st_htable_t *htable, const void *key);
typedef bool (*st_htable_remove_t)(st_htable_t *htable, const void *key);
typedef void (*st_htable_clear_t)(st_htable_t *htable);
typedef bool (*st_htable_contains_t)(st_htable_t *htable, const void *key);
typedef bool (*st_htable_find_t)(st_htable_t *htable, st_htiter_t *dst,
 const void *key);
typedef bool (*st_htable_first_t)(st_htable_t *htable, st_htiter_t *dst);
typedef bool (*st_htable_next_t)(st_htiter_t *current, st_htiter_t *dst);
typedef const void *(*st_htable_get_iter_key_t)(const st_htiter_t *iter);
typedef void *(*st_htable_get_iter_value_t)(const st_htiter_t *iter);

typedef struct {
    st_modctx_funcs_t;
    st_htable_create_t create;
} st_htablectx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_htable_insert_t   insert;
    st_htable_get_t      get;
    st_htable_remove_t   remove;
    st_htable_clear_t    clear;
    st_htable_contains_t contains;
    st_htable_find_t     find;
    st_htable_first_t    get_first;
} st_htable_funcs_t;

typedef struct st_htiter_funcs {
    st_object_funcs_t;
    st_htable_next_t           get_next;
    st_htable_get_iter_key_t   get_key;
    st_htable_get_iter_value_t get_value;
} st_htiter_funcs_t;

#define ST_HTABLECTX_CALL(object, func, ...) \
    ((st_htablectx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
#define ST_HTABLE_CALL(object, func, ...) \
    ((st_htable_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
#define ST_HTITER_CALL(object, func, ...) \
    ((st_htiter_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
