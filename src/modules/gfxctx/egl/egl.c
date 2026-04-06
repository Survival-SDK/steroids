#include "egl.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ERRMSGBUF_SIZE    128
#define GAPI_STR_SIZE_MAX  32
#define RED_BITS            8
#define GREEN_BITS          8
#define BLUE_BITS           8
#define ALPHA_BITS          8
#define CFG_ATTRS_LEN      13
#define CTX_ATTRS_LEN       9

#ifdef _WIN32
    #define MINIMAL_OPENGL_MINOR 1
#elif __linux__
    #define MINIMAL_OPENGL_MINOR 2
#else
    #error Unknown target OS
#endif

typedef struct {
    EGLint red_size;
    EGLint green_size;
    EGLint blue_size;
    EGLint alpha_size;
    EGLint renderable_type;
} cfg_attrs_t;

typedef struct {
    EGLint context_major_version;
    EGLint context_minor_version;
    EGLint context_opengl_debug;
} ctx_attrs_t;

static st_gfxctxctx_t *st_gfxctx_init(const st_param_t params[]);
static void st_gfxctx_quit(st_gfxctxctx_t *gfxctx_ctx);

static st_gfxctx_t *st_gfxctx_create(st_gfxctxctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, st_gapi_t api);
static st_gfxctx_t *st_gfxctx_create_shared(st_gfxctxctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, st_gfxctx_t *other);

static bool st_gfxctx_make_current(st_gfxctx_t *gfxctx);
static bool st_gfxctx_swap_buffers(st_gfxctx_t *gfxctx);
static st_window_t *st_gfxctx_get_window(st_gfxctx_t *gfxctx);
static st_gapi_t st_gfxctx_get_api(const st_gfxctx_t *gfxctx);
static unsigned st_gfxctx_get_shared_index(const st_gfxctx_t *gfxctx);
static void st_gfxctx_destroy(st_gfxctx_t *gfxctx);
static bool st_gfxctx_debug_enabled(const st_gfxctx_t *gfxctx);
static void st_gfxctx_set_userdata(const st_gfxctx_t *gfxctx,
 const char *key, uintptr_t value);
static bool st_gfxctx_get_userdata(const st_gfxctx_t *gfxctx,
 uintptr_t *dst, const char *key);

static st_gfxctxctx_funcs_t gfxctxctx_funcs = {
    ST_MODCTX_FUNCS,
    .create        = st_gfxctx_create,
    .create_shared = st_gfxctx_create_shared,
};

static st_gfxctx_funcs_t gfxctx_funcs = {
    ST_OBJECT_FUNCS,
    .make_current     = st_gfxctx_make_current,
    .swap_buffers     = st_gfxctx_swap_buffers,
    .get_window       = st_gfxctx_get_window,
    .get_api          = st_gfxctx_get_api,
    .get_shared_index = st_gfxctx_get_shared_index,
    .debug_enabled    = st_gfxctx_debug_enabled,
    .set_userdata     = st_gfxctx_set_userdata,
    .get_userdata     = st_gfxctx_get_userdata,
};

static const st_modprerq_t mod_prereqs[] = {
    {"fnv1a", NULL},
    {"htable", NULL},
    {"logger", NULL},
    {"dpsrvconn", NULL},
    {0}
};

st_moddata_t *st_module_gfxctx_egl_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("gfxctx", "egl", ST_MODULE_TYPE, mod_prereqs,
     st_gfxctx_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_gfxctx_egl_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "gfxctx";
static const char *st_module_name = "egl";

static bool st_keyeqfunc(const void *left, const void *right) {
    return left && right && strcmp(left, right) == 0;
}

static st_gfxctxctx_t *st_gfxctx_init(const st_param_t params[]) {
    st_gfxctxctx_t    *gfxctx_ctx;
    st_modsmgr_t      *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    bool               software_opengl = st_modctx_get_param_as_bool(params, 
     "software");
    st_loggerctx_t    *logger_ctx;
    st_dpsrvconnctx_t *dpsrvconn_ctx;
    st_fnv1actx_t     *fnv1a_ctx;
    st_htablectx_t    *htable_ctx;
    char              *env_libgl_always_software;
    char              *env_gallium_driver;

    if (!modsmgr)
        return NULL;

    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    dpsrvconn_ctx = (st_dpsrvconnctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "dpsrvconn", NULL);
    if (!dpsrvconn_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get dpsrvconn context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    fnv1a_ctx = (st_fnv1actx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "fnv1a", NULL);
    if (!fnv1a_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get fnv1a context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    htable_ctx = (st_htablectx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton,
     "htable", NULL);
    if (!htable_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get htable context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    gfxctx_ctx = (st_gfxctxctx_t *)st_modctx_new("gfxctx", "egl",
     sizeof(st_gfxctxctx_t), NULL, &gfxctxctx_funcs,
     (st_object_dtor_t)st_gfxctx_quit);
    if (!gfxctx_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create new gfxctx ctx object", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    gfxctx_ctx->modsmgr = modsmgr;
    gfxctx_ctx->logger_ctx = logger_ctx;
    gfxctx_ctx->dpsrvconn_ctx = dpsrvconn_ctx;
    gfxctx_ctx->fnv1a_ctx = fnv1a_ctx;
    gfxctx_ctx->htable_ctx = htable_ctx;
    gfxctx_ctx->debug_enabled = false;
    gfxctx_ctx->must_quit = false;
    gfxctx_ctx->gfxctxs_count = 0;
    gfxctx_ctx->software_opengl = software_opengl;

    if (software_opengl) {
        env_libgl_always_software = getenv("LIBGL_ALWAYS_SOFTWARE");
        env_gallium_driver = getenv("GALLIUM_DRIVER");

        gfxctx_ctx->old_envs.libgl_always_software.was_set = 
        !!env_libgl_always_software;
        gfxctx_ctx->old_envs.gallium_driver.was_set = !!env_gallium_driver;

        strncpy(gfxctx_ctx->old_envs.libgl_always_software.value, 
        env_libgl_always_software ? env_libgl_always_software : "", 
        OLD_ENVS_SIZE_MAX);
        strncpy(gfxctx_ctx->old_envs.gallium_driver.value, 
        env_gallium_driver ? env_gallium_driver : "", OLD_ENVS_SIZE_MAX);

        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
        setenv("GALLIUM_DRIVER", "llvmpipe", 1);
    }

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, info,
     "%s_%s: Graphics context mgr initialized", st_module_subsystem,
     st_module_name);

    return gfxctx_ctx;
}

static void st_gfxctx_quit(st_gfxctxctx_t *gfxctx_ctx) {
    if (gfxctx_ctx->gfxctxs_count > 0) {
        gfxctx_ctx->must_quit = true;

        return;
    }

    if (gfxctx_ctx->software_opengl) {
        if (gfxctx_ctx->old_envs.libgl_always_software.was_set)
            setenv("LIBGL_ALWAYS_SOFTWARE", 
            gfxctx_ctx->old_envs.libgl_always_software.value, 1);
        else
            unsetenv("LIBGL_ALWAYS_SOFTWARE");

        if (gfxctx_ctx->old_envs.gallium_driver.was_set)
            setenv("GALLIUM_DRIVER", gfxctx_ctx->old_envs.gallium_driver.value, 1);
        else
            unsetenv("GALLIUM_DRIVER");
    }

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, info,
     "%s_%s: Graphics context mgr destroyed", st_module_subsystem,
     st_module_name);

    free(gfxctx_ctx);
}

static EGLenum getegl_api_by_gapi(st_gapi_t api) {
    if (api >= ST_GAPI_GL11 && api <= ST_GAPI_GL46)
        return EGL_OPENGL_API;
    if (api >= ST_GAPI_ES1 && api <= ST_GAPI_ES32)
        return  EGL_OPENGL_ES_API;

    return (EGLenum)-1;
}

static EGLint get_renderable_type_by_gapi(st_gapi_t api) {
    if (api >= ST_GAPI_GL11 && api <= ST_GAPI_GL46)
        return EGL_OPENGL_BIT;
    if (api == ST_GAPI_ES1 || api == ST_GAPI_ES11)
        return EGL_OPENGL_ES_BIT;
    if (api == ST_GAPI_ES2)
        return EGL_OPENGL_ES2_BIT;
    if (api >= ST_GAPI_ES3 && api < ST_GAPI_ES32)
        return EGL_OPENGL_ES3_BIT_KHR;

    return (EGLint)-1;
}

static EGLint get_major_version_by_gapi(st_gapi_t api) {
    if ((api >= ST_GAPI_GL11 && api <= ST_GAPI_GL15) ||
     api == ST_GAPI_ES1 || api == ST_GAPI_ES11)
        return 1;
    if (api == ST_GAPI_GL2 || api == ST_GAPI_GL21 || api == ST_GAPI_ES2)
        return 2;
    if ((api >= ST_GAPI_GL3 && api <= ST_GAPI_GL33) ||
     (api >= ST_GAPI_ES3 && api <= ST_GAPI_ES32))
        return 3;
    if (api >= ST_GAPI_GL4 && api <= ST_GAPI_GL46)
        return 4;

    return (EGLint)-1;
}

static EGLint get_minor_version_by_gapi(st_gapi_t api) {
    switch (api) {
        case ST_GAPI_GL1:
        case ST_GAPI_GL2:
        case ST_GAPI_GL3:
        case ST_GAPI_GL4:
        case ST_GAPI_ES1:
        case ST_GAPI_ES2:
        case ST_GAPI_ES3:
            return 0;
        case ST_GAPI_GL11:
        case ST_GAPI_GL21:
        case ST_GAPI_GL31:
        case ST_GAPI_GL41:
        case ST_GAPI_ES11:
        case ST_GAPI_ES31:
            return 1;
        case ST_GAPI_GL12:
        case ST_GAPI_GL32:
        case ST_GAPI_GL42:
        case ST_GAPI_ES32:
            return 2;
        case ST_GAPI_GL13:
        case ST_GAPI_GL33:
        case ST_GAPI_GL43:
            return 3;
        case ST_GAPI_GL14:
        case ST_GAPI_GL44:
            return 4;
        case ST_GAPI_GL15:
        case ST_GAPI_GL45:
            return 5; // NOLINT(readability-magic-numbers)
        case ST_GAPI_GL46:
            return 6; // NOLINT(readability-magic-numbers)
        case ST_GAPI_MAX:
        default:
            break;
    }

    return (EGLint)-1;
}

const char *get_egl_error_str(EGLint errorcode) {
    switch (errorcode) {
        case EGL_SUCCESS:
            return "The last function succeeded without error";
        case EGL_NOT_INITIALIZED:
            return "EGL is not initialized, or could not be initialized, for "
                   "the specified EGL display connection";
        case EGL_BAD_ACCESS:
            return "EGL cannot access a requested resource (for example a "
                   "context is bound in another thread)";
        case EGL_BAD_ALLOC:
            return "EGL failed to allocate resources for the requested "
                   "operation";
        case EGL_BAD_ATTRIBUTE:
            return "An unrecognized attribute or attribute value was passed in "
                   "the attribute list";
        case EGL_BAD_CONTEXT:
            return "An EGLContext argument does not name a valid EGL rendering "
                   "context";
        case EGL_BAD_CONFIG:
            return "An EGLConfig argument does not name a valid EGL frame "
                   "buffer configuration";
        case EGL_BAD_CURRENT_SURFACE:
            return "The current surface of the calling thread is a window, "
                   "pixel buffer or pixmap that is no longer valid";
        case EGL_BAD_DISPLAY:
            return "An EGLDisplay argument does not name a valid EGL display "
                   "connection";
        case EGL_BAD_SURFACE:
            return "An EGLSurface argument does not name a valid surface "
                   "(window, pixel buffer or pixmap) configured for GL "
                   "rendering";
        case EGL_BAD_MATCH:
            return "Arguments are inconsistent (for example, a valid context "
                   "requires buffers not supplied by a valid surface)";
        case EGL_BAD_PARAMETER:
            return "One or more argument values are invalid";
        case EGL_BAD_NATIVE_PIXMAP:
            return "A NativePixmapType argument does not refer to a valid "
                   "native pixmap";
        case EGL_BAD_NATIVE_WINDOW:
            return "A NativeWindowType argument does not refer to a valid "
                   "native window";
        case EGL_CONTEXT_LOST:
            return "A power management event has occurred. The application "
                   "must destroy all contexts and reinitialise OpenGL ES state "
                   "and objects to continue rendering";
        default:
            break;
    }

    return "Unknown error";
}

static bool extension_supported(EGLDisplay display, const char *ext) {
    const char *extensions = eglQueryString(display, EGL_EXTENSIONS);

    return extensions && strstr(extensions, ext);
}

static void process_attrs(cfg_attrs_t *cfg_arrts, ctx_attrs_t *ctx_attrs,
 bool *version_changed, bool *debug_changed, EGLint egl_version_minor,
 bool have_ctx_extension) {
    *version_changed = false;

    switch (egl_version_minor) {
        case 0:
        case 1:
            if (cfg_arrts->renderable_type != EGL_OPENGL_ES_BIT ||
             ctx_attrs->context_major_version != 1 ||
             ctx_attrs->context_minor_version != 0) {
                *version_changed = true;
                cfg_arrts->renderable_type = EGL_OPENGL_ES_BIT;
                ctx_attrs->context_major_version = 1;
                ctx_attrs->context_minor_version = 0;
            }
            break;
        case 2:
            if (cfg_arrts->renderable_type == EGL_OPENGL_ES_BIT &&
             (ctx_attrs->context_major_version != 1 ||
             ctx_attrs->context_minor_version != 0)) {
                *version_changed = true;
                ctx_attrs->context_major_version = 1;
                ctx_attrs->context_minor_version = 0;
            } else if (cfg_arrts->renderable_type != EGL_OPENGL_ES2_BIT) {
                *version_changed = true;
                cfg_arrts->renderable_type = EGL_OPENGL_ES2_BIT;
                ctx_attrs->context_major_version = 2;
                ctx_attrs->context_minor_version = 0;
            }
            break;
        case 4:
            if ((cfg_arrts->renderable_type == EGL_OPENGL_ES_BIT ||
             cfg_arrts->renderable_type == EGL_OPENGL_BIT) &&
             (!have_ctx_extension && ctx_attrs->context_minor_version != 0)) {
                *version_changed = true;
                ctx_attrs->context_minor_version = 0;
            } else if (cfg_arrts->renderable_type == EGL_OPENGL_ES3_BIT_KHR &&
             !have_ctx_extension) {
                cfg_arrts->renderable_type = EGL_OPENGL_ES2_BIT;
                ctx_attrs->context_major_version = 2;
                ctx_attrs->context_minor_version = 0;
            }
            break;
        case 5: // NOLINT(readability-magic-numbers)
            if (cfg_arrts->renderable_type == EGL_OPENGL_ES3_BIT_KHR)
                cfg_arrts->renderable_type = EGL_OPENGL_ES3_BIT;
        default:
            break;
    }

    if (ctx_attrs->context_opengl_debug
     && ((egl_version_minor < 4)
      || ((egl_version_minor == 4) && !have_ctx_extension))) {
        ctx_attrs->context_opengl_debug = EGL_FALSE;
        *debug_changed = true;
    }
}

static void fill_cfg_attrs(EGLint dst[CFG_ATTRS_LEN], cfg_attrs_t *attrs,
 EGLint egl_version_minor) {
    assert(CFG_ATTRS_LEN == 13);

    dst[0] = EGL_RED_SIZE;
    dst[1] = attrs->red_size;

    dst[2] = EGL_GREEN_SIZE;
    dst[3] = attrs->green_size;

    dst[4] = EGL_BLUE_SIZE;
    dst[5] = attrs->blue_size;

    dst[6] = EGL_ALPHA_SIZE;
    dst[7] = attrs->alpha_size;

    dst[8] = EGL_CONFIG_CAVEAT;
    dst[9] = EGL_NONE;

    dst[10] = EGL_RENDERABLE_TYPE;
    dst[11] = attrs->renderable_type;

    dst[12] = EGL_NONE;
}

static void fill_ctx_attrs(EGLint dst[CTX_ATTRS_LEN], ctx_attrs_t *attrs,
 EGLint egl_version_minor, bool have_ctx_extension) {
    assert(CTX_ATTRS_LEN == 9);

    if (attrs->context_major_version == EGL_NONE) {
        dst[0] = EGL_NONE;
    } else {
        if (egl_version_minor < 4 ||
         (egl_version_minor == 4 && !have_ctx_extension)) {
            dst[0] = EGL_CONTEXT_CLIENT_VERSION;
            dst[1] = attrs->context_major_version;

            dst[2] = EGL_NONE;
        } else if (egl_version_minor == 4 && have_ctx_extension) {
            dst[0] = EGL_CONTEXT_MAJOR_VERSION_KHR;
            dst[1] = attrs->context_major_version;

            dst[2] = EGL_CONTEXT_MINOR_VERSION_KHR;
            dst[3] = attrs->context_minor_version;

            dst[4] = EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR;
            dst[5] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;

            dst[6] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
            dst[7] = attrs->context_opengl_debug;

            dst[8] = EGL_NONE;
        } else {
            dst[0] = EGL_CONTEXT_MAJOR_VERSION;
            dst[1] = attrs->context_major_version;

            dst[2] = EGL_CONTEXT_MINOR_VERSION;
            dst[3] = attrs->context_minor_version;

            dst[4] = EGL_CONTEXT_OPENGL_PROFILE_MASK;
            dst[5] = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;

            dst[6] = EGL_CONTEXT_OPENGL_DEBUG;
            dst[7] = attrs->context_opengl_debug;

            dst[8] = EGL_NONE;
        }
    }
}

static void attrs_fill_gapi_str(char dst[GAPI_STR_SIZE_MAX],
 cfg_attrs_t *cfg_arrts, ctx_attrs_t *ctx_attrs) {
    const char *api_name;

    switch (cfg_arrts->renderable_type) {
        case EGL_OPENGL_ES_BIT:
        case EGL_OPENGL_ES2_BIT:
        case EGL_OPENGL_ES3_BIT:
        #if EGL_OPENGL_ES3_BIT_KHR != EGL_OPENGL_ES3_BIT
        case EGL_OPENGL_ES3_BIT_KHR:
        #endif
            api_name = "OpenGL ES";
            break;
        case EGL_OPENGL_BIT:
            api_name = "OpenGL";
            break;
        default:
            api_name = "Unknown";
            break;
    }

    snprintf(dst, GAPI_STR_SIZE_MAX, "%s %i.%i", api_name,
     ctx_attrs->context_major_version, ctx_attrs->context_minor_version);
}

static int gapi_from_attrs(cfg_attrs_t *cfg_arrts,
 ctx_attrs_t *ctx_attrs) {
    switch (cfg_arrts->renderable_type) {
        case EGL_OPENGL_ES_BIT:
        case EGL_OPENGL_ES2_BIT:
        case EGL_OPENGL_ES3_BIT:
        #if EGL_OPENGL_ES3_BIT_KHR != EGL_OPENGL_ES3_BIT
        case EGL_OPENGL_ES3_BIT_KHR:
        #endif
            switch (ctx_attrs->context_major_version) {
                case 1:
                    return (const int[]){
                        ST_GAPI_ES1,
                        ST_GAPI_ES11,
                    }[ctx_attrs->context_minor_version];
                case 2:
                    return ST_GAPI_ES2;
                case 3:
                    return (const int[]){
                        ST_GAPI_ES3,
                        ST_GAPI_ES31,
                        ST_GAPI_ES32,
                    }[ctx_attrs->context_minor_version];
                default:
                    return ST_GAPI_UNKNOWN;
            }
        case EGL_OPENGL_BIT:
            switch (ctx_attrs->context_major_version) {
                case 1:
                    return (const int[]){
                        ST_GAPI_UNKNOWN,
                        ST_GAPI_GL11,
                        ST_GAPI_GL12,
                        ST_GAPI_GL13,
                        ST_GAPI_GL14,
                        ST_GAPI_GL15,
                    }[ctx_attrs->context_minor_version];
                case 2:
                    return (const int[]){
                        ST_GAPI_GL2,
                        ST_GAPI_GL21,
                    }[ctx_attrs->context_minor_version];
                case 3:
                    return (const int[]){
                        ST_GAPI_GL3,
                        ST_GAPI_GL31,
                        ST_GAPI_GL32,
                        ST_GAPI_GL33,
                    }[ctx_attrs->context_minor_version];
                case 4:
                    return (const int[]){
                        ST_GAPI_GL4,
                        ST_GAPI_GL41,
                        ST_GAPI_GL42,
                        ST_GAPI_GL43,
                        ST_GAPI_GL44,
                        ST_GAPI_GL45,
                        ST_GAPI_GL46,
                    }[ctx_attrs->context_minor_version];
                default:
                    return ST_GAPI_UNKNOWN;
            }
        default:
            return ST_GAPI_UNKNOWN;
    }
}

static unsigned st_shared_data_get_free_index(const st_dlist_t *shared_data) {
    unsigned     free_index = 0;
    st_dlnode_t *node = st_dlist_get_head(shared_data);

    while (node) {
        st_gfxctx_shared_data_t *data = st_dlist_get_data(node);
        if (data->index == free_index) {
            free_index++;
            node = st_dlist_get_head(shared_data);

            continue;
        }

        node = st_dlist_get_next(node);
    }

    return free_index;
}

static void st_debug_callback(__attribute__((unused)) EGLenum error,
 const char *command, EGLint message_type, EGLLabelKHR thread_label,
 __attribute__((unused)) EGLLabelKHR object_label, const char* message) {
    st_gfxctxctx_t *gfxctx_ctx = thread_label;

    switch (message_type) {
        case EGL_DEBUG_MSG_CRITICAL_KHR:
        case EGL_DEBUG_MSG_ERROR_KHR:
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
             "%s_%s: \"%s\": %s", st_module_subsystem, st_module_name,
             command, message);
            break;
        case EGL_DEBUG_MSG_WARN_KHR:
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
             "%s_%s: \"%s\": %s", st_module_subsystem, st_module_name,
             command, message);
            break;
        case EGL_DEBUG_MSG_INFO_KHR:
        default:
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, info,
             "%s_%s: \"%s\": %s", st_module_subsystem, st_module_name,
             command, message);
            break;
    }
}

static void st_try_to_enable_debug(st_gfxctx_t *gfxctx,
 st_gfxctxctx_t *gfxctx_ctx) {
    EGLint ret;

    if (gfxctx_ctx->debug_enabled)
        return;

    if (!extension_supported(gfxctx->display, "EGL_KHR_debug"))
        return;

    gfxctx_ctx->egl_debug_message_control_khr = (void *)eglGetProcAddress(
     "eglDebugMessageControlKHR");
    if (!gfxctx_ctx->egl_debug_message_control_khr)
        return;

    gfxctx_ctx->egl_label_object_khr = (void *)eglGetProcAddress(
     "eglLabelObjectKHR");
    if (!gfxctx_ctx->egl_label_object_khr)
        return;

    ret = gfxctx_ctx->egl_debug_message_control_khr(st_debug_callback,
     (EGLAttrib[]){
        EGL_DEBUG_MSG_INFO_KHR,     EGL_TRUE,
        EGL_DEBUG_MSG_WARN_KHR,     EGL_TRUE,
        EGL_DEBUG_MSG_ERROR_KHR,    EGL_TRUE,
        EGL_DEBUG_MSG_CRITICAL_KHR, EGL_TRUE,
        EGL_NONE
    });

    if (ret != EGL_SUCCESS)
        return;

    ret = gfxctx_ctx->egl_label_object_khr(NULL, EGL_OBJECT_THREAD_KHR, NULL,
     gfxctx_ctx);

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, info,
     "%s_%s: EGL debug enabled", st_module_subsystem, st_module_name);

    gfxctx_ctx->debug_enabled = true;
}

static EGLDisplay st_get_egl_display(EGLNativeDisplayType native_display) {
    EGLDisplay display = eglGetDisplay(native_display);
    if (display != EGL_NO_DISPLAY)
        return display;

    #if defined(__linux__)
        #if defined(PFNEGLGETPLATFORMDISPLAYPROC)
            PFNEGLGETPLATFORMDISPLAYPROC platform_display =
             (PFNEGLGETPLATFORMDISPLAYPROC)eglGetProcAddress(
              "eglGetPlatformDisplay");
        #endif
        PFNEGLGETPLATFORMDISPLAYEXTPROC platform_display_ext =
         (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress(
          "eglGetPlatformDisplayEXT");

        if (
        #if defined(PFNEGLGETPLATFORMDISPLAYPROC)
            platform_display ||
        #endif
        platform_display_ext) {
            display = EGL_NO_DISPLAY;

            #ifdef EGL_PLATFORM_WAYLAND_KHR
                #if defined(PFNEGLGETPLATFORMDISPLAYPROC)
                    display = platform_display
                        ? platform_display(EGL_PLATFORM_WAYLAND_KHR, 
                         native_display, NULL)
                        : platform_display_ext(EGL_PLATFORM_WAYLAND_KHR, 
                         native_display, NULL);
                #else
                    display = platform_display_ext(EGL_PLATFORM_WAYLAND_KHR,
                     native_display, NULL);
                #endif
                if (display != EGL_NO_DISPLAY)
                    return display;
            #endif
            #ifdef EGL_PLATFORM_X11_KHR
                #if defined(PFNEGLGETPLATFORMDISPLAYPROC)
                    display = platform_display
                        ? platform_display(EGL_PLATFORM_X11_KHR, native_display, 
                         NULL)
                        : platform_display_ext(EGL_PLATFORM_X11_KHR, 
                         native_display, NULL);
                #else
                    display = platform_display_ext(EGL_PLATFORM_X11_KHR, 
                     native_display, NULL);
                #endif
                if (display != EGL_NO_DISPLAY)
                    return display;
            #endif
        }
    #endif

    return display;
}

static st_gfxctx_t *st_gfxctx_create_impl(st_gfxctxctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, EGLint renderable_type,
 EGLint major, EGLint minor, st_gfxctx_t *shared) {
    cfg_attrs_t cfg_attrs = {
        .red_size        = RED_BITS,
        .green_size      = GREEN_BITS,
        .blue_size       = BLUE_BITS,
        .alpha_size      = ALPHA_BITS,
        .renderable_type = renderable_type,
    };
    ctx_attrs_t ctx_attrs = {
        .context_major_version = major,
        .context_minor_version = minor,
        .context_opengl_debug  = EGL_TRUE,
    };
    EGLint           egl_cfg_attrs[CFG_ATTRS_LEN] = {0};
    EGLint           egl_ctx_attrs[CTX_ATTRS_LEN] = {0};
    EGLint           configs_count = 0;
    st_gfxctx_t     *gfxctx;
    bool             egl_khr_create_context_supported = false;
    EGLint           egl_version_major = 0;
    EGLint           egl_version_minor = 0;
    bool             version_changed = false;
    bool             debug_changed = false;
    EGLNativeDisplayType native_display;
    EGLNativeWindowType  native_window;

    gfxctx = (st_gfxctx_t *)st_object_new(sizeof(st_gfxctx_t), &gfxctx_funcs,
     (st_object_dtor_t)st_gfxctx_destroy, (st_object_t *)gfxctx_ctx);
    if (!gfxctx) {
        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
         "%s_%s: Unable to allocate memory for gfx context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    gfxctx->userdata = ST_HTABLECTX_CALL(gfxctx_ctx->htable_ctx, create,
     (unsigned int (*)(const void *))ST_FNV1ACTX_CALL(gfxctx_ctx->fnv1a_ctx,
      get_u32hashstr_func),
     st_keyeqfunc, free, NULL);
    if (!gfxctx->userdata)
        goto udata_fail;

    native_display = (EGLNativeDisplayType)ST_MONITOR_CALL(
     monitor, get_native_device_handle);
    native_window = (EGLNativeWindowType)ST_WINDOW_CALL(window,
     get_native_handle);

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: Creating EGL context (native display: %p, native window: %p, "
     "requested gapi: %i.%i)", st_module_subsystem, st_module_name,
     (void *)native_display, (void *)native_window, major, minor);

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: Calling st_get_egl_display()", st_module_subsystem,
     st_module_name);
    gfxctx->display = st_get_egl_display(native_display);
    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: st_get_egl_display() returned %p", st_module_subsystem,
     st_module_name, (void *)gfxctx->display);

    if (gfxctx->display == EGL_NO_DISPLAY) {
        EGLint egl_error = eglGetError();

        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
         "%s_%s: Unable to get EGL display: %s (0x%x)", st_module_subsystem,
         st_module_name, get_egl_error_str(egl_error), egl_error);

        goto get_display_fail;
    }

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: Calling eglInitialize()", st_module_subsystem, st_module_name);
    if (eglInitialize(gfxctx->display, &egl_version_major, &egl_version_minor)
     == EGL_FALSE) {
        if (!gfxctx_ctx->debug_enabled)
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
             "%s_%s: Unable to initialize EGL: %s", st_module_subsystem,
             st_module_name, get_egl_error_str(eglGetError()));

        goto egl_init_fail;
    }
    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: eglInitialize() done, EGL version: %i.%i", st_module_subsystem,
     st_module_name, egl_version_major, egl_version_minor);

    if (egl_version_minor < 5
     && renderable_type == EGL_OPENGL_BIT
     && ((major == 1 && minor > MINIMAL_OPENGL_MINOR) || major > 1)
     && !shared) {
        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
             "%s_%s: EGL 1.%i doesn't able to export core OpenGL %i.%i "
             "functions. Try to use another gfxctx module if available",
             st_module_subsystem, st_module_name, egl_version_minor, major, minor);
    }

    st_try_to_enable_debug(gfxctx, gfxctx_ctx);

    if (gfxctx_ctx->debug_enabled)
        gfxctx_ctx->egl_label_object_khr(gfxctx->display, EGL_OBJECT_DISPLAY_KHR,
         gfxctx->display, monitor);

    egl_khr_create_context_supported = extension_supported(gfxctx->display,
     "EGL_KHR_create_context");

    process_attrs(&cfg_attrs, &ctx_attrs, &version_changed, &debug_changed,
     egl_version_minor, egl_khr_create_context_supported);

    if (version_changed && !shared) {
        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
         "%s_%s: Unable to create context with required version. Possible "
         "reasons: required version of context or minor version of context may "
         "be not supported by current version of EGL or EGL_KHR_create_context "
         "extension is not present", st_module_subsystem, st_module_name);
    }

    if (debug_changed && !shared) {
        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
         "%s_%s: Debug context is not supported. Possible "
         "reasons: debug context is not supported by current version of EGL or "
         "EGL_KHR_create_context extension is not present", st_module_subsystem,
         st_module_name);
    }

    gfxctx->debug = !debug_changed;

    if ((version_changed || debug_changed) && !shared) {
        char gapi_str[GAPI_STR_SIZE_MAX];

        attrs_fill_gapi_str(gapi_str, &cfg_attrs, &ctx_attrs);

        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
         "%s_%s: Current version of EGL: 1.%i", st_module_subsystem,
         st_module_name, egl_version_minor);
        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
         "%s_%s: Fallback context created: %s", st_module_subsystem,
         st_module_name, gapi_str);
    }

    fill_cfg_attrs(egl_cfg_attrs, &cfg_attrs, egl_version_minor);
    fill_ctx_attrs(egl_ctx_attrs, &ctx_attrs, egl_version_minor,
     egl_khr_create_context_supported);

    if (shared)
        gfxctx->gapi = shared->gapi;
    else
        gfxctx->gapi = gapi_from_attrs(&cfg_attrs, &ctx_attrs);

    if (eglChooseConfig(gfxctx->display, egl_cfg_attrs, &gfxctx->cfg, 1,
     &configs_count) == EGL_FALSE || configs_count != 1) {
        if (!gfxctx_ctx->debug_enabled)
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
             "%s_%s: Unable to get matching frame buffer configuration: %s",
             st_module_subsystem, st_module_name,
             get_egl_error_str(eglGetError()));

        goto choose_config_fail;
    }

    if (eglBindAPI(getegl_api_by_gapi((st_gapi_t)gfxctx->gapi)) == EGL_FALSE) {
        if (!gfxctx_ctx->debug_enabled)
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
             "%s_%s: Unable to bind EGL API: %s", st_module_subsystem,
             st_module_name, get_egl_error_str(eglGetError()));

        goto bind_api_fail;
    }

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: Calling eglCreateWindowSurface() (native window: %p)",
     st_module_subsystem, st_module_name, (void *)native_window);
    gfxctx->surface = eglCreateWindowSurface(gfxctx->display,
     gfxctx->cfg, native_window, NULL);
    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: eglCreateWindowSurface() returned %p", st_module_subsystem,
     st_module_name, (void *)gfxctx->surface);
    if (gfxctx->surface == EGL_NO_SURFACE) {
        if (!gfxctx_ctx->debug_enabled)
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
             "%s_%s: Unable to create EGL window surface: %s", st_module_subsystem,
             st_module_name, get_egl_error_str(eglGetError()));

        goto create_surface_fail;
    }

    if (gfxctx_ctx->debug_enabled)
        gfxctx_ctx->egl_label_object_khr(gfxctx->display, EGL_OBJECT_DISPLAY_KHR,
         gfxctx->surface, window);

    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: Calling eglCreateContext()", st_module_subsystem, st_module_name);
    gfxctx->handle = eglCreateContext(gfxctx->display, gfxctx->cfg,
     shared ? shared->handle : EGL_NO_CONTEXT, egl_ctx_attrs);
    ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, debug,
     "%s_%s: eglCreateContext() returned %p", st_module_subsystem,
     st_module_name, (void *)gfxctx->handle);
    if (gfxctx->handle == EGL_NO_CONTEXT) {
        if (!gfxctx_ctx->debug_enabled)
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
             "%s_%s: Unable to create EGL render context: %s", st_module_subsystem,
             st_module_name, get_egl_error_str(eglGetError()));

        goto create_context_fail;
    }

    if (gfxctx_ctx->debug_enabled)
        gfxctx_ctx->egl_label_object_khr(gfxctx->display, EGL_OBJECT_CONTEXT_KHR,
         gfxctx->handle, gfxctx);

    gfxctx->window = window;
    if (!shared) {
        gfxctx->shared_data = st_dlist_create(
         sizeof(st_gfxctx_shared_data_t *), NULL);
        if (gfxctx->shared_data) {
            st_gfxctx_shared_data_t *data = malloc(
             sizeof(st_gfxctx_shared_data_t));
            if (data) {
                data->ctx = gfxctx;
                data->index = 0;
                st_dlist_push_back(gfxctx->shared_data, &data);
            }
        } else {
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
             "%s_%s: Unable to create structure for shared contexts data. "
             "This context will not able to be shared", st_module_subsystem,
             st_module_name);
        }
    } else {
        gfxctx->shared_data = shared->shared_data;
        st_gfxctx_shared_data_t *data = malloc(sizeof(st_gfxctx_shared_data_t));
        if (data) {
            data->ctx = gfxctx;
            data->index = st_shared_data_get_free_index(gfxctx->shared_data);
            st_dlist_push_back(gfxctx->shared_data, &data);
        }
    }

    if (!shared)
        eglMakeCurrent(gfxctx->display, gfxctx->surface, gfxctx->surface,
         gfxctx->handle);
    if (eglSwapInterval(gfxctx->display, 1) == EGL_FALSE) {
        if (!gfxctx_ctx->debug_enabled)
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, warning,
             "%s_%s: Unable to set swap interval: %s", st_module_subsystem,
             st_module_name, get_egl_error_str(eglGetError()));
    }

    gfxctx_ctx->gfxctxs_count++;

    return gfxctx;

create_context_fail:
    eglDestroySurface(gfxctx->display, gfxctx->surface);
bind_api_fail:
create_surface_fail:
choose_config_fail:
    eglTerminate(gfxctx->display);
egl_init_fail:
get_display_fail:
    ST_HTABLE_CALL(gfxctx->userdata, destroy);
udata_fail:
    free(gfxctx);

    return NULL;
}

static st_gfxctx_t *st_gfxctx_create(st_gfxctxctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, st_gapi_t api) {
    if (api < ST_GAPI_GL11 || api > ST_GAPI_ES32) {
        ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
         "%s_%s: Unsupported gfx API", st_module_subsystem, st_module_name);

        return NULL;
    }

    return st_gfxctx_create_impl(gfxctx_ctx, monitor, window,
     get_renderable_type_by_gapi(api), get_major_version_by_gapi(api),
     get_minor_version_by_gapi(api), NULL);
}

static st_gfxctx_t *st_gfxctx_create_shared(st_gfxctxctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, st_gfxctx_t *other) {
    if (!monitor || !window || !other || !other->shared_data)
        return NULL;

    return st_gfxctx_create_impl(gfxctx_ctx, monitor, window,
     get_renderable_type_by_gapi((st_gapi_t)other->gapi),
     get_major_version_by_gapi((st_gapi_t)other->gapi),
     get_minor_version_by_gapi((st_gapi_t)other->gapi), other);
}

static bool st_gfxctx_make_current(st_gfxctx_t *gfxctx) {
    return eglMakeCurrent(gfxctx->display, gfxctx->surface, gfxctx->surface,
     gfxctx->handle);
}

static bool st_gfxctx_swap_buffers(st_gfxctx_t *gfxctx) {
    return eglSwapBuffers(gfxctx->display, gfxctx->surface);
}

static st_window_t *st_gfxctx_get_window(st_gfxctx_t *gfxctx) {
    return gfxctx->window;
}

static st_gapi_t st_gfxctx_get_api(const st_gfxctx_t *gfxctx) {
    return (st_gapi_t)gfxctx->gapi;
}

static unsigned st_gfxctx_get_shared_index(const st_gfxctx_t *gfxctx) {
    st_dlnode_t *node = st_dlist_get_head(gfxctx->shared_data);

    while (node) {
        st_gfxctx_shared_data_t *data = st_dlist_get_data(node);

        if (data->ctx == gfxctx)
            return data->index;

        node = st_dlist_get_next(node);
    }

    assert(false && "missing shared data for gfxctx");
    return 0;
}

static void st_gfxctx_destroy(st_gfxctx_t *gfxctx) {
    st_gfxctxctx_t *gfxctx_ctx = (st_gfxctxctx_t *)st_object_get_owner(
     (st_object_t *)gfxctx);

    if (eglGetCurrentContext() == gfxctx->handle)
        eglMakeCurrent(gfxctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE,
         EGL_NO_CONTEXT);
    eglDestroyContext(gfxctx->display, gfxctx->handle);
    eglDestroySurface(gfxctx->display, gfxctx->surface);
    eglTerminate(gfxctx->display);

    if (st_dlist_get_elems_count(gfxctx->shared_data) == 1) {
        st_dlist_destroy(gfxctx->shared_data);
    } else {
        st_dlnode_t *node = st_dlist_get_head(gfxctx->shared_data);

        while (node) {
            st_gfxctx_shared_data_t **data_ptr = st_dlist_get_data(node);
            st_gfxctx_shared_data_t *data = *data_ptr;

            if (data->ctx == gfxctx) {
                st_dlist_remove(node);

                break;
            }

            node = st_dlist_get_next(node);
        }
    }

    ST_HTABLE_CALL(gfxctx->userdata, destroy);

    gfxctx_ctx->gfxctxs_count--;
    if (gfxctx_ctx->must_quit)
        st_gfxctx_quit(gfxctx_ctx);
}

static bool st_gfxctx_debug_enabled(const st_gfxctx_t *gfxctx) {
    return gfxctx->debug;
}

static void st_gfxctx_set_userdata(const st_gfxctx_t *gfxctx, const char *key,
 uintptr_t value) {
    st_gfxctxctx_t *gfxctx_ctx = (st_gfxctxctx_t *)st_object_get_owner(
     (const st_object_t *)gfxctx);
    char           *keydup = strdup(key);

    if (keydup) {
        ST_HTABLE_CALL(gfxctx->userdata, insert, NULL, keydup, (void *)value);
    } else {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(gfxctx_ctx->logger_ctx, error,
             "%s_%s: Unable to allocate memory for userdata of gfxctx "
             "\"%s\": %s", st_module_subsystem, st_module_name, key, errbuf);
    }
}

static bool st_gfxctx_get_userdata(const st_gfxctx_t *gfxctx, uintptr_t *dst,
 const char *key) {
    st_htiter_t it;
    void       *userdata;

    if (!ST_HTABLE_CALL(gfxctx->userdata, find, &it, key))
        return false;

    userdata = ST_HTITER_CALL(&it, get_value);
    *dst = (uintptr_t)userdata;

    return true;
}
