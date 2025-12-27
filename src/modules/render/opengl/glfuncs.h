#pragma once

#include "steroids/modules/gfxctx.h"
#include "steroids/modules/glloader.h"
#include "steroids/modules/logger.h"

typedef struct {
    /* Shader - Main */
    GLuint (*create_shader)(GLenum shaderType);
    void   (*shader_source)(GLuint shader, GLsizei count, const GLchar **string,
     const GLint *length);
    void   (*compile_shader)(GLuint shader);
    void   (*get_shader_iv)(GLuint shader, GLenum pname, GLint *params);
    GLuint (*create_program)(void);
    void   (*attach_shader)(GLuint program, GLuint shader);
    void   (*link_program)(GLuint program);
    void   (*get_program_iv)(GLuint program, GLenum pname, GLint *params);
    void   (*use_program)(GLuint program);
    GLint  (*get_attrib_location)(GLuint program, const GLchar *name);
    void   (*vertex_attrib_pointer)(GLuint index, GLint size, GLenum type,
     GLboolean normalized, GLsizei stride, const void *pointer);
    void   (*enable_vertex_attrib_array)(GLuint index);
    void   (*disable_vertex_attrib_array)(GLuint index);

    /* Shader - Delete */
    void   (*delete_shader)(GLuint shader);

    /* Shader - Log */
    void   (*get_shader_info_log)(GLuint shader, GLsizei maxLength,
     GLsizei *length, GLchar *infoLog);

    /* Shader program - Delete */
    void   (*delete_program)(GLuint program);

    /* Shader program - Log */
    void   (*get_program_info_log)(GLuint program, GLsizei maxLength,
     GLsizei *length, GLchar *infoLog);

    /* Buffer object - Main */
    void   (*gen_buffers)(GLsizei n, GLuint *buffers);
    void   (*bind_buffer)(GLenum target, GLuint buffer);
    void   (*buffer_data)(GLenum target, GLsizeiptr size, const void *data,
     GLenum usage);

    /* Buffer object - Delete */
    void   (*delete_buffers)(GLsizei n, const GLuint *buffers);

    /* Buffer object - Subdata */
    void   (*buffer_sub_data)(GLenum target, GLintptr offset, GLsizeiptr size,
     const void *data);

    /* VAO - Main */
    void   (*gen_vertex_arrays)(GLsizei n, GLuint *arrays);
    void   (*bind_vertex_array)(GLuint array);

    /* VAO - Delete */
    void   (*delete_vertex_arrays)(GLsizei n, const GLuint *arrays);
} st_glfuncs_t;

typedef struct {
    bool shader_main;
    bool shader_del;
    bool shader_log;
    bool shdprog_del;
    bool shdprog_log;
    bool buf_main;
    bool buf_del;
    bool buf_subdata;
    bool vao_main;
    bool vao_delete;
} st_glsupported_t;

static bool glfuncs_load_all(st_glfuncs_t *glfuncs,
 st_glsupported_t *glsupported, st_loggerctx_t *logger_ctx,
 st_glloaderctx_t *glloader_ctx, st_gapi_t gapi);

#include "glfuncs.imp"
