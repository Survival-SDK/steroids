#pragma once

#include "steroids/module.h"
#include "steroids/modules/gfxctx.h"

typedef st_modctx_t *(*st_glloader_init_t)(st_modctx_t *logger_ctx,
 st_gfxctx_t *gfxctx);
typedef void (*st_glloader_quit_t)(st_modctx_t *glloader_ctx);

typedef void *(*st_glloader_get_proc_address_t)(st_modctx_t *glloader_ctx,
 const char *funcname);

typedef struct {
    st_glloader_init_t             glloader_init;
    st_glloader_quit_t             glloader_quit;
    st_glloader_get_proc_address_t glloader_get_proc_address;
} st_glloader_funcs_t;
