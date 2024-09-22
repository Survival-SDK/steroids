#pragma once

#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/object.h"

#ifndef ST_SOCTX_T_DEFINED
    typedef struct st_soctx_s st_soctx_t;
#endif
#ifndef ST_SO_T_DEFINED
    typedef struct st_so_s st_so_t;
#endif

typedef st_soctx_t *(*st_so_init_t)(struct st_loggerctx_s *logger_ctx);
typedef void (*st_so_quit_t)(st_soctx_t *so_ctx);

typedef st_so_t *(*st_so_open_t)(st_soctx_t *so_ctx, const char *filename);
typedef st_so_t *(*st_so_memopen_t)(st_soctx_t *so_ctx, const void *data,
 size_t size);
typedef void (*st_so_close_t)(st_so_t *so);
typedef void *(*st_so_load_symbol_t)(st_so_t *so, const char *name);

typedef struct {
    st_so_quit_t    quit;
    st_so_open_t    open;
    st_so_memopen_t memopen;
} st_soctx_funcs_t;

typedef struct {
    st_so_close_t       close;
    st_so_load_symbol_t load_symbol;
} st_so_funcs_t;

#define ST_SOCTX_CALL(object, func, ...) \
    ((st_soctx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
#define ST_SO_CALL(object, func, ...) \
    ((st_so_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
