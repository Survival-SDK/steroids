#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"
#include "steroids/runnablectx.h"

#ifndef ST_LUAJITCTX_T_DEFINED
    typedef st_runnablectx_t st_luajitctx_t;
#endif
#ifndef ST_LUAJITSTATE_T_DEFINED
    typedef st_object_t st_luajitstate_t;
#endif

typedef bool (*st_luajit_run_t)(st_runnablectx_t *luajit_ctx,
 const st_param_t params[]);
typedef st_luajitstate_t *(*st_luajit_newstate_t)(st_luajitctx_t *luajit_ctx,
 const char *name);
typedef st_luajitstate_t *(*st_luajit_getstate_t)(st_luajitctx_t *luajit_ctx,
 const char *name);

typedef st_luajitstate_t *(*st_luajit_newthread_t)(
 st_luajitstate_t *luajit_state, const char *name);
typedef bool (*st_luajit_run_string_t)(st_luajitstate_t *state,
 const char *string);
typedef bool (*st_luajit_run_file_t)(st_luajitstate_t *state,
 const char *filename);

typedef struct {
    st_modctx_funcs_t;
    st_luajit_run_t      run;
    st_luajit_newstate_t new_state;
    st_luajit_getstate_t get_state;
} st_luajitctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_luajit_newthread_t  new_thread;
    st_luajit_run_string_t run_string;
    st_luajit_run_file_t   run_file;
} st_luajitstate_funcs_t;

#define ST_LUAJITCTX_CALL(object, func, ...) \
    ((st_luajitctx_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
#define ST_LUAJITSTATE_CALL(object, func, ...) \
    ((st_luajitstate_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
