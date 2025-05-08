#pragma once

#include <stddef.h>
#include <stdint.h>

#define ST_OBJECT_CALL(obj, func, ...) obj->funcs->func(obj, ## __VA_ARGS__);

struct st_object;

typedef void (*st_object_dtor_t)(void *obj);

typedef const struct st_object *(*st_object_get_owner_t)(const void *obj);
typedef struct st_object *(*st_object_get_owner_unsafe_t)(void *obj);
typedef void (*st_object_destroy_t)(void *obj);

typedef struct {
    st_object_get_owner_t        get_owner;
    st_object_get_owner_unsafe_t get_owner_unsafe;
    st_object_destroy_t          destroy;
} st_object_funcs_t;

typedef struct st_object {
    const st_object_funcs_t *funcs;
    struct st_object        *st_owner;
    st_object_dtor_t         st_dtor;
    uintptr_t                st_userdata;
} st_object_t;

static const st_object_t *st_object_get_owner(const void *obj);
static st_object_t *st_object_get_owner_unsafe(void *obj);
static void st_object_destroy(void *obj);

static const st_object_funcs_t st_object_funcs = { 
    .get_owner        = st_object_get_owner,
    .get_owner_unsafe = st_object_get_owner_unsafe,
    .destroy          = st_object_destroy,
};

static st_object_t *st_object_init(st_object_t *obj, const void *funcs, 
 st_object_dtor_t dtor, st_object_t *owner) {
    if (!obj)
        return NULL;

    obj->funcs    = funcs ?: &st_object_funcs;
    obj->st_dtor  = dtor;
    obj->st_owner = owner;
    obj->st_userdata = 0ul;

    return obj;
}

static st_object_t *st_object_new(size_t size, const void *funcs, 
 st_object_dtor_t dtor, st_object_t *owner) {
    st_object_t *obj = malloc(size);

    return st_object_init(obj, funcs, dtor, owner);
}

static st_object_t *st_object_placement_new(void *buffer, const void *funcs, 
 st_object_dtor_t dtor, st_object_t *owner) {
    return dtor
        ? st_object_init(buffer, funcs, dtor, owner)
        : NULL;
}

static const st_object_t *st_object_get_owner(const void *obj) {
    return ((const st_object_t *)obj)->st_owner;
}

static st_object_t *st_object_get_owner_unsafe(void *obj) {
    return ((st_object_t *)obj)->st_owner;
}

static void st_object_destroy(void *obj) {
    if (((st_object_t *)obj)->st_dtor)
        ((st_object_t *)obj)->st_dtor(obj);
    else
        free(obj);
}

/* For stack objects, created with placement new */
static void st_object_fake_dtor(void *obj) {
    /* noop */
}
