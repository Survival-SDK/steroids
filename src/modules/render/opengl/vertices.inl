#pragma once

#define INITIAL_VERTICES_CAPACITY 1048576

static bool vertices_init(st_dynarr_t **vertices, st_loggerctx_t *logger_ctx, 
 st_dynarrctx_t *dynarr_ctx) {
    *vertices = ST_DYNARRCTX_CALL(dynarr_ctx, create,
     sizeof(float), INITIAL_VERTICES_CAPACITY);

    if (!*vertices) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable to create dynamic array for vertices data");

        return false;
    }

    return true;
}

static void vertices_free(st_dynarr_t *vertices) {
    ST_DYNARR_CALL(vertices, destroy);
}

static bool vertices_add(st_dynarr_t *vertices, float x, float y, float z,
 float u, float v) {
    return ST_DYNARR_CALL(vertices, append, &x)
        && ST_DYNARR_CALL(vertices, append, &y)
        && ST_DYNARR_CALL(vertices, append, &z)
        && ST_DYNARR_CALL(vertices, append, &u)
        && ST_DYNARR_CALL(vertices, append, &v);
}

static bool vertices_clear(st_dynarr_t *vertices) {
    return ST_DYNARR_CALL(vertices, clear);
}

static const void *vertices_get_all(st_dynarr_t *vertices) {
    return ST_DYNARR_CALL(vertices, get_all);
}

static size_t vertices_size(const st_dynarr_t *vertices) {
    return ST_DYNARR_CALL(vertices, get_elements_count) * sizeof(float);
}
