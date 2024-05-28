#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/object.h"

#ifndef ST_DYNARR_T_DEFINED
    typedef struct st_dynarr_s st_dynarr_t;
#endif

typedef st_modctx_t *(*st_dynarr_init_t)(st_modctx_t *logger_ctx);
typedef void (*st_dynarr_quit_t)(st_modctx_t *dynarr_ctx);

typedef st_dynarr_t *(*st_dynarr_create_t)(st_modctx_t *dynarr_ctx,
 size_t data_size, size_t initial_capacity);
typedef void (*st_dynarr_destroy_t)(st_dynarr_t *dynarr);
typedef bool (*st_dynarr_append_t)(st_dynarr_t *dynarr, const void *data);
typedef bool (*st_dynarr_set_t)(st_dynarr_t *dynarr, size_t index,
 const void *data);
typedef bool (*st_dynarr_clear_t)(st_dynarr_t *dynarr);
typedef bool (*st_dynarr_sort_t)(st_dynarr_t *dynarr,
 int (*cmpfunc)(const void *, const void *, void *), void *userptr);
typedef bool (*st_dynarr_extract_t)(const st_dynarr_t *dynarr, void *dst,
 size_t index);
typedef const void *(*st_dynarr_get_t)(st_dynarr_t *dynarr, size_t index);
typedef const void *(*st_dynarr_get_all_t)(st_dynarr_t *dynarr);
typedef size_t (*st_dynarr_get_elements_count_t)(const st_dynarr_t *dynarr);
typedef bool (*st_dynarr_is_empty_t)(const st_dynarr_t *dynarr);

typedef struct {
    st_dynarr_quit_t   quit;
    st_dynarr_create_t create;
} st_dynarrctx_funcs_t;

typedef struct {
    st_dynarr_destroy_t            destroy;
    st_dynarr_append_t             append;
    st_dynarr_set_t                set;
    st_dynarr_clear_t              clear;
    st_dynarr_sort_t               sort;
    st_dynarr_extract_t            extract;
    st_dynarr_get_t                get;
    st_dynarr_get_all_t            get_all;
    st_dynarr_get_elements_count_t get_elements_count;
    st_dynarr_is_empty_t           is_empty;
} st_dynarr_funcs_t;

#define ST_DYNARR_CALL(object, func, ...) \
    ((st_dynarr_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
