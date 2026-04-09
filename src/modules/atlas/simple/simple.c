#include "simple.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/atlloader.h"

#define LOADER_NAME_SIZE 32

static st_atlasctx_t *st_atlas_init(const st_param_t params[]);
static void st_atlas_quit(st_atlasctx_t *atlas_ctx);

static st_atlas_t *st_atlas_load(st_atlasctx_t *atlas_ctx, 
 const char *filename);
static st_atlas_t *st_atlas_memload(st_atlasctx_t *atlas_ctx, const void *data, 
 size_t size);
static st_atlas_t *st_atlas_create_empty(st_atlasctx_t *atlas_ctx, 
 const char *filename);

static bool st_atlas_add_subimage(st_atlas_t *atlas, const char *name, 
 unsigned x, unsigned y, unsigned width, unsigned height);
static const char *st_atlas_get_filename(const st_atlas_t *atlas);
static int st_atlas_get_subimages_count(const st_atlas_t *atlas);
static const char *st_atlas_get_subimage_name(const st_atlas_t *atlas, 
 unsigned index);
static unsigned st_atlas_get_subimage_x(const st_atlas_t *atlas, 
 unsigned index);
static unsigned st_atlas_get_subimage_y(const st_atlas_t *atlas, 
 unsigned index);
static unsigned st_atlas_get_subimage_width(const st_atlas_t *atlas, 
 unsigned index);
static unsigned st_atlas_get_subimage_height(const st_atlas_t *atlas, 
 unsigned index);

static st_atlasctx_funcs_t atlasctx_funcs = {
    ST_MODCTX_FUNCS,
    .load         = st_atlas_load,
    .memload      = st_atlas_memload,
    .create_empty = st_atlas_create_empty,
};

static st_atlas_funcs_t atlas_funcs = {
    ST_OBJECT_FUNCS,
    .add_subimage        = st_atlas_add_subimage,
    .get_filename        = st_atlas_get_filename,
    .get_subimages_count = st_atlas_get_subimages_count,
    .get_subimage_name   = st_atlas_get_subimage_name,
    .get_subimage_x      = st_atlas_get_subimage_x,
    .get_subimage_y      = st_atlas_get_subimage_y,
    .get_subimage_width  = st_atlas_get_subimage_width,
    .get_subimage_height = st_atlas_get_subimage_height,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "dynarr", NULL, },
    {0},
};

st_moddata_t *st_module_atlas_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("atlas", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_atlas_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_atlas_simple_init(modsmgr);
}
#endif

static void st_atlas_init_loaders(st_atlasctx_t *atlas_ctx) {
    char  loader_names[ST_ATL_LOADERS_MAX][LOADER_NAME_SIZE] = {0};
    char *ploader_names[ST_ATL_LOADERS_MAX];

    for (size_t i = 0; i < ST_ATL_LOADERS_MAX; i++)
        ploader_names[i] = loader_names[i];

    atlas_ctx->atl_loaders_count = 0;

    ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, info,
     "atlas_simple: Searching texture atlas loaders");

    ST_MODSMGR_CALL(atlas_ctx->modsmgr, get_module_names, ploader_names,
     ST_ATL_LOADERS_MAX, LOADER_NAME_SIZE, "atlloader");

    for (size_t i = 0; i < ST_ATL_LOADERS_MAX; i++) {
        st_ctx_ctor_t      ctx_ctor;
        st_atlloaderctx_t *ctx;
        char              *loader_name = ploader_names[i];

        if (!*loader_name)
            break;

        ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, info,
         "atlas_simple: Found module \"atlloader_%s\"", loader_name);

        ctx_ctor = ST_MODSMGR_CALL(atlas_ctx->modsmgr, get_ctor,
         "atlloader", loader_name);

        if (!ctx_ctor) {
            ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, error,
             "atlas_simple: Unable to get ctor from module \"atlloader_%s\"",
             loader_name);

            continue;
        }

        ctx = ctx_ctor((st_params_t){
            {"modsmgr", (uintptr_t)atlas_ctx->modsmgr},
            {"atlas_ctx", (uintptr_t)atlas_ctx},
        });
        if (!ctx)
            continue;

        atlas_ctx->atl_loaders[atlas_ctx->atl_loaders_count++] = ctx;
    }
}

static st_atlasctx_t *st_atlas_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_dynarrctx_t *dynarr_ctx = (st_dynarrctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "dynarr", NULL);
    st_atlasctx_t  *atlas_ctx = (st_atlasctx_t *)st_modctx_new("atlas",
     "simple", sizeof(st_atlasctx_t), NULL, &atlasctx_funcs,
     (st_object_dtor_t)st_atlas_quit);

    if (!atlas_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "atlas_simple: unable to create new atlas ctx object");

        return NULL;
    }

    atlas_ctx->modsmgr    = modsmgr;
    atlas_ctx->logger_ctx = logger_ctx;
    atlas_ctx->dynarr_ctx = dynarr_ctx;

    st_atlas_init_loaders(atlas_ctx);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "atlas_simple: Texture Atlas mgr initialized.");

    return atlas_ctx;
}

static void st_atlas_quit(st_atlasctx_t *atlas_ctx) {
    for (size_t i = 0; i < atlas_ctx->atl_loaders_count; i++)
        ST_ATLLOADERCTX_CALL(atlas_ctx->atl_loaders[i], destroy);

    ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, info,
     "atlas_simple: Texture Atlas mgr destroyed");
    free(atlas_ctx);
}

static st_atlas_t *st_atlas_load(st_atlasctx_t *atlas_ctx,
 const char *filename) {
    for (size_t i = 0; i < atlas_ctx->atl_loaders_count; i++) {
        st_atlas_t *atlas = ST_ATLLOADERCTX_CALL(atlas_ctx->atl_loaders[i], 
         load, filename);
        if (atlas)
            return atlas;
    }

    ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, error,
     "atlas_simple: No suitable loader for loading texture atlas \"%s\"", 
     filename);

    return NULL;
}

static st_atlas_t *st_atlas_memload(st_atlasctx_t *atlas_ctx,
 const void *data, size_t size) {
    for (size_t i = 0; i < atlas_ctx->atl_loaders_count; i++) {
        st_atlas_t *atlas = ST_ATLLOADERCTX_CALL(atlas_ctx->atl_loaders[i], 
         memload, data, size);
        if (atlas)
            return atlas;
    }

    ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, error,
     "atlas_simple: No suitable loader for loading texture atlas");

    return NULL;
}

static void st_atlas_free(st_atlas_t *atlas) {
    const st_atlsubimage_t *subimages = ST_DYNARR_CALL(atlas->subimages, 
     get_all);

    for (size_t i = 0; i < ST_DYNARR_CALL(atlas->subimages, get_elements_count); 
     i++)
        free(subimages[i].name);

    free(atlas->filename);
    ST_DYNARR_CALL(atlas->subimages, destroy);
    free(atlas);
}

static st_atlas_t *st_atlas_create_empty(st_atlasctx_t *atlas_ctx,
 const char *filename) {
    st_atlas_t *atlas;

    assert(filename);

    atlas = (st_atlas_t *)st_object_new(sizeof(st_atlas_t), &atlas_funcs, 
     (st_object_dtor_t)st_atlas_free, (st_object_t *)atlas_ctx);
    if (!atlas)
        return NULL;

    atlas->filename = strdup(filename);
    if (!atlas->filename) {
        ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, error,
         "atlas_simple: Unable to allocate memory for filename: %s",
         filename);

        goto namedup_fail;
    }
    atlas->subimages = ST_DYNARRCTX_CALL(atlas_ctx->dynarr_ctx, create,
     sizeof(st_atlsubimage_t), 4);
    if (!atlas->subimages) {
        ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, error,
         "atlas_simple: Unable to create dynarr for subimages of %s image", 
         filename);

        goto dynarr_create_fail;
    }

    return atlas;

dynarr_create_fail:
    free(atlas->filename);
namedup_fail:
    free(atlas);

    return NULL;
}

static bool st_atlas_add_subimage(st_atlas_t *atlas, const char *name, 
 unsigned x, unsigned y, unsigned width, unsigned height) {
    st_atlasctx_t *atlas_ctx = (st_atlasctx_t *)st_object_get_owner(atlas);
    st_atlsubimage_t subimage = {
        .name = strdup(name),
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };

    if (!subimage.name) {
        ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, error,
         "atlas_simple: Unable to allocate memory for subimage name \"%s\" of "
         "image \"%s\"", name, atlas->filename);

        return false;
    }

    if (!ST_DYNARR_CALL(atlas->subimages, append, &subimage)) {
        free(subimage.name);

        ST_LOGGERCTX_CALL(atlas_ctx->logger_ctx, error,
         "atlas_simple: Unable to add subimage \"%s\" of image \"%s\" to atlas",
         name, atlas->filename);

        return false;
    }

    return true;
}

static const char *st_atlas_get_filename(const st_atlas_t *atlas) {
    return atlas->filename;
}

static int st_atlas_get_subimages_count(const st_atlas_t *atlas) {
    return ST_DYNARR_CALL(atlas->subimages, get_elements_count);
}

static const char *st_atlas_get_subimage_name(const st_atlas_t *atlas, 
 unsigned index) {
    return (
     (st_atlsubimage_t *)ST_DYNARR_CALL(atlas->subimages, get, index))->name;
}

static unsigned st_atlas_get_subimage_x(const st_atlas_t *atlas, 
 unsigned index) {
    return (
     (st_atlsubimage_t *)ST_DYNARR_CALL(atlas->subimages, get, index))->x;
}

static unsigned st_atlas_get_subimage_y(const st_atlas_t *atlas, 
 unsigned index) {
    return (
     (st_atlsubimage_t *)ST_DYNARR_CALL(atlas->subimages, get, index))->y;
}

static unsigned st_atlas_get_subimage_width(const st_atlas_t *atlas, 
 unsigned index) {
    return (
     (st_atlsubimage_t *)ST_DYNARR_CALL(atlas->subimages, get, index))->width;
}

static unsigned st_atlas_get_subimage_height(const st_atlas_t *atlas, 
 unsigned index) {
    return (
     (st_atlsubimage_t *)ST_DYNARR_CALL(atlas->subimages, get, index))->height;
}
