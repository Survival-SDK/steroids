#pragma once

#include <stddef.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_ZIPCTX_T_DEFINED
    typedef st_modctx_t st_zipctx_t;
#endif
#ifndef ST_ZIP_T_DEFINED
    typedef st_object_t st_zip_t;
#endif

typedef enum {
    ST_ZET_UNKNOWN = 0,
    ST_ZET_FILE    = 1,
    ST_ZET_DIR     = 2,
} st_zipentrytype_t;

typedef st_zip_t *(*st_zip_open_t)(st_zipctx_t *zip_ctx, const char *filename);
typedef st_zip_t *(*st_zip_memopen_t)(st_zipctx_t *zip_ctx, const void *data,
 size_t size);
typedef ssize_t (*st_zip_get_entries_count_t)(st_zip_t *zip);
typedef bool (*st_zip_get_entry_name_t)(st_zip_t *zip, char *dst,
 size_t dstsize, size_t entrynum);
typedef st_zipentrytype_t (*st_zip_get_entry_type_t)(st_zip_t *zip,
 size_t entrynum);
typedef bool (*st_zip_extract_entry_t)(st_zip_t *zip, size_t entrynum,
 const char *path);

typedef struct {
    st_modctx_funcs_t;
    st_zip_open_t    open;
    st_zip_memopen_t memopen;
} st_zipctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_zip_get_entries_count_t get_entries_count;
    st_zip_get_entry_name_t    get_entry_name;
    st_zip_get_entry_type_t    get_entry_type;
    st_zip_extract_entry_t     extract_entry;
} st_zip_funcs_t;

#define ST_ZIPCTX_CALL(object, func, ...) \
    ((st_zipctx_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
#define ST_ZIP_CALL(object, func, ...) \
    ((st_zip_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
