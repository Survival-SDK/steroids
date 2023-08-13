#ifndef ST_MODULES_GLLOADER_EGL_EGL_H
#define ST_MODULES_GLLOADER_EGL_EGL_H

#include "config.h" // IWYU pragma: keep
#include "steroids/glloader.h"
#include "steroids/types/modules/logger.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_glloader_egl_logger_t;

typedef struct {
    st_glloader_egl_logger_t logger;
} st_glloader_egl_t;

st_glloader_funcs_t st_glloader_egl_funcs = {
    .glloader_init             = st_glloader_init,
    .glloader_quit             = st_glloader_quit,
    .glloader_get_proc_address = st_glloader_get_proc_address,
};

st_modfuncentry_t st_module_glloader_egl_funcs[] = {
    {"init",             st_glloader_init},
    {"quit",             st_glloader_quit},
    {"get_proc_address", st_glloader_get_proc_address},
    {NULL, NULL},
};

#endif /* ST_MODULES_GLLOADER_EGL_EGL_H */
