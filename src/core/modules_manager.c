#include "types.h"
#include "modules_manager.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal_modules.h"
#include "utils.h"

#define FOUND_MODULES_MAX 8
#define ST_MODSMGR_FUNCS                                 \
    &(st_modsmgr_funcs_t){                               \
        .get_module_names = st_modsmgr_get_module_names, \
        .load_module      = st_modsmgr_load_module,      \
        .process_deps     = st_modsmgr_process_deps,     \
        .get_ctor         = st_modsmgr_get_ctor,         \
     }

st_modctx_t *st_modsmgr_init_module_ctx(st_modsmgr_t *modsmgr,
 const st_moddata_t *module_data, size_t data_size);
void st_free_module_ctx(st_modsmgr_t *modsmgr, st_modctx_t *modctx);

static st_moddata_t *st_modsmgr_find_module(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    st_dlnode_t  *node;
    st_moddata_t *found_modules[FOUND_MODULES_MAX];
    size_t        found_count = 0;

    if (!modsmgr || !subsystem)
        return NULL;

    node = st_dlist_get_head(modsmgr);
    while (node) {
        st_moddata_t *module_data = st_dlist_get_data(node);
        bool          subsystem_equal = st_utl_strings_equal(
         module_data->subsystem, subsystem);
        bool          name_equal = st_utl_strings_equal(module_data->name,
         module_name);
        bool          name_is_null = module_name == NULL;

        if (subsystem_equal && (name_equal || name_is_null))
            found_modules[found_count++] = module_data;

        node = st_dlist_get_next(node);
    }

    for (size_t i = 0; i < found_count; i++) {
        if (!st_utl_strings_equal(found_modules[i]->name, "simple"))
            return found_modules[i];
    }

    return found_count > 0 ? found_modules[0] : NULL;
}

static inline bool st_modsmgr_have_module(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    return !!st_modsmgr_find_module(modsmgr, subsystem, module_name);
}

static bool st_modsmgr_module_have_deps(const st_modsmgr_t *modsmgr,
 const st_moddata_t *module_data) {
    int i = 0;

    while (memcmp(&module_data->prereqs[i], &(st_modprerq_t){0}, sizeof(st_modprerq_t)) != 0) {
        bool have_prereq = st_modsmgr_have_module(modsmgr,
         module_data->prereqs[i].subsystem, module_data->prereqs[i].name);

        if (have_prereq) {
            i++;

            continue;
        }

        if (module_data->prereqs[i].name == NULL)
            fprintf(stderr, "steroids: Missing module of subsystem \"%s\" as "
             "prerequisite of module \"%s_%s\"\n",
             module_data->prereqs[i].subsystem,
             module_data->subsystem, module_data->name);
        else
            fprintf(stderr,
             "steroids: Missing module \"%s_%s\" as prerequisite of module "
             "\"%s_%s\"\n",
             module_data->prereqs[i].subsystem, module_data->prereqs[i].name,
             module_data->subsystem, module_data->name);

        return false;
    }

    return true;
}

static void st_modsmgr_process_deps(st_modsmgr_t *modsmgr) { // NOLINT(readability-function-cognitive-complexity)
    st_dlnode_t *node = st_dlist_get_head(modsmgr);

    while (node) {
        st_moddata_t *module_data = st_dlist_get_data(node);

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

    node = st_dlist_get_head(modsmgr);

    while (node) {
        st_moddata_t *module_data = st_dlist_get_data(node);
        char         *modname;
        int           ret;

        node = st_dlist_get_next(node);

        if (!st_utl_strings_equal(module_data->subsystem, subsystem))
            continue;

        modname = dst[mod_index];
        if (!modname)
            break;

        /* False positive?  */
        #if defined(__GNUC__) && __GNUC__ >= 7
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-truncation"
        #endif
        ret = snprintf(modname, modname_size, "%s", module_data->name);
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
bool st_modsmgr_load_module(st_modsmgr_t *modsmgr,
 st_modinitfunc_t modinit_func, bool force) {
    st_moddata_t *module_data = modinit_func(modsmgr, ST_MODSMGR_FUNCS);

    printf("steroids: Trying to add module \"%s_%s\"\n", module_data->subsystem,
     module_data->name);

    if (!force && !st_modsmgr_module_have_deps(modsmgr, module_data))
        return false;

    return st_dlist_push_back(modsmgr, module_data);
}

st_modsmgr_t *st_modsmgr_init(void) {
    st_modsmgr_t *modsmgr = st_dlist_create(sizeof(st_moddata_t), NULL);

    if (!modsmgr)
        return NULL;

    printf("steroids: Searching internal modules...\n");
    for (size_t i = 0; i < ST_INTERNAL_MODULES_COUNT; i++) {
        st_moddata_t *module_data =
         st_internal_modules_entrypoints.modules_init_funcs[i](modsmgr,
          ST_MODSMGR_FUNCS);

        if (!module_data)
            continue;

        printf("steroids: Found module \"%s_%s\"\n", module_data->subsystem,
         module_data->name);

        st_dlist_push_back(modsmgr, module_data);
    }

    st_modsmgr_process_deps(modsmgr);

    return modsmgr;
}

void st_modsmgr_destroy(st_modsmgr_t *modsmgr) {
    st_dlist_destroy(modsmgr);
}

void *st_modsmgr_get_ctor(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name) {
    st_moddata_t *module_data = st_modsmgr_find_module(modsmgr, subsystem,
     module_name);

    return module_data ? module_data->ctor : NULL;
}