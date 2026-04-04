#pragma once

#include <GL/gl.h>
#include <GL/glext.h>

#include "steroids/modctx.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/gfxctx.h"
#include "steroids/modules/glloader.h"
#include "steroids/modules/logger.h"

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

struct st_gldebugctx;

typedef struct {
    void (*set_callback)(const struct st_gldebugctx *gldebug_ctx);
    void (*init_control)(const struct st_gldebugctx *gldebug_ctx);
    void (*remove_callback)(const struct st_gldebugctx *gldebug_ctx);
    void (*label_buffer)(const struct st_gldebugctx *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_shader)(const struct st_gldebugctx *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_shdprog)(const struct st_gldebugctx *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_vao)(const struct st_gldebugctx *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_pipeline)(const struct st_gldebugctx *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_texture)(const struct st_gldebugctx *gldebug_ctx, unsigned id,
     const char *label);
    void (*label_framebuffer)(const struct st_gldebugctx *gldebug_ctx, unsigned id,
     const char *label);
    void (*unlabel_buffer)(const struct st_gldebugctx *gldebug_ctx, unsigned id);
    void (*unlabel_shader)(const struct st_gldebugctx *gldebug_ctx, unsigned id);
    void (*unlabel_shdprog)(const struct st_gldebugctx *gldebug_ctx, unsigned id);
    void (*unlabel_vao)(const struct st_gldebugctx *gldebug_ctx, unsigned id);
    void (*unlabel_pipeline)(const struct st_gldebugctx *gldebug_ctx, unsigned id);
    void (*unlabel_texture)(const struct st_gldebugctx *gldebug_ctx, unsigned id);
    void (*unlabel_framebuffer)(const struct st_gldebugctx *gldebug_ctx, unsigned id);
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

typedef struct st_gldebugctx {
    st_modctx_t;
    st_modsmgr_t      *modsmgr;
    st_loggerctx_t    *logger_ctx;
    st_glloaderctx_t  *glloader_ctx;
    st_gfxctx_t       *gfxctx;
    st_gapi_t          api;

    st_gldebug_glfuncs_t       gl;
    st_gldebug_glsupported_t   glsupported;
    st_gldebug_apiagnostic_t   agn;
} st_gldebugctx_t;

#define ST_GLDEBUGCTX_T_DEFINED
