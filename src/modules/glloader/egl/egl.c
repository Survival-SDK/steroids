#include "egl.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <EGL/egl.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_glloaderctx_t *st_glloader_init(const st_param_t params[]);
static void st_glloader_quit(st_glloaderctx_t *glloader_ctx);

static void *st_glloader_get_proc_address(st_glloaderctx_t *glloader_ctx,
 const char *funcname);

static st_glloaderctx_funcs_t glloaderctx_funcs = {
    st_modctx_funcs,
    .get_proc_address = st_glloader_get_proc_address,
};

static const st_modprerq_t mod_prereqs[] = {
    { "gfxctx", "egl", },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_glloader_egl_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("glloader", "egl", ST_MODULE_TYPE, mod_prereqs,
     st_glloader_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_glloader_egl_init(modsmgr);
}
#endif

static st_glloaderctx_t *st_glloader_init(const st_param_t params[]) {
    st_modsmgr_t     *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t   *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_gfxctx_t      *gfxctx = st_modctx_get_param_as_ptr(params, "gfxctx");
    st_glloaderctx_t *glloader_ctx = (st_glloaderctx_t *)st_modctx_new(
     "glloader", "egl", sizeof(st_glloaderctx_t), NULL, &glloaderctx_funcs,
     (st_object_dtor_t)st_glloader_quit);

    if (!glloader_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "glloader_egl: unable to create new glloader ctx object");

        return NULL;
    }

    glloader_ctx->modsmgr    = modsmgr;
    glloader_ctx->logger_ctx = logger_ctx;
    glloader_ctx->gfxctx     = gfxctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "glloader_egl: Open GL functions loader initialized");

    return glloader_ctx;
}

static void st_glloader_quit(st_glloaderctx_t *glloader_ctx) {
    ST_LOGGERCTX_CALL(glloader_ctx->logger_ctx, info,
     "glloader_egl: Open GL functions loader destroyed");
    free(glloader_ctx);
}

static void *st_glloader_get_proc_address(st_glloaderctx_t *glloader_ctx,
 const char *funcname) {
    ST_GFXCTX_CALL(glloader_ctx->gfxctx, make_current);

    return eglGetProcAddress(funcname);
}
