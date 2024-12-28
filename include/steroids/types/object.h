#pragma once

#include <stddef.h>

// #define ST_OBJCALL(obj, func, ...) obj->funcs.func(obj, ## __VA_ARGS__);

typedef uintptr_t st_weakptr_t;

struct st_object;

typedef size_t (*st_object_dtor_t)(struct st_object *obj);

typedef st_weakptr_t (*st_object_get_owner_t)(struct st_object *obj);
typedef struct st_object *(*st_object_grab_t)(struct st_object *obj);
typedef void (*st_object_release_t)(void *obj);
typedef struct st_object *(*st_object_move_t)(struct st_object **obj);

typedef size_t (*st_object_destroy_t)(struct st_object *obj);

static st_weakptr_t st_weakptr_create(struct st_object *obj);

typedef struct {
    st_object_get_owner_t get_owner;
    st_object_grab_t      grab;
    st_object_release_t   release;
    st_object_move_t      move;
} st_object_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_object_destroy_t destroy;
} st_objbase_funcs_t;

typedef struct st_object {
    st_object_dtor_t         dtor;
    const st_object_funcs_t *funcs;
    size_t                   st_refs;
    st_weakptr_t             st_owner;
} st_object_t;

static st_weakptr_t st_object_get_owner(st_object_t *obj);
static st_object_t *st_object_grab(st_object_t *obj);
static void st_object_release(void *obj);
static st_object_t *st_object_move(st_object_t **obj);
static size_t st_object_destroy(st_object_t *obj);

static const st_object_funcs_t st_object_funcs = { 
    .get_owner = st_object_get_owner,
    .grab      = st_object_grab,      
    .release   = st_object_release,   
    .move      = st_object_move,
};

static st_object_t *st_object_new(size_t size, st_object_dtor_t dtor, 
 const void *funcs, st_object_t *owner) {
    st_object_t *obj = malloc(size);

    if (!obj)
        return NULL;

    obj->dtor     = dtor ?: st_object_destroy;
    obj->funcs    = funcs ?: &st_object_funcs;
    obj->st_refs  = 1;
    obj->st_owner = st_weakptr_create(owner);

    return obj;
}

static st_weakptr_t st_object_get_owner(st_object_t *obj) {
    return obj->st_owner;
}

static st_object_t *st_object_grab(st_object_t *obj) {
    if (obj)
        obj->st_refs++;
    return obj;
}

static void st_object_release(void *obj) {
    if (obj && --((st_object_t *)obj)->st_refs == 0)
        ((st_object_t *)obj)->dtor(obj);
}

static st_object_t *st_object_move(st_object_t **obj) {
    st_object_t *temp = *obj;

    *obj = NULL;

    return temp;
}

static size_t st_object_destroy(st_object_t *obj) {
    size_t remain_refs = obj->st_refs;

    free(obj);

    return remain_refs;
}

static st_weakptr_t st_weakptr_create(st_object_t *obj) {
    return (st_weakptr_t)obj;
}

static st_object_t *st_weakptr_grab(st_weakptr_t weakptr) {
    return st_object_grab((st_object_t *)weakptr);
}
