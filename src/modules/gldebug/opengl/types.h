#pragma once

#include <GL/gl.h>
#include <GL/glext.h>

#include "steroids/modules/gfxctx.h"
#include "steroids/modules/glloader.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t        *ctx;
    st_logger_debug_t   debug;
    st_logger_info_t    info;
    st_logger_warning_t warning;
    st_logger_error_t   error;
} st_gldebug_opengl_logger_t;

typedef struct {
    st_gfxctx_t *handle;
    st_gapi_t    api;
} st_gldebug_opengl_gfxctx_t;

typedef struct {
    /* Core */
    const GLubyte *(*get_string_i)(GLenum name, GLuint index);
    void (*debug_message_callback)(GLDEBUGPROC callback, void *userParam);
    void (*debug_message_control)(GLenum source, GLenum type, GLenum severity,
     GLsizei count, const GLuint *ids, GLboolean enabled);
    void (*object_label)(GLenum identifier, GLuint name, GLsizei length,
     const char *label);
    void (*object_ptr_label)(void *ptr, GLsizei length, const char *label);
    void (*get_object_label)(GLenum identifier, GLuint name, GLsizei bufSize,
     GLsizei *length, char *label);
    void (*get_object_ptr_label)(void *ptr, GLsizei bufSize, GLsizei *length,
     char *label);

    /* ARB */
    void (*debug_message_callback_arb)(GLDEBUGPROCARB callback,
     const void* userParam);
    void (*debug_message_control_arb)(GLenum source, GLenum type,
     GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);

    /* AMD */
    void (*debug_message_callback_amd)(GLDEBUGPROCAMD callback,
     void* userParam);
    void (*debug_message_enable_amd)(GLenum category, GLenum severity,
     GLsizei count, const GLuint* ids, GLboolean enabled);
} st_gldebug_glfuncs_t;

typedef struct {
    void (*set_callback)(const st_modctx_t *gldebug_ctx);
    void (*init_control)(const st_modctx_t *gldebug_ctx);
    void (*remove_callback)(const st_modctx_t *gldebug_ctx);
    void (*label_buffer)(const st_modctx_t *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_shader)(const st_modctx_t *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_shdprog)(const st_modctx_t *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_vao)(const st_modctx_t *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_pipeline)(const st_modctx_t *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_texture)(const st_modctx_t *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_framebuffer)(const st_modctx_t *gldebug_ctx, unsigned id,
     const char *label);
    void (*unlabel_buffer)(const st_modctx_t *gldebug_ctx, unsigned id);
    void (*unlabel_shader)(const st_modctx_t *gldebug_ctx, unsigned id);
    void (*unlabel_shdprog)(const st_modctx_t *gldebug_ctx, unsigned id);
    void (*unlabel_vao)(const st_modctx_t *gldebug_ctx, unsigned id);
    void (*unlabel_pipeline)(const st_modctx_t *gldebug_ctx, unsigned id);
    void (*unlabel_texture)(const st_modctx_t *gldebug_ctx, unsigned id);
    void (*unlabel_framebuffer)(const st_modctx_t *gldebug_ctx, unsigned id);
} st_gldebug_apiagnostic_t;

typedef enum {
    EXT_NONE = 0,
    EXT_CORE,
    EXT_ARB,
    EXT_AMD,
} st_ext_t;

typedef struct {
    bool     cbk_main;
    bool     cbk_ctrl;
    bool     cbk_label;
    st_ext_t cbk_ext;
} st_gldebug_glsupported_t;

typedef struct {
    st_gldebug_opengl_gfxctx_t gfxctx;
    st_gldebug_opengl_logger_t logger;

    st_gldebug_glfuncs_t       gl;
    st_gldebug_glsupported_t   glsupported;
    st_gldebug_apiagnostic_t   agn;
} st_gldebug_opengl_t;
