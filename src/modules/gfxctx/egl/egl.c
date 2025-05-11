#include "egl.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "steroids/modules/monitor.h"

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

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_gfxctx_funcs_t gfxctx_funcs = {
    .make_current     = st_gfxctx_make_current,
    .swap_buffers     = st_gfxctx_swap_buffers,
    .get_window       = st_gfxctx_get_window,
    .get_api          = st_gfxctx_get_api,
    .get_shared_index = st_gfxctx_get_shared_index,
    .destroy          = st_gfxctx_destroy,
    .debug_enabled    = st_gfxctx_debug_enabled,
    .set_userdata     = st_gfxctx_set_userdata,
    .get_userdata     = st_gfxctx_get_userdata,
};

ST_MODULE_DEF_GET_FUNC(gfxctx_egl)
ST_MODULE_DEF_INIT_FUNC(gfxctx_egl)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_gfxctx_egl_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_gfxctx_import_functions(st_modctx_t *gfxctx_ctx,
 st_modctx_t *logger_ctx) {
    st_gfxctx_egl_t *module = gfxctx_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "gfxctx_egl: Unable to load function \"error\" from module "
         "\"logger\"\n"
        );

        return false;
    }

    ST_LOAD_FUNCTION("gfxctx_egl", fnv1a, NULL, get_u32hashstr_func);

    ST_LOAD_FUNCTION("gfxctx_egl", htable, NULL, create);
    ST_LOAD_FUNCTION("gfxctx_egl", htable, NULL, init);
    ST_LOAD_FUNCTION("gfxctx_egl", htable, NULL, quit);

    ST_LOAD_FUNCTION_FROM_CTX("gfxctx_egl", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("gfxctx_egl", logger, info);
    ST_LOAD_FUNCTION_FROM_CTX("gfxctx_egl", logger, warning);

    return true;
}

static bool st_keyeqfunc(const void *left, const void *right) {
    return left && right && strcmp(left, right) == 0;
}

static st_modctx_t *st_gfxctx_init(st_modctx_t *logger_ctx,
 st_modctx_t *monitor_ctx) {
    st_modctx_t     *gfxctx_ctx;
    st_gfxctx_egl_t *module;

    gfxctx_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_gfxctx_egl_data, sizeof(st_gfxctx_egl_t));

    if (!gfxctx_ctx)
        return NULL;

    gfxctx_ctx->funcs = &st_gfxctx_egl_funcs;

    module = gfxctx_ctx->data;
    module->logger.ctx = logger_ctx;
    module->monitor.ctx = monitor_ctx;
    module->debug_enabled = false;
    module->must_quit = false;
    module->gfxctxs_count = 0;

    if (!st_gfxctx_import_functions(gfxctx_ctx, logger_ctx))
        goto import_fail;

    module->htable.ctx = module->htable.init(module->logger.ctx);
    if (!module->htable.ctx)
        goto ht_ctx_init_fail;

    module->logger.info(module->logger.ctx,
     "gfxctx_egl: Graphics context mgr initialized");

    return gfxctx_ctx;

ht_ctx_init_fail:
import_fail:
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, gfxctx_ctx);

    return NULL;
}

static void st_gfxctx_quit(st_modctx_t *gfxctx_ctx) {
    st_gfxctx_egl_t *module = gfxctx_ctx->data;

    if (module->gfxctxs_count > 0) {
        module->must_quit = true;

        return;
    }

    module->htable.quit(module->htable.ctx);

    module->logger.info(module->logger.ctx,
     "gfxctx_egl: Graphics context mgr destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, gfxctx_ctx);
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

static st_logger_generic_msg_t msgtype_to_logger_func(st_gfxctx_egl_t *module,
 EGLint message_type) {
    switch (message_type) {
        case EGL_DEBUG_MSG_CRITICAL_KHR:
        case EGL_DEBUG_MSG_ERROR_KHR:
            return module->logger.error;
        case EGL_DEBUG_MSG_WARN_KHR:
            return module->logger.warning;
        case EGL_DEBUG_MSG_INFO_KHR:
        default:
            return module->logger.info;
    };

    return module->logger.info;
}

static void st_debug_callback(__attribute__((unused)) EGLenum error,
 const char *command, EGLint message_type, EGLLabelKHR thread_label,
 __attribute__((unused)) EGLLabelKHR object_label, const char* message) {
    st_gfxctx_egl_t *module = thread_label;

    msgtype_to_logger_func(module, message_type)(module->logger.ctx,
     "gfxctx_egl: \"%s\": %s", command, message);
}

void st_try_to_enable_debug(st_gfxctx_t *gfxctx) {
    st_gfxctx_egl_t *module = ((st_modctx_t *)st_object_get_owner(gfxctx))->data;
    EGLint           ret;

    if (module->debug_enabled)
        return;

    if (!extension_supported(gfxctx->display, "EGL_KHR_debug"))
        return;

    module->egl_debug_message_control_khr = (void *)eglGetProcAddress(
     "eglDebugMessageControlKHR");
    if (!module->egl_debug_message_control_khr)
        return;

    module->egl_label_object_khr = (void *)eglGetProcAddress(
     "eglLabelObjectKHR");
    if (!module->egl_label_object_khr)
        return;

    ret = module->egl_debug_message_control_khr(st_debug_callback,
     (EGLAttrib[]){
        EGL_DEBUG_MSG_INFO_KHR,     EGL_TRUE,
        EGL_DEBUG_MSG_WARN_KHR,     EGL_TRUE,
        EGL_DEBUG_MSG_ERROR_KHR,    EGL_TRUE,
        EGL_DEBUG_MSG_CRITICAL_KHR, EGL_TRUE,
        EGL_NONE
    });

    if (ret != EGL_SUCCESS)
        return;

    ret = module->egl_label_object_khr(NULL, EGL_OBJECT_THREAD_KHR, NULL,
     module);

    module->logger.info(module->logger.ctx, "gfxctx_egl: EGL debug enabled");

    module->debug_enabled = true;
}

static st_gfxctx_t *st_gfxctx_create_impl(st_modctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, EGLint renderable_type,
 EGLint major, EGLint minor, st_gfxctx_t *shared) {
    st_gfxctx_egl_t *module = gfxctx_ctx->data;
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

    gfxctx = malloc(sizeof(st_gfxctx_t));
    if (!gfxctx) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "gfxctx_egl: Unable to allocate memory for gfx context: %s",
             errbuf);

        return NULL;
    }

    st_object_make(gfxctx, gfxctx_ctx, &gfxctx_funcs);

    gfxctx->userdata = module->htable.create(module->htable.ctx,
     (unsigned int (*)(const void *))module->fnv1a.get_u32hashstr_func(NULL),
     st_keyeqfunc, free, NULL);
    if (!gfxctx->userdata)
        goto udata_fail;

    gfxctx->display = eglGetDisplay(
     (EGLNativeDisplayType)ST_MONITOR_CALL(monitor, get_handle));

    if (gfxctx->display == EGL_NO_DISPLAY) {
        module->logger.error(module->logger.ctx,
         "gfxctx_egl: Unable to get EGL display");

        goto get_display_fail;
    }



    if (eglInitialize(gfxctx->display, &egl_version_major, &egl_version_minor)
     == EGL_FALSE) {
        if (!module->debug_enabled)
            module->logger.error(module->logger.ctx,
             "gfxctx_egl: Unable to initialize EGL: %s",
             get_egl_error_str(eglGetError()));

        goto egl_init_fail;
    }

    if (egl_version_minor < 5
     && renderable_type == EGL_OPENGL_BIT
     && ((major == 1 && minor > MINIMAL_OPENGL_MINOR) || major > 1)
     && !shared) {
        module->logger.warning(module->logger.ctx,
             "gfxctx_egl: EGL 1.%i doesn't able to export core OpenGL %i.%i "
             "functions. Try to use another gfxctx module if available",
             egl_version_minor, major, minor);
    }

    st_try_to_enable_debug(gfxctx);

    if (module->debug_enabled)
        module->egl_label_object_khr(gfxctx->display, EGL_OBJECT_DISPLAY_KHR,
         gfxctx->display, monitor);

    egl_khr_create_context_supported = extension_supported(gfxctx->display,
     "EGL_KHR_create_context");

    process_attrs(&cfg_attrs, &ctx_attrs, &version_changed, &debug_changed,
     egl_version_minor, egl_khr_create_context_supported);

    if (version_changed && !shared) {
        module->logger.warning(module->logger.ctx,
         "gfxctx_egl: Unable to create context with required version. Possible "
         "reasons: required version of context or minor version of context may "
         "be not supported by current version of EGL or EGL_KHR_create_context "
         "extension is not present");
    }

    if (debug_changed && !shared) {
        module->logger.warning(module->logger.ctx,
         "gfxctx_egl: Debug context is not supported. Possible "
         "reasons: debug context is not supported by current version of EGL or "
         "EGL_KHR_create_context extension is not present");
    }

    gfxctx->debug = !debug_changed;

    if ((version_changed || debug_changed) && !shared) {
        char gapi_str[GAPI_STR_SIZE_MAX];

        attrs_fill_gapi_str(gapi_str, &cfg_attrs, &ctx_attrs);

        module->logger.warning(module->logger.ctx,
         "gfxctx_egl: Current version of EGL: 1.%i", egl_version_minor);
        module->logger.warning(module->logger.ctx,
         "gfxctx_egl: Fallback context created: %s", gapi_str);
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
        if (!module->debug_enabled)
            module->logger.error(module->logger.ctx,
             "gfxctx_egl: Unable to get matching frame buffer configuration: "
             "%s",
             get_egl_error_str(eglGetError()));

        goto choose_config_fail;
    }

    if (eglBindAPI(getegl_api_by_gapi((st_gapi_t)gfxctx->gapi)) == EGL_FALSE) {
        if (!module->debug_enabled)
            module->logger.error(module->logger.ctx,
             "gfxctx_egl: Unable to bind EGL API: %s",
             get_egl_error_str(eglGetError()));

        goto bind_api_fail;
    }

    gfxctx->surface = eglCreateWindowSurface(gfxctx->display,
     gfxctx->cfg,
     (EGLNativeWindowType)*(void *[]){ ST_WINDOW_CALL(window, get_handle) }, NULL);
    if (gfxctx->surface == EGL_NO_SURFACE) {
        if (!module->debug_enabled)
            module->logger.error(module->logger.ctx,
             "gfxctx_egl: Unable to create EGL window surface: %s",
             get_egl_error_str(eglGetError()));

        goto create_surface_fail;
    }

    if (module->debug_enabled)
        module->egl_label_object_khr(gfxctx->display, EGL_OBJECT_DISPLAY_KHR,
         gfxctx->surface, window);

    gfxctx->handle = eglCreateContext(gfxctx->display, gfxctx->cfg,
     shared ? shared->handle : EGL_NO_CONTEXT, egl_ctx_attrs);
    if (gfxctx->handle == EGL_NO_CONTEXT) {
        if (!module->debug_enabled)
            module->logger.error(module->logger.ctx,
             "gfxctx_egl: Unable to create EGL render context: %s",
             get_egl_error_str(eglGetError()));

        goto create_context_fail;
    }

    if (module->debug_enabled)
        module->egl_label_object_khr(gfxctx->display, EGL_OBJECT_CONTEXT_KHR,
         gfxctx->handle, gfxctx);

    gfxctx->window = window;
    if (!shared) {
        gfxctx->shared_data = st_dlist_create(sizeof(st_gfxctx_shared_data_t),
         NULL);
        if (gfxctx->shared_data) {
            st_dlist_push_back(gfxctx->shared_data, &(st_gfxctx_shared_data_t){
                .ctx   = gfxctx,
                .index = 0,
            });
        } else {
            module->logger.warning(module->logger.ctx,
             "gfxctx_egl: Unable to create structure for shared contexts data. "
             "This context will not able to be shared");
        }
    } else {
        gfxctx->shared_data = shared->shared_data;
        st_dlist_push_back(gfxctx->shared_data, &(st_gfxctx_shared_data_t){
            .ctx   = gfxctx,
            .index = st_shared_data_get_free_index(gfxctx->shared_data),
        });
    }

    if (!shared)
        eglMakeCurrent(gfxctx->display, gfxctx->surface, gfxctx->surface,
         gfxctx->handle);
    if (eglSwapInterval(gfxctx->display, 1) == EGL_FALSE) {
        if (!module->debug_enabled)
            module->logger.warning(module->logger.ctx,
             "gfxctx_egl: Unable to set swap interval: %s",
             get_egl_error_str(eglGetError()));
    }

    module->gfxctxs_count++;

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

static st_gfxctx_t *st_gfxctx_create(st_modctx_t *gfxctx_ctx,
 st_monitor_t *monitor, st_window_t *window, st_gapi_t api) {
    st_gfxctx_egl_t *module = gfxctx_ctx->data;

    if (api < ST_GAPI_GL11 || api > ST_GAPI_ES32) {
        module->logger.error(module->logger.ctx,
         "gfxctx_egl: Unsupported gfx API");

        return NULL;
    }

    return st_gfxctx_create_impl(gfxctx_ctx, monitor, window,
     get_renderable_type_by_gapi(api), get_major_version_by_gapi(api),
     get_minor_version_by_gapi(api), NULL);
}

static st_gfxctx_t *st_gfxctx_create_shared(st_modctx_t *gfxctx_ctx,
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

static st_gapi_t st_gfxctx_get_api(st_gfxctx_t *gfxctx) {
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
    st_modctx_t     *gfxctx_ctx = st_object_get_owner(gfxctx);
    st_gfxctx_egl_t *module = gfxctx_ctx->data;

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
            st_gfxctx_shared_data_t *data = st_dlist_get_data(node);

            if (data->ctx == gfxctx) {
                st_dlist_remove(node);

                break;
            }

            node = st_dlist_get_next(node);
        }
    }

    ST_HTABLE_CALL(gfxctx->userdata, destroy);

    free(gfxctx);

    module->gfxctxs_count--;
    if (module->must_quit)
        st_gfxctx_quit(gfxctx_ctx);

}

static bool st_gfxctx_debug_enabled(const st_gfxctx_t *gfxctx) {
    return gfxctx->debug;
}

static void st_gfxctx_set_userdata(const st_gfxctx_t *gfxctx, const char *key,
 uintptr_t value) {
    st_gfxctx_egl_t *module = ((st_modctx_t *)st_object_get_owner(gfxctx))->data;
    char            *keydup = strdup(key);

    if (keydup) {
        ST_HTABLE_CALL(gfxctx->userdata, insert, NULL, keydup, (void *)value);
    } else {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "gfxctx_egl: Unable to allocate memory for userdata of gfxctx "
             "\"%s\": %s", key, errbuf);
    }
}

static bool st_gfxctx_get_userdata(const st_gfxctx_t *gfxctx, uintptr_t *dst,
 const char *key) {
    st_gfxctx_egl_t *module = ((st_modctx_t *)st_object_get_owner(gfxctx))->data;
    st_htiter_t      it;
    void            *userdata;

    if (!ST_HTABLE_CALL(gfxctx->userdata, find, &it, key))
        return false;

    userdata = ST_HTITER_CALL(&it, get_value);
    *dst = (uintptr_t)userdata;

    return true;
}
