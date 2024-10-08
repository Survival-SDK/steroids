#pragma once

#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/object.h"

#ifndef ST_ZIP_T_DEFINED
    typedef struct st_zip_s st_zip_t;
#endif

typedef enum {
    ST_ZET_UNKNOWN = 0,
    ST_ZET_FILE    = 1,
    ST_ZET_DIR     = 2,
} st_zipentrytype_t;

typedef st_modctx_t *(*st_zip_init_t)(st_modctx_t *fs_ctx,
 st_modctx_t *logger_ctx, st_modctx_t *pathtools_ctx);
typedef void (*st_zip_quit_t)(st_modctx_t *zip_ctx);
typedef st_zip_t *(*st_zip_open_t)(st_modctx_t *zip_ctx, const char *filename);
typedef st_zip_t *(*st_zip_memopen_t)(st_modctx_t *zip_ctx, const void *data,
 size_t size);
typedef void (*st_zip_close_t)(st_zip_t *zip);
typedef ssize_t (*st_zip_get_entries_count_t)(st_zip_t *zip);
typedef bool (*st_zip_get_entry_name_t)(st_zip_t *zip, char *dst,
 size_t dstsize, size_t entrynum);
typedef st_zipentrytype_t (*st_zip_get_entry_type_t)(st_zip_t *zip,
 size_t entrynum);
typedef bool (*st_zip_extract_entry_t)(st_zip_t *zip, size_t entrynum,
 const char *path);

typedef struct {
    st_zip_init_t    zip_init;
    st_zip_quit_t    zip_quit;
    st_zip_open_t    zip_open;
    st_zip_memopen_t zip_memopen;
} st_zipctx_funcs_t;

typedef struct {
    st_zip_close_t             close;
    st_zip_get_entries_count_t get_entries_count;
    st_zip_get_entry_name_t    get_entry_name;
    st_zip_get_entry_type_t    get_entry_type;
    st_zip_extract_entry_t     extract_entry;
} st_zip_funcs_t;

#define ST_ZIP_CALL(object, func, ...) \
    ((st_zip_funcs_t *)((const st_object_t *)object)->funcs)->func(object, ## __VA_ARGS__)
