#pragma once

#include "steroids/modules/dynarr.h"
#include "steroids/modules/logger.h"

#define INITIAL_ENTRIES_CAPACITY 8192
#define VERTICES_PER_TEXTURE     6

static bool batcher_init(st_batcher_t *batcher, st_loggerctx_t *logger_ctx, 
 st_dynarrctx_t *dynarr_ctx) {
    batcher->entries = ST_DYNARRCTX_CALL(dynarr_ctx, create,
     sizeof(st_batch_entry_t), INITIAL_ENTRIES_CAPACITY);

    if (!batcher->entries) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "render_opengl: Unable to create dynamic array for batcher entries");

        return false;
    }
    batcher->current_texture = NULL;
    batcher->current_first_vertex_index = 0;
    batcher->current_vertex_index = 0;

    return true;
}

static void batcher_free(st_batcher_t *batcher) {
    ST_DYNARR_CALL(batcher->entries, destroy);
}

static bool batcher_clear(st_batcher_t *batcher) {
    batcher->current_texture = NULL;
    batcher->current_first_vertex_index = 0;
    batcher->current_vertex_index = 0;
    return ST_DYNARR_CALL(batcher->entries, clear);
}

static void batcher_finalize(st_batcher_t *batcher) {
    ST_DYNARR_CALL(batcher->entries, append, &(st_batch_entry_t){
        batcher->current_texture,
        batcher->current_first_vertex_index,
        batcher->current_vertex_index - batcher->current_first_vertex_index,
    });
}

static void batcher_process_texture(st_batcher_t *batcher,
 const st_texture_t *texture) {
    if (!batcher->current_texture)
        batcher->current_texture = texture;

    if (texture != batcher->current_texture) {
        batcher_finalize(batcher);
        batcher->current_first_vertex_index = batcher->current_vertex_index;
        batcher->current_texture = texture;
    }

    batcher->current_vertex_index += VERTICES_PER_TEXTURE;
}

static bool batcher_bind_texture(st_batcher_t *batcher, size_t entry_index,
 unsigned gfxctx_shared_index) {
    const st_batch_entry_t *entry = ST_DYNARR_CALL(batcher->entries, get,
     entry_index);

    return ST_TEXTURE_CALL(entry->texture, bind, gfxctx_shared_index);
}

static GLsizei batcher_get_first_vertex_index(const st_batcher_t *batcher,
 size_t entry_index) {
    const st_batch_entry_t *entry = ST_DYNARR_CALL(batcher->entries, get,
     entry_index);

    return (GLsizei)entry->first_vertex_index;
}

static GLint batcher_get_vertices_count(const st_batcher_t *batcher,
 size_t entry_index) {
    const st_batch_entry_t *entry = ST_DYNARR_CALL(batcher->entries, get,
     entry_index);

    return (GLint)entry->vertices_count;
}

static size_t batcher_get_entries_count(const st_batcher_t *batcher) {
    return ST_DYNARR_CALL(batcher->entries, get_elements_count);
}
