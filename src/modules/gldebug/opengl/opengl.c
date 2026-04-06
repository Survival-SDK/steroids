#include "opengl.h"

#include <stdarg.h>
#include <stdio.h>

#include <GL/gl.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/params.h"

#ifdef _WIN32
    #define MINIMAL_OPENGL "1.1"
#elif __linux__
    #define MINIMAL_OPENGL "1.2"
#else
    #error Unknown target OS
#endif

static st_gldebugctx_t *st_gldebug_init(const st_param_t params[]);
static void st_gldebug_quit(st_gldebugctx_t *gldebug_ctx);

static void st_gldebug_label_buffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
static void st_gldebug_label_shader(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
static void st_gldebug_label_shdprog(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
static void st_gldebug_label_vao(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
static void st_gldebug_label_pipeline(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
static void st_gldebug_label_texture(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
static void st_gldebug_label_framebuffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label);
static void st_gldebug_unlabel_buffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
static void st_gldebug_unlabel_shader(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
static void st_gldebug_unlabel_shdprog(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
static void st_gldebug_unlabel_vao(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
static void st_gldebug_unlabel_pipeline(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
static void st_gldebug_unlabel_texture(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
static void st_gldebug_unlabel_framebuffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id);
static const char *st_gldebug_get_error_msg(const st_gldebugctx_t *gldebug_ctx,
 unsigned err);

static st_gldebugctx_funcs_t st_gldebug_opengl_funcs = {
    ST_MODCTX_FUNCS,
    .label_buffer        = st_gldebug_label_buffer,
    .label_shader        = st_gldebug_label_shader,
    .label_shdprog       = st_gldebug_label_shdprog,
    .label_vao           = st_gldebug_label_vao,
    .label_pipeline      = st_gldebug_label_pipeline,
    .label_texture       = st_gldebug_label_texture,
    .label_framebuffer   = st_gldebug_label_framebuffer,
    .unlabel_buffer      = st_gldebug_unlabel_buffer,
    .unlabel_shader      = st_gldebug_unlabel_shader,
    .unlabel_shdprog     = st_gldebug_unlabel_shdprog,
    .unlabel_vao         = st_gldebug_unlabel_vao,
    .unlabel_pipeline    = st_gldebug_unlabel_pipeline,
    .unlabel_texture     = st_gldebug_unlabel_texture,
    .unlabel_framebuffer = st_gldebug_unlabel_framebuffer,
    .get_error_msg       = st_gldebug_get_error_msg,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "glloader", NULL, },
    {0},
};

st_moddata_t *st_module_gldebug_opengl_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("gldebug", "opengl", ST_MODULE_TYPE, mod_prereqs,
     st_gldebug_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_gldebug_opengl_init(modsmgr);
}
#endif

static bool glapi_least(st_gapi_t current_api, st_gapi_t req_api) {
    return req_api >= ST_GAPI_GL1
        && req_api <= ST_GAPI_GL46
        && current_api >= req_api;
}

static bool extension_supported(st_gldebugctx_t *gldebug_ctx, const char *ext) {
    const GLubyte *extensions;

    if (glapi_least(gldebug_ctx->api, ST_GAPI_GL3)) {
        GLint extensions_count;

        glGetIntegerv(GL_NUM_EXTENSIONS, &extensions_count);
        if (extensions_count <= 0)
            return false;

        for (GLuint i = 0; i < (GLuint)extensions_count; i++) {
            if (strcmp((const char *)gldebug_ctx->gl.get_string_i(GL_EXTENSIONS, 
             i), ext) == 0)
                return true;
        }
    }

    extensions = glGetString(GL_EXTENSIONS);

    return extensions && strstr((const char *)extensions, ext);
}

static void *glfuncs_load_with_check(st_glloaderctx_t *glloader_ctx,
 bool *prop_out, bool prop_check, const char *func_name) {
    void *func = NULL;

    if (!prop_check)
        return NULL;

    func = ST_GLLOADERCTX_CALL(glloader_ctx, get_proc_address, func_name);

    *prop_out = !!func;

    return func;
}

void impl_set_callback_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx) {}

void impl_init_control_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx) {}

void impl_remove_callback_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx) {}

void impl_label_buffer_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id,
 __attribute__((unused)) const char *label) {}

void impl_label_shader_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused))  unsigned id,
 __attribute__((unused)) const char *label) {}

void impl_label_shdprog_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id,
 __attribute__((unused)) const char *label) {}

void impl_label_vao_stub(__attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id,
 __attribute__((unused)) const char *label) {}

void impl_label_pipeline_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id,
 __attribute__((unused)) const char *label) {}

void impl_label_texture_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id,
 __attribute__((unused)) const char *label) {}

void impl_label_framebuffer_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id,
 __attribute__((unused)) const char *label) {}

void impl_unlabel_buffer_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id) {}

void impl_unlabel_shader_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused))  unsigned id) {}

void impl_unlabel_shdprog_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id) {}

void impl_unlabel_vao_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id) {}

void impl_unlabel_pipeline_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id) {}

void impl_unlabel_texture_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id) {}

void impl_unlabel_framebuffer_stub(
 __attribute__((unused)) const st_gldebugctx_t *gldebug_ctx,
 __attribute__((unused)) unsigned id) {}

static void severity_to_logger_call(const st_gldebugctx_t *gldebug_ctx,
 GLenum severity, const char *format, ...) {
    va_list args;

    va_start(args, format);
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            ST_LOGGERCTX_CALL(gldebug_ctx->logger_ctx, error, format, args);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
        case GL_DEBUG_SEVERITY_LOW:
            ST_LOGGERCTX_CALL(gldebug_ctx->logger_ctx, warning, format, args);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        default:
            ST_LOGGERCTX_CALL(gldebug_ctx->logger_ctx, info, format, args);
            break;
    };
    va_end(args);
}

static void callback(__attribute__((unused)) GLenum source,
 __attribute__((unused)) GLenum type, __attribute__((unused)) GLuint id,
 GLenum severity, __attribute__((unused)) GLsizei length, const GLchar* message,
 const void* userParam) {
    const st_gldebugctx_t *gldebug_ctx = userParam;

    severity_to_logger_call(gldebug_ctx, severity, "gldebug_opengl: %s",
     message);
}

static void callback_amd(__attribute__((unused)) GLuint id,
 __attribute__((unused)) GLenum category, GLenum severity,
 __attribute__((unused)) GLsizei length, const GLchar* message,
 GLvoid* userParam) {
    const st_gldebugctx_t *gldebug_ctx = userParam;

    severity_to_logger_call(gldebug_ctx, severity, "gldebug_opengl: %s",
     message);
}

static void impl_set_callback_core(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_callback(callback, (void *)gldebug_ctx);
}

static void impl_init_control_core(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_control(GL_DONT_CARE, GL_DONT_CARE,
     GL_DONT_CARE, 0, NULL, GL_TRUE);
}

static void impl_remove_callback_core(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_callback(NULL, NULL);
}

static void impl_label_buffer_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->gl.object_label(GL_BUFFER, id, -1, label);
}

static void impl_label_shader_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->gl.object_label(GL_SHADER, id, -1, label);
}

static void impl_label_shdprog_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->gl.object_label(GL_PROGRAM, id, -1, label);
}

static void impl_label_vao_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->gl.object_label(GL_VERTEX_ARRAY, id, -1, label);
}

static void impl_label_pipeline_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->gl.object_label(GL_PROGRAM_PIPELINE, id, -1, label);
}

static void impl_label_texture_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->gl.object_label(GL_TEXTURE, id, -1, label);
}

static void impl_label_framebuffer_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->gl.object_label(GL_FRAMEBUFFER, id, -1, label);
}

static void impl_unlabel_buffer_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->gl.object_label(GL_BUFFER, id, -1, NULL);
}

static void impl_unlabel_shader_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->gl.object_label(GL_SHADER, id, -1, NULL);
}

static void impl_unlabel_shdprog_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->gl.object_label(GL_PROGRAM, id, -1, NULL);
}

static void impl_unlabel_vao_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->gl.object_label(GL_VERTEX_ARRAY, id, -1, NULL);
}

static void impl_unlabel_pipeline_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->gl.object_label(GL_PROGRAM_PIPELINE, id, -1, NULL);
}

static void impl_unlabel_texture_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->gl.object_label(GL_TEXTURE, id, -1, NULL);
}

static void impl_unlabel_framebuffer_core(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->gl.object_label(GL_FRAMEBUFFER, id, -1, NULL);
}

static void impl_set_callback_arb(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_callback_arb(callback, gldebug_ctx);
}

static void impl_init_control_arb(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_control_arb(GL_DONT_CARE, GL_DONT_CARE,
     GL_DONT_CARE, 0, NULL, GL_TRUE);
}

static void impl_remove_callback_arb(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_callback_arb(NULL, NULL);
}

static void impl_set_callback_amd(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_callback_amd(callback_amd, (void *)gldebug_ctx);
}

static void impl_init_control_amd(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_enable_amd(0, 0, 0, NULL, GL_TRUE);
}

static void impl_remove_callback_amd(const st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->gl.debug_message_callback_amd(NULL, NULL);
}

static void load_gl_funcs(st_gldebugctx_t *gldebug_ctx) {
    st_glloaderctx_t *glloader_ctx = gldebug_ctx->glloader_ctx;

    if (!glloader_ctx) {
        gldebug_ctx->agn.set_callback        = impl_set_callback_stub;
        gldebug_ctx->agn.init_control        = impl_init_control_stub;
        gldebug_ctx->agn.remove_callback     = impl_remove_callback_stub;
        gldebug_ctx->agn.label_buffer        = impl_label_buffer_stub;
        gldebug_ctx->agn.label_shader        = impl_label_shader_stub;
        gldebug_ctx->agn.label_shdprog       = impl_label_shdprog_stub;
        gldebug_ctx->agn.label_vao           = impl_label_vao_stub;
        gldebug_ctx->agn.label_pipeline      = impl_label_pipeline_stub;
        gldebug_ctx->agn.label_texture       = impl_label_texture_stub;
        gldebug_ctx->agn.label_framebuffer   = impl_label_framebuffer_stub;
        gldebug_ctx->agn.unlabel_buffer      = impl_unlabel_buffer_stub;
        gldebug_ctx->agn.unlabel_shader      = impl_unlabel_shader_stub;
        gldebug_ctx->agn.unlabel_shdprog     = impl_unlabel_shdprog_stub;
        gldebug_ctx->agn.unlabel_vao         = impl_unlabel_vao_stub;
        gldebug_ctx->agn.unlabel_pipeline    = impl_unlabel_pipeline_stub;
        gldebug_ctx->agn.unlabel_texture     = impl_unlabel_texture_stub;
        gldebug_ctx->agn.unlabel_framebuffer = impl_unlabel_framebuffer_stub;

        return;
    }

    if (glapi_least(gldebug_ctx->api, ST_GAPI_GL3))
        gldebug_ctx->gl.get_string_i = ST_GLLOADERCTX_CALL(glloader_ctx,
         get_proc_address, "glGetStringi");

    if (glapi_least(gldebug_ctx->api, ST_GAPI_GL43)
     || (glapi_least(gldebug_ctx->api, ST_GAPI_GL11)
     && extension_supported(gldebug_ctx, "GL_KHR_debug"))) {
        /* Callback - Main */
        gldebug_ctx->gl.debug_message_callback = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_main,
            true,
            "glDebugMessageCallback"
        );
        /* Callback - Control */
        gldebug_ctx->gl.debug_message_control = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_ctrl,
            gldebug_ctx->glsupported.cbk_main,
            "glDebugMessageControl"
        );
        /* Callback - Label */
        gldebug_ctx->gl.object_label = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_label,
            gldebug_ctx->glsupported.cbk_main,
            "glObjectLabel"
        );
        gldebug_ctx->gl.object_ptr_label = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_label,
            gldebug_ctx->glsupported.cbk_label,
            "glObjectPtrLabel"
        );
        gldebug_ctx->gl.get_object_label = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_label,
            gldebug_ctx->glsupported.cbk_label,
            "glGetObjectLabel"
        );
        gldebug_ctx->gl.get_object_ptr_label = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_label,
            gldebug_ctx->glsupported.cbk_label,
            "glGetObjectPtrLabel"
        );
        if (gldebug_ctx->glsupported.cbk_main)
            gldebug_ctx->glsupported.cbk_ext = EXT_CORE;
    } else if (glapi_least(gldebug_ctx->api, ST_GAPI_GL11)
     && extension_supported(gldebug_ctx, "GL_ARB_debug_output")) {
        /* Callback - Main */
        gldebug_ctx->gl.debug_message_callback_arb = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_main,
            true,
            "glDebugMessageCallbackARB"
        );
        /* Callback - Control */
        gldebug_ctx->gl.debug_message_control_arb = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_ctrl,
            gldebug_ctx->glsupported.cbk_main,
            "glDebugMessageControlARB"
        );
        if (gldebug_ctx->glsupported.cbk_main)
            gldebug_ctx->glsupported.cbk_ext = EXT_ARB;
    } else if (glapi_least(gldebug_ctx->api, ST_GAPI_GL11)
     && extension_supported(gldebug_ctx, "GL_AMD_debug_output")) {
        /* Callback - Main */
        gldebug_ctx->gl.debug_message_callback_amd = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_main,
            true,
            "glDebugMessageCallbackAMD"
        );
        /* Callback - Control */
        gldebug_ctx->gl.debug_message_enable_amd = glfuncs_load_with_check(
            glloader_ctx,
            &gldebug_ctx->glsupported.cbk_ctrl,
            gldebug_ctx->glsupported.cbk_main,
            "glDebugMessageEnableAMD"
        );
        if (gldebug_ctx->glsupported.cbk_main)
            gldebug_ctx->glsupported.cbk_ext = EXT_AMD;
    }

    if (gldebug_ctx->glsupported.cbk_ext == EXT_CORE) {
        if (gldebug_ctx->glsupported.cbk_main) {
            gldebug_ctx->agn.set_callback = impl_set_callback_core;
            gldebug_ctx->agn.remove_callback = impl_remove_callback_core;
        } else {
            gldebug_ctx->agn.set_callback = impl_set_callback_stub;
            gldebug_ctx->agn.remove_callback = impl_remove_callback_stub;
        }
        gldebug_ctx->agn.init_control = gldebug_ctx->glsupported.cbk_ctrl
            ? impl_init_control_core
            : impl_init_control_stub;

        if (gldebug_ctx->glsupported.cbk_label) {
            gldebug_ctx->agn.label_buffer        = impl_label_buffer_core;
            gldebug_ctx->agn.label_shader        = impl_label_shader_core;
            gldebug_ctx->agn.label_shdprog       = impl_label_shdprog_core;
            gldebug_ctx->agn.label_vao           = impl_label_vao_core;
            gldebug_ctx->agn.label_pipeline      = impl_label_pipeline_core;
            gldebug_ctx->agn.label_texture       = impl_label_texture_core;
            gldebug_ctx->agn.label_framebuffer   = impl_label_framebuffer_core;
            gldebug_ctx->agn.unlabel_buffer      = impl_unlabel_buffer_core;
            gldebug_ctx->agn.unlabel_shader      = impl_unlabel_shader_core;
            gldebug_ctx->agn.unlabel_shdprog     = impl_unlabel_shdprog_core;
            gldebug_ctx->agn.unlabel_vao         = impl_unlabel_vao_core;
            gldebug_ctx->agn.unlabel_pipeline    = impl_unlabel_pipeline_core;
            gldebug_ctx->agn.unlabel_texture     = impl_unlabel_texture_core;
            gldebug_ctx->agn.unlabel_framebuffer = impl_unlabel_framebuffer_core;
        } else {
            gldebug_ctx->agn.label_buffer        = impl_label_buffer_stub;
            gldebug_ctx->agn.label_shader        = impl_label_shader_stub;
            gldebug_ctx->agn.label_shdprog       = impl_label_shdprog_stub;
            gldebug_ctx->agn.label_vao           = impl_label_vao_stub;
            gldebug_ctx->agn.label_pipeline      = impl_label_pipeline_stub;
            gldebug_ctx->agn.label_texture       = impl_label_texture_stub;
            gldebug_ctx->agn.label_framebuffer   = impl_label_framebuffer_stub;
            gldebug_ctx->agn.unlabel_buffer      = impl_unlabel_buffer_stub;
            gldebug_ctx->agn.unlabel_shader      = impl_unlabel_shader_stub;
            gldebug_ctx->agn.unlabel_shdprog     = impl_unlabel_shdprog_stub;
            gldebug_ctx->agn.unlabel_vao         = impl_unlabel_vao_stub;
            gldebug_ctx->agn.unlabel_pipeline    = impl_unlabel_pipeline_stub;
            gldebug_ctx->agn.unlabel_texture     = impl_unlabel_texture_stub;
            gldebug_ctx->agn.unlabel_framebuffer = impl_unlabel_framebuffer_stub;
        }
    } else if (gldebug_ctx->glsupported.cbk_ext == EXT_ARB) {
        if (gldebug_ctx->glsupported.cbk_main) {
            gldebug_ctx->agn.set_callback = impl_set_callback_arb;
            gldebug_ctx->agn.remove_callback = impl_remove_callback_arb;
        } else {
            gldebug_ctx->agn.set_callback = impl_set_callback_stub;
            gldebug_ctx->agn.remove_callback = impl_remove_callback_stub;
        }
        gldebug_ctx->agn.init_control = gldebug_ctx->glsupported.cbk_ctrl
            ? impl_init_control_arb
            : impl_init_control_stub;

        gldebug_ctx->agn.label_buffer        = impl_label_buffer_stub;
        gldebug_ctx->agn.label_shader        = impl_label_shader_stub;
        gldebug_ctx->agn.label_shdprog       = impl_label_shdprog_stub;
        gldebug_ctx->agn.label_vao           = impl_label_vao_stub;
        gldebug_ctx->agn.label_pipeline      = impl_label_pipeline_stub;
        gldebug_ctx->agn.label_texture       = impl_label_texture_stub;
        gldebug_ctx->agn.label_framebuffer   = impl_label_framebuffer_stub;
        gldebug_ctx->agn.unlabel_buffer      = impl_unlabel_buffer_stub;
        gldebug_ctx->agn.unlabel_shader      = impl_unlabel_shader_stub;
        gldebug_ctx->agn.unlabel_shdprog     = impl_unlabel_shdprog_stub;
        gldebug_ctx->agn.unlabel_vao         = impl_unlabel_vao_stub;
        gldebug_ctx->agn.unlabel_pipeline    = impl_unlabel_pipeline_stub;
        gldebug_ctx->agn.unlabel_texture     = impl_unlabel_texture_stub;
        gldebug_ctx->agn.unlabel_framebuffer = impl_unlabel_framebuffer_stub;
    } else if (gldebug_ctx->glsupported.cbk_ext == EXT_AMD) {
        if (gldebug_ctx->glsupported.cbk_main) {
            gldebug_ctx->agn.set_callback = impl_set_callback_amd;
            gldebug_ctx->agn.remove_callback = impl_remove_callback_amd;
        } else {
            gldebug_ctx->agn.set_callback = impl_set_callback_stub;
            gldebug_ctx->agn.remove_callback = impl_remove_callback_stub;
        }
        gldebug_ctx->agn.init_control = gldebug_ctx->glsupported.cbk_ctrl
            ? impl_init_control_amd
            : impl_init_control_stub;

        gldebug_ctx->agn.label_buffer        = impl_label_buffer_stub;
        gldebug_ctx->agn.label_shader        = impl_label_shader_stub;
        gldebug_ctx->agn.label_shdprog       = impl_label_shdprog_stub;
        gldebug_ctx->agn.label_vao           = impl_label_vao_stub;
        gldebug_ctx->agn.label_pipeline      = impl_label_pipeline_stub;
        gldebug_ctx->agn.label_texture       = impl_label_texture_stub;
        gldebug_ctx->agn.label_framebuffer   = impl_label_framebuffer_stub;
        gldebug_ctx->agn.unlabel_buffer      = impl_unlabel_buffer_stub;
        gldebug_ctx->agn.unlabel_shader      = impl_unlabel_shader_stub;
        gldebug_ctx->agn.unlabel_shdprog     = impl_unlabel_shdprog_stub;
        gldebug_ctx->agn.unlabel_vao         = impl_unlabel_vao_stub;
        gldebug_ctx->agn.unlabel_pipeline    = impl_unlabel_pipeline_stub;
        gldebug_ctx->agn.unlabel_texture     = impl_unlabel_texture_stub;
        gldebug_ctx->agn.unlabel_framebuffer = impl_unlabel_framebuffer_stub;
        gldebug_ctx->agn.remove_callback     = impl_remove_callback_stub;
    } else {
        gldebug_ctx->agn.set_callback        = impl_set_callback_stub;
        gldebug_ctx->agn.init_control        = impl_init_control_stub;
        gldebug_ctx->agn.label_buffer        = impl_label_buffer_stub;
        gldebug_ctx->agn.label_shader        = impl_label_shader_stub;
        gldebug_ctx->agn.label_shdprog       = impl_label_shdprog_stub;
        gldebug_ctx->agn.label_vao           = impl_label_vao_stub;
        gldebug_ctx->agn.label_pipeline      = impl_label_pipeline_stub;
        gldebug_ctx->agn.label_texture       = impl_label_texture_stub;
        gldebug_ctx->agn.label_framebuffer   = impl_label_framebuffer_stub;
        gldebug_ctx->agn.unlabel_buffer      = impl_unlabel_buffer_stub;
        gldebug_ctx->agn.unlabel_shader      = impl_unlabel_shader_stub;
        gldebug_ctx->agn.unlabel_shdprog     = impl_unlabel_shdprog_stub;
        gldebug_ctx->agn.unlabel_vao         = impl_unlabel_vao_stub;
        gldebug_ctx->agn.unlabel_pipeline    = impl_unlabel_pipeline_stub;
        gldebug_ctx->agn.unlabel_texture     = impl_unlabel_texture_stub;
        gldebug_ctx->agn.unlabel_framebuffer = impl_unlabel_framebuffer_stub;
        gldebug_ctx->agn.remove_callback     = impl_remove_callback_stub;
    }
}

static bool st_gldebug_load_gl_functions(st_gldebugctx_t *gldebug_ctx) {
    gldebug_ctx->api = ST_GFXCTX_CALL(gldebug_ctx->gfxctx, get_api);

    if (!gldebug_ctx->glloader_ctx) {
        ST_LOGGERCTX_CALL(gldebug_ctx->logger_ctx, warning,
         "gldebug_opengl: Unable to get glloader module. This is why unable "
         "to use OpenGL functions above OpenGL %s and extensions\n",
         MINIMAL_OPENGL);
    }

    ST_GFXCTX_CALL(gldebug_ctx->gfxctx, make_current);
    load_gl_funcs(gldebug_ctx);

    return true;
}

static st_gldebugctx_t *st_gldebug_init(const st_param_t params[]) {
    st_modsmgr_t     *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t   *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_glloaderctx_t *glloader_ctx = (st_glloaderctx_t *)ST_MODSMGR_CALL(
     modsmgr, get_singleton, "glloader", NULL);
    st_gfxctx_t      *gfxctx = st_modctx_get_param_as_ptr(params, "gfxctx");
    st_gldebugctx_t  *gldebug_ctx = (st_gldebugctx_t *)st_modctx_new(
     "gldebug", "opengl", sizeof(st_gldebugctx_t), NULL,
     &st_gldebug_opengl_funcs, (st_object_dtor_t)st_gldebug_quit);

    if (!gldebug_ctx)
        return NULL;

    gldebug_ctx->modsmgr      = modsmgr;
    gldebug_ctx->logger_ctx   = logger_ctx;
    gldebug_ctx->glloader_ctx = glloader_ctx;
    gldebug_ctx->gfxctx       = gfxctx;
    gldebug_ctx->gl           = (st_gldebug_glfuncs_t){0};
    gldebug_ctx->glsupported  = (st_gldebug_glsupported_t){0};

    if (!st_gldebug_load_gl_functions(gldebug_ctx))
        goto func_import_fail;

    if (gldebug_ctx->glsupported.cbk_main) {
        uintptr_t gldebug_ref_counter;

        if (gldebug_ctx->glsupported.cbk_ext == EXT_CORE) {
            GLint gfxctx_flags;

            glGetIntegerv(GL_CONTEXT_FLAGS, &gfxctx_flags);
            if (gfxctx_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            }
        } else if (gldebug_ctx->glsupported.cbk_ext == EXT_ARB) {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        }

        gldebug_ctx->agn.set_callback(gldebug_ctx);
        gldebug_ctx->agn.init_control(gldebug_ctx);

        if (!ST_GFXCTX_CALL(gfxctx, get_userdata, &gldebug_ref_counter,
         "gldebug_ref_counter"))
            gldebug_ref_counter = 0;

        ST_GFXCTX_CALL(gfxctx, set_userdata, "gldebug_ref_counter",
         ++gldebug_ref_counter);
    }

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "gldebug_opengl: OpenGL debug initialized");

    return gldebug_ctx;

func_import_fail:
    free(gldebug_ctx);

    return NULL;
}

static void st_gldebug_quit(st_gldebugctx_t *gldebug_ctx) {
    if (gldebug_ctx->glsupported.cbk_main) {
        uintptr_t gldebug_ref_counter;

        if (ST_GFXCTX_CALL(gldebug_ctx->gfxctx, get_userdata,
         &gldebug_ref_counter, "gldebug_ref_counter")) {
            ST_GFXCTX_CALL(gldebug_ctx->gfxctx, set_userdata,
             "gldebug_ref_counter", --gldebug_ref_counter);

            if (gldebug_ref_counter == 0)
                gldebug_ctx->agn.remove_callback(gldebug_ctx);
        }
    }

    ST_LOGGERCTX_CALL(gldebug_ctx->logger_ctx, info,
     "gldebug_opengl: OpenGL debug destroyed");
    free(gldebug_ctx);
}

static void st_gldebug_label_buffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->agn.label_buffer(gldebug_ctx, id, label);
}

static void st_gldebug_label_shader(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->agn.label_shader(gldebug_ctx, id, label);
}

static void st_gldebug_label_shdprog(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->agn.label_shdprog(gldebug_ctx, id, label);
}

static void st_gldebug_label_vao(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->agn.label_vao(gldebug_ctx, id, label);
}

static void st_gldebug_label_pipeline(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->agn.label_pipeline(gldebug_ctx, id, label);
}

static void st_gldebug_label_texture(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->agn.label_texture(gldebug_ctx, id, label);
}

static void st_gldebug_label_framebuffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id, const char *label) {
    gldebug_ctx->agn.label_framebuffer(gldebug_ctx, id, label);
}

static void st_gldebug_unlabel_buffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->agn.unlabel_buffer(gldebug_ctx, id);
}

static void st_gldebug_unlabel_shader(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->agn.unlabel_shader(gldebug_ctx, id);
}

static void st_gldebug_unlabel_shdprog(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->agn.unlabel_shdprog(gldebug_ctx, id);
}

static void st_gldebug_unlabel_vao(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->agn.unlabel_vao(gldebug_ctx, id);
}

static void st_gldebug_unlabel_pipeline(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->agn.unlabel_pipeline(gldebug_ctx, id);
}

static void st_gldebug_unlabel_texture(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->agn.unlabel_texture(gldebug_ctx, id);
}

static void st_gldebug_unlabel_framebuffer(const st_gldebugctx_t *gldebug_ctx,
 unsigned id) {
    gldebug_ctx->agn.unlabel_framebuffer(gldebug_ctx, id);
}

static const char *st_gldebug_get_error_msg(const st_gldebugctx_t *gldebug_ctx,
 unsigned err) {
    switch (err) {
        case GL_NO_ERROR:
            return "No error";
        case GL_INVALID_ENUM:
            return "Invalid enum";
        case GL_INVALID_VALUE:
            return "Invalid value";
        case GL_INVALID_OPERATION:
            return "Invalid operation";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Invalid framebuffer operation";
        case GL_OUT_OF_MEMORY:
            return "Out of memory";
        case GL_STACK_UNDERFLOW:
            return "Stack underflow";
        case GL_STACK_OVERFLOW:
            return "Stack overflow";
        default:
            break;
    }

    return "Unknown error";
}
