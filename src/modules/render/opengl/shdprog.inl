#pragma once

#define SHDPROG_LOG_SIZE 1024

static bool shdprog_init(st_shdprog_t *shdprog, st_loggerctx_t *logger_ctx,
 st_gldebugctx_t *gldebug_ctx, const st_glfuncs_t *gl, st_shader_t *vert,
 st_shader_t *frag) {
    GLint  linked;
    GLchar log[SHDPROG_LOG_SIZE];

    shdprog->gl = gl;
    shdprog->handle = gl->create_program();
    if (!shdprog->handle) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable create shader program: %s",
         ST_GLDEBUGCTX_CALL(gldebug_ctx, get_error_msg, glGetError()));

        return false;
    }

    gl->attach_shader(shdprog->handle, vert->handle);
    gl->attach_shader(shdprog->handle, frag->handle);
    gl->link_program(shdprog->handle);

    gl->get_program_iv(shdprog->handle, GL_LINK_STATUS, &linked);
    if(!linked) {
        gl->get_program_info_log(shdprog->handle, SHDPROG_LOG_SIZE, NULL, log);
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable to link shader program: %s", log);
        shdprog->handle = 0;

        return false;
    }

    return true;
}

static void shdprog_free(st_shdprog_t *shdprog) {
    if (shdprog && shdprog->handle) {
        shdprog->gl->delete_program(shdprog->handle);
        shdprog->handle = 0;
    }
}

static void shdprog_use(const st_shdprog_t *shdprog) {
    shdprog->gl->use_program(shdprog->handle);
}

static void shdprog_unuse(const st_shdprog_t *shdprog) {
    shdprog->gl->use_program(0);
}
