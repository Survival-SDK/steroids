#pragma once

static bool vertattr_init(st_vertattr_t *vertattr, st_loggerctx_t *logger_ctx,
 st_gldebugctx_t *gldebug_ctx, const st_glfuncs_t *gl,
 const st_vbo_t *vbo, const st_shdprog_t *shdprog, const char *name,
 unsigned components_count, unsigned offset) {
    GLenum error;

    vertattr->gl = gl;
    vertattr->handle = gl->get_attrib_location(shdprog->handle, name);

    if (vertattr->handle == -1) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable to get attribute \"%s\" location in shader "
         "program: %s",
         name,
         ST_GLDEBUGCTX_CALL(gldebug_ctx, get_error_msg, glGetError()));

        return false;
    }

    gl->enable_vertex_attrib_array((GLuint)vertattr->handle);
    vbo_bind(vbo);
    gl->vertex_attrib_pointer((GLuint)vertattr->handle, (GLint)components_count,
     GL_FLOAT, GL_FALSE,
     (GLsizei)(sizeof(float) * vbo_get_components_per_vertex(vbo)),
     (void *)(uintptr_t)offset);

    error = glGetError();
    if (error != GL_NO_ERROR)
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable to init vertex attribute: %s",
         ST_GLDEBUGCTX_CALL(gldebug_ctx, get_error_msg, error));

    vbo_unbind(vbo);
    gl->disable_vertex_attrib_array((GLuint)vertattr->handle);

    if (error != GL_NO_ERROR)
        vertattr->handle = -1;

    return error == GL_NO_ERROR;
}

static void vertattr_free(st_vertattr_t *vertattr) {
    vertattr->handle = -1;
}

static void vertattr_enable(const st_vertattr_t *vertattr) {
    if (vertattr->handle != -1)
        vertattr->gl->enable_vertex_attrib_array((GLuint)vertattr->handle);
}

static void vertattr_disable(const st_vertattr_t *vertattr) {
    if (vertattr->handle != -1)
        vertattr->gl->disable_vertex_attrib_array((GLuint)vertattr->handle);
}
