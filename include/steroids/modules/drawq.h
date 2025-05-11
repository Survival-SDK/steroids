#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"
#include "steroids/modules/sprite.h"
#include "steroids/object.h"

#ifndef ST_DRAWQ_T_DEFINED
    typedef struct st_drawq_s st_drawq_t;
#endif

typedef struct {
    const st_sprite_t *sprite;
    float              x;
    float              y;
    float              z;
    float              hscale;
    float              vscale;
    float              angle;
    float              hshear;
    float              vshear;
    float              pivot_x;
    float              pivot_y;
} st_drawrec_t;

typedef st_modctx_t *(*st_drawq_init_t)(st_modctx_t *dynarr_ctx,
 st_modctx_t *logger_ctx, st_modctx_t *sprite_ctx);
typedef void (*st_drawq_quit_t)(st_modctx_t *drawq_ctx);

typedef st_drawq_t *(*st_drawq_create_t)(st_modctx_t *drawq_ctx);
typedef void (*st_drawq_destroy_t)(st_drawq_t *drawq);
typedef size_t (*st_drawq_len_t)(const st_drawq_t *drawq);
typedef bool (*st_drawq_empty_t)(const st_drawq_t *drawq);
typedef bool (*st_drawq_export_entry_t)(const st_drawq_t *drawq,
 st_drawrec_t *drawrec, size_t index);
typedef const st_drawrec_t *(*st_drawq_get_all_t)(const st_drawq_t *drawq);
typedef bool (*st_drawq_add_t)(st_drawq_t *drawq, const st_sprite_t *sprite,
 float x, float y, float z, float hscale, float vscale, float angle,
 float hshear, float vshear, float pivot_x, float pivot_y);
typedef bool (*st_drawq_sort_t)(st_drawq_t *drawq);
typedef bool (*st_drawq_clear_t)(st_drawq_t *drawq);

typedef struct {
    st_drawq_quit_t   quit;
    st_drawq_create_t create;
} st_drawqctx_funcs_t;

typedef struct {
    st_drawq_destroy_t      destroy;
    st_drawq_len_t          len;
    st_drawq_empty_t        empty;
    st_drawq_export_entry_t export_entry;
    st_drawq_get_all_t      get_all;
    st_drawq_add_t          add;
    st_drawq_sort_t         sort;
    st_drawq_clear_t        clear;
} st_drawq_funcs_t;

#define ST_DRAWQ_CALL(object, func, ...) \
    ((st_drawq_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
