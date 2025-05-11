#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/modctx.h"
#include "steroids/types/modsmgr.h"
#include "steroids/types/modules/logger.h"
#include "steroids/types/object.h"

#ifndef ST_INICTX_T_DEFINED
    typedef struct st_inictx_s st_inictx_t;
#endif
#ifndef ST_INI_T_DEFINED
    typedef struct st_ini_s st_ini_t;
#endif

typedef st_inictx_t *(*st_ini_init_t)(struct st_loggerctx_s *logger_ctx,
 st_modsmgr_t *modsmgr);

typedef st_ini_t *(*st_ini_create_t)(st_inictx_t *ini_ctx);
typedef st_ini_t *(*st_ini_load_t)(st_inictx_t *ini_ctx, const char *filename);
typedef st_ini_t *(*st_ini_memload_t)(st_inictx_t *ini_ctx, const void *ptr,
 size_t size);
typedef bool (*st_ini_section_exists_t)(const st_ini_t *ini,
 const char *section);
typedef bool (*st_ini_key_exists_t)(const st_ini_t *ini, const char *section,
 const char *key);
typedef const char *(*st_ini_get_str_t)(const st_ini_t *ini,
 const char *section, const char *key);
typedef bool (*st_ini_fill_str_t)(const st_ini_t *ini, char *dst,
 size_t dstsize, const char *section, const char *key);
typedef bool (*st_ini_delete_section_t)(st_ini_t *ini, const char *section);
typedef bool (*st_ini_delete_key_t)(st_ini_t *ini, const char *section,
 const char *key);
typedef bool (*st_ini_clear_section_t)(st_ini_t *ini, const char *section);
typedef bool (*st_ini_add_section_t)(st_ini_t *ini, const char *section);
typedef bool (*st_ini_add_key_t)(st_ini_t *ini, const char *section,
 const char *key, const char *value);
typedef bool (*st_ini_export_t)(const st_ini_t *ini, char *buffer,
 size_t bufsize);
typedef bool (*st_ini_save_t)(const st_ini_t *ini, const char *filename);

typedef struct {
    st_modctx_funcs_t;
    st_ini_create_t  create;
    st_ini_load_t    load;
    st_ini_memload_t memload;
} st_inictx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_ini_section_exists_t section_exists;
    st_ini_key_exists_t     key_exists;
    st_ini_get_str_t        get_str;
    st_ini_fill_str_t       fill_str;
    st_ini_delete_section_t delete_section;
    st_ini_delete_key_t     delete_key;
    st_ini_clear_section_t  clear_section;
    st_ini_add_section_t    add_section;
    st_ini_add_key_t        add_key;
    st_ini_export_t         to_buffer;
    st_ini_save_t           save;
} st_ini_funcs_t;

#define ST_INICTX_CALL(ctx, func, ...) \
    ((st_inictx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_INI_CALL(object, func, ...) \
    ((st_ini_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
