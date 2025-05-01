#pragma once

#include <stddef.h>
#include <stdint.h>

#define ST_OBJCALL(obj, func, ...) obj->funcs.func(obj, ## __VA_ARGS__);
#define ST_ALLOCATOR_CALL(allocator, func, ...) \
    ((st_allocator_funcs_t *)((const st_object_t *)allocator)->funcs)->func(allocator, ## __VA_ARGS__)

struct st_object;
struct st_allocator;

typedef void (*st_object_dtor_t)(void *obj);
typedef struct st_object *(*st_object_get_owner_t)(void *obj);
typedef void (*st_object_destroy_t)(void *obj);
typedef void *(*st_allocator_alloc_t)(struct st_allocator *obj);
typedef void (*st_allocator_free_t)(struct st_allocator *obj, void *ptr);

typedef struct {
    st_object_get_owner_t get_owner;
    st_object_destroy_t   destroy;
} st_object_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_allocator_alloc_t alloc;
    st_allocator_free_t  free;
} st_allocator_funcs_t;

typedef struct st_object {
    const st_object_funcs_t *funcs;
    struct st_object        *st_owner;
    st_object_dtor_t         st_dtor;
    struct st_allocator     *st_allocator;
} st_object_t;

typedef struct st_allocator {
    st_object_t;
} st_allocator_t;

static st_object_t *st_object_get_owner(void *obj);
static void st_object_destroy(void *obj);

static const st_object_funcs_t st_object_funcs = { 
    .get_owner = st_object_get_owner,
    .destroy = st_object_destroy,
};

static st_object_t *st_object_init(st_object_t *obj, const void *funcs, 
 st_object_dtor_t dtor, st_object_t *owner, st_allocator_t *allocator) {
    if (!obj)
        return NULL;

    obj->funcs        = funcs ?: &st_object_funcs;
    obj->st_dtor      = dtor;
    obj->st_owner     = owner;
    obj->st_allocator = allocator;

    return obj;
}

static st_object_t *st_object_new(size_t size, const void *funcs, 
 st_object_dtor_t dtor, st_object_t *owner) {
    st_object_t *obj = malloc(size);

    return st_object_init(obj, funcs, dtor, owner, NULL);
}

static st_object_t *st_object_alloc(const void *funcs, 
 st_object_dtor_t dtor, st_object_t *owner, st_allocator_t *allocator) {
    st_object_t *obj = ST_ALLOCATOR_CALL(allocator, alloc);

    return st_object_init(obj, funcs, dtor, owner, allocator);
}

static st_object_t *st_object_get_owner(void *obj) {
    return ((st_object_t *)obj)->st_owner;
}

static void st_object_destroy(void *obj) {
    if (((st_object_t *)obj)->st_dtor)
        ((st_object_t *)obj)->st_dtor(obj);
    else if (((st_object_t *)obj)->st_allocator)
        ST_ALLOCATOR_CALL(((st_object_t *)obj)->st_allocator, free, obj);
    else
        free(obj);
}
