#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct st_dlnode_s st_dlnode_t;
typedef struct st_dlist_s st_dlist_t;

st_dlist_t *st_dlist_create(size_t data_size, void (*elem_free_func)(void *));
st_dlnode_t *st_dlist_get_head(const st_dlist_t *list);
st_dlnode_t *st_dlist_get_tail(const st_dlist_t *list);
st_dlnode_t *st_dlist_get_prev(const st_dlnode_t *node);
st_dlnode_t *st_dlist_get_next(const st_dlnode_t *node);
size_t st_dlist_get_elems_count(const st_dlist_t *list);
bool st_dlist_empty(const st_dlist_t *list);
st_dlnode_t *st_dlist_insert_before(st_dlnode_t *node, const void *data);
st_dlnode_t *st_dlist_insert_after(st_dlnode_t *node, const void *data);
st_dlnode_t *st_dlist_push_front(st_dlist_t *list, const void *data);
st_dlnode_t *st_dlist_push_back(st_dlist_t *list, const void *data);
void st_dlist_remove(st_dlnode_t *node);
bool st_dlist_clear(st_dlist_t *list);
bool st_dlist_destroy(st_dlist_t *list);
bool st_dlist_import_data(st_dlnode_t *node, const void *data);
bool st_dlist_export_data(void *data, const st_dlnode_t *node);
/* For cases where data stores pointer */
void *st_dlist_export_ptr(const st_dlnode_t *node);
void *st_dlist_get_data(st_dlnode_t *node);
