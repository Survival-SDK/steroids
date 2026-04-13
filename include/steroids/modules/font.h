#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "steroids/modctx.h"
#include "steroids/modules/sprite.h"
#include "steroids/object.h"

#ifndef ST_FONTCTX_T_DEFINED
    typedef st_modctx_t st_fontctx_t;
#endif
#ifndef ST_FONT_T_DEFINED
    typedef st_object_t st_font_t;
#endif

typedef st_font_t *(*st_font_load_t)(st_fontctx_t *font_ctx,
 const char *filename);
typedef st_font_t *(*st_font_memload_t)(st_fontctx_t *font_ctx,
 const void *data, size_t size);

/* Callbacks for loaders */
typedef st_font_t *(*st_font_create_empty_t)(st_fontctx_t *font_ctx, 
 unsigned line_height, unsigned base, unsigned texture_width, 
 unsigned texture_height, unsigned pages_count);
typedef bool (*st_font_add_page_t)(st_font_t *font, unsigned index, 
 const char *filename);
typedef bool (*st_font_memadd_page_t)(st_font_t *font, unsigned index, 
 const void *data, size_t size);
typedef bool (*st_font_add_char_t)(st_font_t *font, uint32_t ucs4code, 
 unsigned subimage_x, unsigned subimage_y, unsigned subimage_width, 
 unsigned subimage_height, int xoffset, int yoffset, int xadvance, 
 unsigned page);

typedef unsigned (*st_font_get_line_height_t)(const st_font_t *font);
typedef unsigned (*st_font_get_base_t)(const st_font_t *font);
typedef const st_sprite_t *(*st_font_get_sprite_t)(const st_font_t *font, 
 uint32_t ucs4code);
typedef int (*st_font_get_xoffset_t)(const st_font_t *font, uint32_t ucs4code);
typedef int (*st_font_get_yoffset_t)(const st_font_t *font, uint32_t ucs4code);
typedef int (*st_font_get_xadvance_t)(const st_font_t *font, uint32_t ucs4code);

typedef struct {
    st_modctx_funcs_t;
    st_font_load_t         load;
    st_font_memload_t      memload;
    st_font_create_empty_t create_empty;
} st_fontctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_font_add_page_t         add_page;
    st_font_memadd_page_t      memadd_page;
    st_font_add_char_t         add_char;
    st_font_get_line_height_t  get_line_height;
    st_font_get_base_t         get_base;
    st_font_get_sprite_t       get_sprite;
    st_font_get_xoffset_t      get_xoffset;
    st_font_get_yoffset_t      get_yoffset;
    st_font_get_xadvance_t     get_xadvance;
} st_font_funcs_t;

#define ST_FONTCTX_CALL(ctx, func, ...) \
    ((st_fontctx_funcs_t *)((const st_object_t *)ctx)->funcs)->func(ctx, \
     ## __VA_ARGS__)
#define ST_FONT_CALL(object, func, ...) \
    ((st_font_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
