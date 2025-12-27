#pragma once

#define DEFAULT_VAO_NAMES_NUMBER 1

static void vao_init(st_vao_t *vao, const st_glfuncs_t *gl) {
    vao->gl = gl;
    
    gl->gen_vertex_arrays(DEFAULT_VAO_NAMES_NUMBER, &vao->handle);
    gl->bind_vertex_array(vao->handle);
}

static void vao_free(st_vao_t *vao) {
    vao->gl->delete_vertex_arrays(DEFAULT_VAO_NAMES_NUMBER, &vao->handle);
}

static void vao_bind(const st_vao_t *vao) {
    vao->gl->bind_vertex_array(vao->handle);
}

static void vao_unbind(const st_vao_t *vao) {
    /* TODO(edomin): noop on the Core Profile */
    vao->gl->bind_vertex_array(0);
}
