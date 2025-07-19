#include "dlist.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

st_dlist_t *st_dlist_create(size_t data_size, void (*elem_free_func)(void *)) {
    st_dlist_t *list;

    if (data_size == 0)
        return NULL;

    list = malloc(sizeof(st_dlist_t));
    if (!list)
        return NULL;

    list->head = NULL;
    list->tail = NULL;
    list->data_size = data_size;
    list->elems_count = 0;
    list->elem_free_func = elem_free_func;

    return list;
}

st_dlnode_t *st_dlist_get_head(const st_dlist_t *list) {
    return list ? list->head : NULL;
}

st_dlnode_t *st_dlist_get_tail(const st_dlist_t *list) {
    return list ? list->tail : NULL;
}

st_dlnode_t *st_dlist_get_prev(const st_dlnode_t *node) {
    return node ? node->prev : NULL;
}

st_dlnode_t *st_dlist_get_next(const st_dlnode_t *node) {
    return node ? node->next : NULL;
}

size_t st_dlist_get_elems_count(const st_dlist_t *list) {
    return list ? list->elems_count : 0;
}

bool st_dlist_empty(const st_dlist_t *list) {
    return list ? list->elems_count == 0 : false;
}

static st_dlnode_t *st_dlist_create_node(st_dlist_t *list, const void *data) {
    st_dlnode_t *node;

    if (!list || !data)
        return NULL;

    node = malloc(sizeof(st_dlnode_t) + list->data_size);
    if (!node)
        return NULL;

    node->list = list;
    node->prev = NULL;
    node->next = NULL;
    memcpy(node->data, data, list->data_size);

    return node;
}

st_dlnode_t *st_dlist_insert_before(st_dlnode_t *node, const void *data) {
    st_dlist_t  *list;
    st_dlnode_t *new_node;
    st_dlnode_t *prev;

    if (!node || !data)
        return NULL;

    list = node->list;
    prev = node->prev;

    new_node = st_dlist_create_node(list, data);
    if (!new_node)
        return NULL;

    new_node->prev = prev;
    new_node->next = node;
    node->prev = new_node;
    if (prev)
        prev->next = new_node;

    if (list->head == node)
        list->head = new_node;

    list->elems_count++;

    return new_node;
}

st_dlnode_t *st_dlist_insert_after(st_dlnode_t *node, const void *data) {
    st_dlist_t  *list;
    st_dlnode_t *new_node;
    st_dlnode_t *next;

    if (!node || !data)
        return NULL;

    list = node->list;
    next = node->next;

    new_node = st_dlist_create_node(list, data);
    if (!new_node)
        return NULL;

    new_node->prev = node;
    new_node->next = next;
    if (next)
        next->prev = new_node;
    node->next = new_node;

    if (list->tail == node)
        list->tail = new_node;

    list->elems_count++;

    return new_node;
}

static st_dlnode_t *st_dlist_insert_initial(st_dlist_t *list,
 const void *data) {
    st_dlnode_t *node;

    if (!list || !data || list->head)
        return NULL;

    node = st_dlist_create_node(list, data);
    list->head = node;
    list->tail = node;
    list->elems_count = 1;

    return node;
}

st_dlnode_t *st_dlist_push_front(st_dlist_t *list, const void *data) {
    if (!list || !data)
        return NULL;

    if (st_dlist_empty(list))
        return st_dlist_insert_initial(list, data);

    return st_dlist_insert_before(list->head, data);
}

st_dlnode_t *st_dlist_push_back(st_dlist_t *list, const void *data) {
    if (!list || !data)
        return NULL;

    if (st_dlist_empty(list))
        return st_dlist_insert_initial(list, data);

    return st_dlist_insert_after(list->tail, data);
}

void st_dlist_remove(st_dlnode_t *node) {
    st_dlist_t  *list;
    st_dlnode_t *prev;
    st_dlnode_t *next;

    if (!node)
        NULL;

    list = node->list;
    prev = node->prev;
    next = node->next;

    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;

    if (node == list->tail)
        list->tail = next;
    if (node == list->head)
        list->head = prev;

    list->elems_count--;

    if (list->elem_free_func)
        list->elem_free_func(node->data);

    free(node);
}

bool st_dlist_clear(st_dlist_t *list) {
    if (!list)
        return false;

    while (list->head)
        st_dlist_remove(list->head); // NOLINT(clang-analyzer-unix.Malloc)

    return true;
}

bool st_dlist_destroy(st_dlist_t *list) {
    if (!list)
        return false;

    st_dlist_clear(list);
    free(list);

    return true;
}

bool st_dlist_import_data(st_dlnode_t *node, const void *data) {
    if (!node)
        return false;

    memcpy(node->data, data, node->list->data_size);

    return true;
}

bool st_dlist_export_data(void *data, const st_dlnode_t *node) {
    if (!node)
        return false;

    memcpy(data, node->data, node->list->data_size);

    return true;
}

/* For cases where data stores pointer */
void *st_dlist_export_ptr(const st_dlnode_t *node) {
    return node ? *(void **)node->data : NULL;
}

void *st_dlist_get_data(st_dlnode_t *node) {
    return node ? node->data : NULL;
}
