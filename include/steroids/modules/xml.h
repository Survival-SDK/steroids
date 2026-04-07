#pragma once

#include <stdbool.h>

#include "steroids/modctx.h"
#include "steroids/object.h"

#ifndef ST_XMLCTX_T_DEFINED
    typedef st_modctx_t st_xmlctx_t;
#endif
#ifndef ST_XML_T_DEFINED
    typedef st_object_t st_xml_t;
#endif
#ifndef ST_XMLCHILDITER_T_DEFINED
    typedef st_object_t st_xmlchilditer_t;
#endif
#ifndef ST_XMLNAMEDCHILDITER_T_DEFINED
    typedef st_object_t st_xmlnamedchilditer_t;
#endif

/* ctx methods */
typedef st_xml_t *(*st_xml_load_t)(st_xmlctx_t *xml_ctx, const char *filename);
typedef st_xml_t *(*st_xml_memload_t)(st_xmlctx_t *xml_ctx, const void *data,
 size_t size);

/* tag methods */
typedef bool (*st_xml_first_child_t)(st_xml_t *xml, st_xmlchilditer_t *dst);
typedef bool (*st_xml_first_named_child_t)(st_xml_t *xml, 
 st_xmlnamedchilditer_t *dst, const char *tag_name);
typedef const char *(*st_xml_get_tag_name_t)(const st_xml_t *xml);
typedef const char *(*st_xml_get_tag_text_t)(const st_xml_t *xml);
typedef bool (*st_xml_tag_has_attribute_t)(const st_xml_t *xml, 
 const char *attribute_name);
typedef int (*st_xml_get_tag_attributes_count_t)(const st_xml_t *xml);
typedef const char *(*st_xml_get_tag_attribute_name_t)(const st_xml_t *xml, 
 int index);
typedef const char *(*st_xml_get_tag_attribute_value_t)(const st_xml_t *xml, 
 const char *attribute_name);

/* child iter methods */
typedef bool (*st_xml_next_child_t)(st_xmlchilditer_t *current, 
 st_xmlchilditer_t *dst);
typedef bool (*st_xml_get_iter_tag_t)(const st_xmlchilditer_t *iter, 
 st_xml_t *dst);

/* named child iter methods */
typedef bool (*st_xml_next_named_child_t)(st_xmlnamedchilditer_t *current, 
 st_xmlnamedchilditer_t *dst);
typedef bool (*st_xml_get_named_iter_tag_t)(const st_xmlnamedchilditer_t *iter, 
 st_xml_t *dst);

typedef struct {
    st_modctx_funcs_t;
    st_xml_load_t    load;
    st_xml_memload_t memload;
} st_xmlctx_funcs_t;

typedef struct {
    st_object_funcs_t;
    st_xml_first_child_t              first_child;
    st_xml_first_named_child_t        first_named_child;
    st_xml_get_tag_name_t             get_name;
    st_xml_get_tag_text_t             get_text;
    st_xml_tag_has_attribute_t        has_attribute;
    st_xml_get_tag_attributes_count_t get_attributes_count;
    st_xml_get_tag_attribute_name_t   get_attribute_name;
    st_xml_get_tag_attribute_value_t  get_attribute_value;
} st_xml_funcs_t;

typedef struct st_xmlchilditer_funcs {
    st_object_funcs_t;
    st_xml_next_child_t   get_next;
    st_xml_get_iter_tag_t get_tag;
} st_xmlchilditer_funcs_t;

typedef struct st_xmlnamedchilditer_funcs {
    st_object_funcs_t;
    st_xml_next_named_child_t   get_next;
    st_xml_get_named_iter_tag_t get_tag;
} st_xmlnamedchilditer_funcs_t;

#define ST_XMLCTX_CALL(object, func, ...) \
    ((st_xmlctx_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
#define ST_XML_CALL(object, func, ...) \
    ((st_xml_funcs_t *)((const st_object_t *)object)->funcs)->func(object, \
     ## __VA_ARGS__)
#define ST_XMLCHILDITER_CALL(object, func, ...) \
    ((st_xmlchilditer_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
#define ST_XMLNAMEDCHILDITER_CALL(object, func, ...) \
    ((st_xmlnamedchilditer_funcs_t *)((const st_object_t *)object)->funcs)->func( \
     object, ## __VA_ARGS__)
