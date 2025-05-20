#pragma once

#include <stdbool.h>

#include "steroids/modctx.h"
#include "steroids/moddata.h"
#include "steroids/object.h"

#ifndef ST_MODSMGR_T_DEFINED
    typedef st_object_t st_modsmgr_t;
#endif

struct st_modsmgr_funcs;

typedef st_moddata_t *(*st_modinitfunc_t)(st_modsmgr_t *modsmgr);

typedef bool (*st_modsmgr_load_module_t)(st_modsmgr_t *modsmgr,
 st_modinitfunc_t modinit_func, bool force);
typedef void (*st_modsmgr_process_deps_t)(st_modsmgr_t *modsmgr);
typedef void (*st_modsmgr_get_module_names_t)(st_modsmgr_t *modsmgr, char **dst,
 size_t mods_count, size_t modname_size, const char *subsystem);
typedef st_ctx_ctor_t (*st_modsmgr_get_ctor_t)(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name);

typedef st_modctx_t *(*st_modsmgr_create_singleton_t)(
 const st_modsmgr_t *modsmgr, const char *subsystem, const char *module_name,
 const st_param_t params[]);
typedef bool (*st_modsmgr_have_singleton_t)(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name);
typedef st_modctx_t *(*st_modsmgr_get_singleton_t)(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name);

typedef struct st_modsmgr_funcs {
    st_object_funcs_t;
    st_modsmgr_load_module_t      load_module;
    st_modsmgr_process_deps_t     process_deps;
    st_modsmgr_get_module_names_t get_module_names;
    st_modsmgr_get_ctor_t         get_ctor;
    st_modsmgr_create_singleton_t create_singleton;
    st_modsmgr_have_singleton_t   have_singleton;
    st_modsmgr_get_singleton_t    get_singleton;
} st_modsmgr_funcs_t;

#define ST_MODSMGR_CALL(object, func, ...) \
    ((st_modsmgr_funcs_t *)object->funcs)->func(object, ## __VA_ARGS__)
