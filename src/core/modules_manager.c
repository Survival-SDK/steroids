#include "types.h"
#include "modules_manager.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal_modules.h"
#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "utils.h"

#define FOUND_MODULES_MAX    8
#define FOUND_SINGLETONS_MAX 8

static bool st_modsmgr_load_module(st_modsmgr_t *modsmgr,
 st_modinitfunc_t modinit_func, bool force);
static void st_modsmgr_process_deps(st_modsmgr_t *modsmgr);
static void st_modsmgr_get_module_names(st_modsmgr_t *modsmgr, char **dst,
 size_t mods_count, size_t modname_size, const char *subsystem);
static st_ctx_ctor_t st_modsmgr_get_ctor(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name);
static st_modctx_t *st_modsmgr_create_singleton(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name, const st_param_t params[]);
static bool st_modsmgr_have_singleton(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name);
static st_modctx_t *st_modsmgr_get_singleton(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name);

static void st_modsmgr_destroy(st_modsmgr_t *modsmgr);

static st_modsmgr_funcs_t modsmgr_funcs = {
    st_object_funcs,
    .load_module      = st_modsmgr_load_module,
    .process_deps     = st_modsmgr_process_deps,
    .get_module_names = st_modsmgr_get_module_names,
    .get_ctor         = st_modsmgr_get_ctor,
    .create_singleton = st_modsmgr_create_singleton,
    .have_singleton   = st_modsmgr_have_singleton,
    .get_singleton    = st_modsmgr_get_singleton,
};

static st_moddata_t *st_modsmgr_find_module(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    st_dlnode_t  *node;
    st_moddata_t *found_modules[FOUND_MODULES_MAX];
    size_t        found_count = 0;

    if (!modsmgr || !subsystem)
        return NULL;

    node = st_dlist_get_head(modsmgr->modsdata);
    while (node) {
        st_moddata_t *module_data = st_dlist_export_ptr(node);
        bool          subsystem_equal = st_utl_strings_equal(
         ST_MODDATA_CALL(module_data, get_subsystem), subsystem);
        bool          name_equal = st_utl_strings_equal(
         ST_MODDATA_CALL(module_data, get_name), module_name);
        bool          name_is_null = module_name == NULL;

        if (subsystem_equal && (name_equal || name_is_null))
            found_modules[found_count++] = module_data;

        node = st_dlist_get_next(node);
    }

    for (size_t i = 0; i < found_count; i++) {
        if (!st_utl_strings_equal(ST_MODDATA_CALL(found_modules[i], get_name),
         "simple"))
            return found_modules[i];
    }

    return found_count > 0 ? found_modules[0] : NULL;
}

static bool st_modsmgr_have_module(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    return !!st_modsmgr_find_module(modsmgr, subsystem, module_name);
}

static bool st_modsmgr_module_have_deps(const st_modsmgr_t *modsmgr,
 const st_moddata_t *module_data) {
    int                  i = 0;
    const st_modprerq_t *prereqs = ST_MODDATA_CALL(module_data, get_prereqs);

    while (memcmp(&prereqs[i], &(st_modprerq_t){0}, sizeof(st_modprerq_t)) != 0) {
        bool have_prereq = st_modsmgr_have_module(modsmgr,
         prereqs[i].subsystem, prereqs[i].name);

        if (have_prereq) {
            i++;

            continue;
        }

        if (prereqs[i].name == NULL)
            fprintf(stderr, "steroids: Missing module of subsystem \"%s\" as "
             "prerequisite of module \"%s_%s\"\n",
             prereqs[i].subsystem, ST_MODDATA_CALL(module_data, get_subsystem),
             ST_MODDATA_CALL(module_data, get_name));
        else
            fprintf(stderr,
             "steroids: Missing module \"%s_%s\" as prerequisite of module "
             "\"%s_%s\"\n",
             prereqs[i].subsystem, prereqs[i].name,
             ST_MODDATA_CALL(module_data, get_subsystem),
             ST_MODDATA_CALL(module_data, get_name));

        return false;
    }

    return true;
}

static void st_modsmgr_process_deps(st_modsmgr_t *modsmgr) { // NOLINT(readability-function-cognitive-complexity)
    st_dlnode_t *node = st_dlist_get_head(modsmgr->modsdata);

    while (node) {
        st_moddata_t *module_data = st_dlist_export_ptr(node);

        if (!st_modsmgr_module_have_deps(modsmgr, module_data)) {
            st_dlist_remove(node);
            st_modsmgr_process_deps(modsmgr);

            return;
        }

        node = st_dlist_get_next(node);
    }
}

static void st_modsmgr_get_module_names(st_modsmgr_t *modsmgr, char **dst,
 size_t mods_count, size_t modname_size, const char *subsystem) {
    st_dlnode_t *node;
    size_t       mod_index = 0;

    if (!modsmgr || !subsystem || !dst || !mods_count || !modname_size)
        return;

    node = st_dlist_get_head(modsmgr->modsdata);

    while (node) {
        st_moddata_t *module_data = st_dlist_export_ptr(node);
        char         *modname;
        int           ret;

        node = st_dlist_get_next(node);

        if (!st_utl_strings_equal(
         ST_MODDATA_CALL(module_data, get_subsystem), subsystem))
            continue;

        modname = dst[mod_index];
        if (!modname)
            break;

        /* False positive?  */
        #if defined(__GNUC__) && __GNUC__ >= 7
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-truncation"
        #endif
        ret = snprintf(modname, modname_size, "%s",
         ST_MODDATA_CALL(module_data, get_name));
        #if __GNUC__ >= 7
            #pragma GCC diagnostic pop
        #endif
        if (ret < 0 || (size_t)ret == modname_size)
            continue;

        if (++mod_index == mods_count)
            break;
    }
}

/*
 * Load noninternal module. Internal modules being loaded by function
 * st_modsmgr_init
 */
static bool st_modsmgr_load_module(st_modsmgr_t *modsmgr,
 st_modinitfunc_t modinit_func, bool force) {
    st_moddata_t *module_data = modinit_func(modsmgr);

    printf("steroids: Trying to add module \"%s_%s\"\n",
     ST_MODDATA_CALL(module_data, get_subsystem),
     ST_MODDATA_CALL(module_data, get_name));

    if (!force && !st_modsmgr_module_have_deps(modsmgr, module_data))
        return false;

    return st_dlist_push_back(modsmgr->modsdata, module_data);
}

st_modsmgr_t *st_modsmgr_init(void) {
    st_modsmgr_t *modsmgr = (st_modsmgr_t *)st_object_new(sizeof(st_modsmgr_t),
     &modsmgr_funcs, (st_object_dtor_t)st_modsmgr_destroy, NULL);

    if (!modsmgr)
        return NULL;

    modsmgr->modsdata = st_dlist_create(sizeof(st_moddata_t *),
     st_object_free_by_ptr);
    if (!modsmgr->modsdata)
        goto modsdata_list_fail;

    modsmgr->singletons = st_dlist_create(sizeof(st_modctx_t *),
     st_object_free_by_ptr);
    if (!modsmgr->singletons)
        goto singletons_list_fail;

    printf("steroids: Searching internal modules...\n");
    for (size_t i = 0; i < ST_INTERNAL_MODULES_COUNT; i++) {
        st_moddata_t *module_data =
         st_internal_modules_entrypoints.modules_init_funcs[i](modsmgr);

        if (!module_data)
            continue;

        printf("steroids: Found module \"%s_%s\"\n",
         ST_MODDATA_CALL(module_data, get_subsystem),
         ST_MODDATA_CALL(module_data, get_name));

        st_dlist_push_back(modsmgr->modsdata, &module_data);
    }

    st_modsmgr_process_deps(modsmgr);

    return modsmgr;

singletons_list_fail:
    st_dlist_destroy(modsmgr->modsdata);
modsdata_list_fail:
    free(modsmgr);

    return NULL;
}

static void st_modsmgr_destroy(st_modsmgr_t *modsmgr) {
    st_dlist_destroy(modsmgr->singletons);
    st_dlist_destroy(modsmgr->modsdata);
    free(modsmgr);
}

static st_ctx_ctor_t st_modsmgr_get_ctor(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    st_moddata_t *module_data = st_modsmgr_find_module(modsmgr, subsystem,
     module_name);

    return module_data
        ? ST_MODDATA_CALL(module_data, get_ctx_ctor)
        : NULL;
}

static st_modctx_t *st_modsmgr_get_singleton(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    st_dlnode_t *node;
    st_modctx_t *found_singletons[FOUND_SINGLETONS_MAX];
    size_t       found_count = 0;

    if (!modsmgr || !subsystem)
        return NULL;

    node = st_dlist_get_head(modsmgr->singletons);
    while (node) {
        st_modctx_t *singleton = st_dlist_export_ptr(node);
        bool         subsystem_equal = st_utl_strings_equal(
         ST_MODCTX_CALL(singleton, get_subsystem), subsystem);
        bool         name_equal = st_utl_strings_equal(
         ST_MODCTX_CALL(singleton, get_name), module_name);
        bool         name_is_null = module_name == NULL;

        if (subsystem_equal && (name_equal || name_is_null))
            found_singletons[found_count++] = singleton;

        node = st_dlist_get_next(node);
    }

    for (size_t i = 0; i < found_count; i++) {
        if (!st_utl_strings_equal(ST_MODCTX_CALL(found_singletons[i], get_name),
         "simple"))
            return found_singletons[i];
    }

    return found_count > 0 ? found_singletons[0] : NULL;
}

static bool st_modsmgr_have_singleton(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    return !!st_modsmgr_get_singleton(modsmgr, subsystem, module_name);
}

static st_modctx_t *st_modsmgr_create_singleton(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name, const st_param_t params[]) {
    if (!st_modsmgr_have_singleton(modsmgr, subsystem, module_name)) {
        st_moddata_t *module_data = st_modsmgr_find_module(modsmgr, subsystem,
        module_name);
        st_ctx_ctor_t ctx_ctor = module_data
            ? ST_MODDATA_CALL(module_data, get_ctx_ctor)
            : NULL;
        st_modctx_t  *ctx = ctx_ctor
            ? ctx_ctor(params)
            : NULL;

        if (!st_dlist_push_back(modsmgr->singletons, &ctx)) {
            ST_MODCTX_CALL(ctx, destroy);
            ctx = NULL;
        }

        return ctx;
    }

    return NULL;
}
