#pragma once

#define VERTEX_SHADER_SOURCE_GL33                         \
    "#version 330 core\n"                                 \
    "attribute vec3 pos;\n"                               \
    "attribute vec2 vert_tex_coord;\n"                    \
    "out       vec2 frag_tex_coord;\n"                    \
    "void main() {\n"                                     \
    "    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n" \
    "    frag_tex_coord = vert_tex_coord;\n"              \
    "}\n"

#define FRAGMENT_SHADER_SOURCE_GL33                      \
    "#version 330 core\n"                                \
    "in  vec2 frag_tex_coord;\n"                         \
    "out vec4 color;\n"                                  \
    "uniform sampler2D in_texture;"                      \
    "void main() {\n"                                    \
    "    color = texture(in_texture, frag_tex_coord);\n" \
    "}\n"

#define SHD_SOURCES_COUNT 1
#define SHD_LOG_SIZE      1024

typedef enum {
    SHD_VERTEX   = GL_VERTEX_SHADER,
    SHD_FRAGMENT = GL_FRAGMENT_SHADER,
} st_shader_type_t;

static bool shader_init(st_shader_t *shader, st_loggerctx_t *logger_ctx, 
 st_gldebugctx_t *gldebug_ctx, const st_glfuncs_t *gl, st_shader_type_t type, 
 const char *source) {
    GLint  compiled;
    GLchar log[SHD_LOG_SIZE];

    shader->gl = gl;
    shader->handle = gl->create_shader(type);
    if (!shader->handle) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable create shader: %s",
         ST_GLDEBUGCTX_CALL(gldebug_ctx, get_error_msg, glGetError()));

        return false;
    }

    gl->shader_source(shader->handle, SHD_SOURCES_COUNT, &source, NULL);
    gl->compile_shader(shader->handle);

    gl->get_shader_iv(shader->handle, GL_COMPILE_STATUS, &compiled);
    if(!compiled) {
        gl->get_shader_info_log(shader->handle, SHD_LOG_SIZE, NULL, log);
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable to compile shader: %s", log);
        shader->handle = 0;

        return false;
    }

    return true;
}

static void shader_free(st_shader_t *shader) {
    if (shader && shader->handle) {
        shader->gl->delete_shader(shader->handle);
        shader->handle = 0;
    }
}
