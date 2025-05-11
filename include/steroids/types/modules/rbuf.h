#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/modctx.h"
#include "steroids/types/object.h"

#ifndef ST_RBUFCTX_T_DEFINED
    typedef struct st_rbufctx_s st_rbufctx_t;
#endif
#ifndef ST_RBUF_T_DEFINED
    typedef struct st_rbuf_s st_rbuf_t;
#endif

typedef st_rbufctx_t *(*st_rbuf_init_t)(struct st_loggerctx_s *logger_ctx);

typedef st_rbuf_t *(*st_rbuf_create_t)(st_rbufctx_t *rbuf_ctx, size_t size);
typedef bool (*st_rbuf_push_t)(st_rbuf_t *rbuf, const void *data, size_t size);
typedef bool (*st_rbuf_peek_t)(const st_rbuf_t *rbuf, void *data, size_t size);
typedef bool (*st_rbuf_pop_t)(st_rbuf_t *rbuf, void *data, size_t size);
typedef bool (*st_rbuf_drop_t)(st_rbuf_t *rbuf, size_t size);
typedef bool (*st_rbuf_clear_t)(st_rbuf_t *rbuf);
typedef size_t (*st_rbuf_get_free_space_t)(const st_rbuf_t *rbuf);
typedef bool (*st_rbuf_is_empty_t)(const st_rbuf_t *rbuf);

typedef struct {
    st_modctx_funcs_t;
    st_rbuf_create_t create;
} st_rbufctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_rbuf_push_t           push;
    st_rbuf_peek_t           peek;
    st_rbuf_pop_t            pop;
    st_rbuf_drop_t           drop;
    st_rbuf_clear_t          clear;
    st_rbuf_get_free_space_t get_free_space;
    st_rbuf_is_empty_t       is_empty;
} st_rbuf_funcs_t;

#define ST_RBUFCTX_CALL(ctx, func, ...) \
    ((st_rbufctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_RBUF_CALL(object, func, ...) \
    ((st_rbuf_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
