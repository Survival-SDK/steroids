#include "sdl3.h"

#include <stdlib.h>

#include <SDL3/SDL.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define SDL3_FUNC_ENTRY(name) { #name, (void *)name }

static st_sdl3loaderctx_t *st_sdl3loader_init(const st_param_t params[]);
static void st_sdl3loader_quit(st_sdl3loaderctx_t *sdl3loader_ctx);

static void *st_sdl3loader_get_proc_address(
 const st_sdl3loaderctx_t *sdl3loader_ctx, const char *funcname);

static const struct {
    const char *name;
    void *ptr;
} sdl3_functions[] = {
    /* subsystem */
    SDL3_FUNC_ENTRY(SDL_InitSubSystem),
    SDL3_FUNC_ENTRY(SDL_QuitSubSystem),
    SDL3_FUNC_ENTRY(SDL_WasInit),

    /* events */
    SDL3_FUNC_ENTRY(SDL_PollEvent),

    /* video */
    SDL3_FUNC_ENTRY(SDL_GetCurrentVideoDriver),

    /* display */
    SDL3_FUNC_ENTRY(SDL_GetPrimaryDisplay),
    SDL3_FUNC_ENTRY(SDL_GetDisplays),
    SDL3_FUNC_ENTRY(SDL_GetDisplayName),
    SDL3_FUNC_ENTRY(SDL_GetDisplayBounds),
    SDL3_FUNC_ENTRY(SDL_GetDisplayProperties),

    /* window */
    SDL3_FUNC_ENTRY(SDL_CreateWindow),
    SDL3_FUNC_ENTRY(SDL_CreateWindowWithProperties),
    SDL3_FUNC_ENTRY(SDL_DestroyWindow),
    SDL3_FUNC_ENTRY(SDL_GetWindows),
    SDL3_FUNC_ENTRY(SDL_GetWindowProperties),
    SDL3_FUNC_ENTRY(SDL_GetWindowID),
    SDL3_FUNC_ENTRY(SDL_GetWindowFromID),
    SDL3_FUNC_ENTRY(SDL_GetDisplayForWindow),

    /* properties */
    SDL3_FUNC_ENTRY(SDL_CreateProperties),
    SDL3_FUNC_ENTRY(SDL_DestroyProperties),
    SDL3_FUNC_ENTRY(SDL_SetStringProperty),
    SDL3_FUNC_ENTRY(SDL_SetNumberProperty),
    SDL3_FUNC_ENTRY(SDL_SetBooleanProperty),
    SDL3_FUNC_ENTRY(SDL_SetPointerProperty),
    SDL3_FUNC_ENTRY(SDL_GetNumberProperty),
    SDL3_FUNC_ENTRY(SDL_GetPointerProperty),

    /* misc */
    SDL3_FUNC_ENTRY(SDL_SetHint),
    SDL3_FUNC_ENTRY(SDL_free),
    SDL3_FUNC_ENTRY(SDL_GetError),

    { NULL, NULL },
};

static st_sdl3loaderctx_funcs_t sdl3loaderctx_funcs = {
    ST_MODCTX_FUNCS,
    .get_proc_address = st_sdl3loader_get_proc_address,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_sdl3loader_sdl3_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("sdl3loader", "sdl3", ST_MODULE_TYPE, mod_prereqs,
     st_sdl3loader_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_sdl3loader_sdl3_init(modsmgr);
}
#endif

static st_sdl3loaderctx_t *st_sdl3loader_init(const st_param_t params[]) {
    st_modsmgr_t       *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t     *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_sdl3loaderctx_t *sdl3loader_ctx = (st_sdl3loaderctx_t *)st_modctx_new(
     "sdl3loader", "sdl3", sizeof(st_sdl3loaderctx_t), NULL, 
     &sdl3loaderctx_funcs, (st_object_dtor_t)st_sdl3loader_quit);

    if (!sdl3loader_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "sdl3loader_sdl3: unable to create new sdl3loader ctx object");

        return NULL;
    }

    sdl3loader_ctx->modsmgr    = modsmgr;
    sdl3loader_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "sdl3loader_sdl3: SDL3 functions loader initialized");

    return sdl3loader_ctx;
}

static void st_sdl3loader_quit(st_sdl3loaderctx_t *sdl3loader_ctx) {
    ST_LOGGERCTX_CALL(sdl3loader_ctx->logger_ctx, info,
     "sdl3loader_sdl3: SDL3 functions loader destroyed");

    free(sdl3loader_ctx);
}

static void *st_sdl3loader_get_proc_address(
 __attribute__((unused)) const st_sdl3loaderctx_t *sdl3loader_ctx,
 const char *funcname) {
    for (size_t i = 0; sdl3_functions[i].name != NULL; i++) {
        if (strcmp(sdl3_functions[i].name, funcname) == 0)
            return sdl3_functions[i].ptr;
    }
    
    return NULL;
}
