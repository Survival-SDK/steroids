#include "ezxml.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ezxml.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/object.h"
#include "steroids/modules/xml.h"

static st_xmlctx_t *st_xml_init(const st_param_t params[]);
static void st_xml_quit(st_xmlctx_t *xml_ctx);

/* ctx methods */
static st_xml_t *st_xml_load(st_xmlctx_t *xml_ctx, const char *filename);
static st_xml_t *st_xml_memload(st_xmlctx_t *xml_ctx, const void *data,
 size_t size);

/* tag methods */
static bool st_xml_first_child(st_xml_t *xml, st_xmlchilditer_t *dst);
static bool st_xml_first_named_child(st_xml_t *xml, 
 st_xmlnamedchilditer_t *dst, const char *tag_name);
static const char *st_xml_get_tag_name(const st_xml_t *xml);
static const char *st_xml_get_tag_text(const st_xml_t *xml);
static bool st_xml_tag_has_attribute(const st_xml_t *xml, 
 const char *attribute_name);
static int st_xml_get_tag_attributes_count(const st_xml_t *xml);
static const char *st_xml_get_tag_attribute_name(const st_xml_t *xml, 
 int index);
static const char *st_xml_get_tag_attribute_value(const st_xml_t *xml, 
 const char *attribute_name);

/* tag destructor */
static void st_xml_destroy(st_xml_t *xml);

/* child iter methods */
static bool st_xml_next_child(st_xmlchilditer_t *current, 
 st_xmlchilditer_t *dst);
static bool st_xml_get_iter_tag(const st_xmlchilditer_t *iter, st_xml_t *dst);

/* named child iter methods */
static bool st_xml_next_named_child(st_xmlnamedchilditer_t *current, 
 st_xmlnamedchilditer_t *dst);
static bool st_xml_get_named_iter_tag(const st_xmlnamedchilditer_t *iter, 
 st_xml_t *dst);

static st_xmlctx_funcs_t xmlctx_funcs = {
    ST_MODCTX_FUNCS,
    .load = st_xml_load,
    .memload = st_xml_memload,
};

static st_xml_funcs_t xml_funcs = {
    ST_OBJECT_FUNCS,
    .first_child          = st_xml_first_child,
    .first_named_child    = st_xml_first_named_child,
    .get_name             = st_xml_get_tag_name,
    .get_text             = st_xml_get_tag_text,
    .has_attribute        = st_xml_tag_has_attribute,
    .get_attributes_count = st_xml_get_tag_attributes_count,
    .get_attribute_name   = st_xml_get_tag_attribute_name,
    .get_attribute_value  = st_xml_get_tag_attribute_value,
};

static st_xmlchilditer_funcs_t xmlchilditer_funcs = {
    ST_OBJECT_FUNCS,
    .get_next = st_xml_next_child,
    .get_tag  = st_xml_get_iter_tag,
};

static st_xmlnamedchilditer_funcs_t xmlnamedchilditer_funcs = {
    ST_OBJECT_FUNCS,
    .get_next = st_xml_next_named_child,
    .get_tag  = st_xml_get_named_iter_tag,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_xml_ezxml_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("xml", "ezxml", ST_MODULE_TYPE, mod_prereqs,
     st_xml_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_xml_ezxml_init(modsmgr);
}
#endif

static st_xmlctx_t *st_xml_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_xmlctx_t    *xml_ctx = (st_xmlctx_t *)st_modctx_new("xml", "ezxml", 
     sizeof(st_xmlctx_t), NULL, &xmlctx_funcs, (st_object_dtor_t)st_xml_quit);

    if (!xml_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "xml_ezxml: unable to create new xml ctx object");

        return NULL;
    }

    xml_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "xml_ezxml: XML manipulation module context initialized");

    return xml_ctx;
}

static void st_xml_quit(st_xmlctx_t *xml_ctx) {
    ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, info,
     "xml_ezxml: XML manipulation module context destroyed");
    free(xml_ctx);
}

static st_xml_t *st_xml_load_impl(st_xmlctx_t *xml_ctx, char *data,
 size_t size) {
    struct ezxml *handle;

    handle = ezxml_parse_str(data, size);
    if (!handle) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to parse XML data");

        return NULL;
    }

    st_xml_t *xml = (st_xml_t *)st_object_new(sizeof(st_xml_t), &xml_funcs,
     (st_object_dtor_t)st_xml_destroy, (st_object_t *)xml_ctx);
    if (!xml) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to allocate memory for XML object");

        ezxml_free(handle);

        return NULL;
    }

    xml->handle = handle;
    xml->buffer = data;

    return xml;
}

static st_xml_t *st_xml_load(st_xmlctx_t *xml_ctx, const char *filename) {
    FILE   *fp;
    long    fsize;
    char   *data;
    st_xml_t *xml;

    if (!filename) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Filename is NULL");
        
        return NULL;
    }

    fp = fopen(filename, "rb");
    if (!fp) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to open XML file \"%s\"", filename);

        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to seek XML file \"%s\"", filename);
        
        goto fseek_end_fail;
    }

    fsize = ftell(fp);
    if (fsize < 0) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to get XML file size \"%s\"", filename);
        
        goto ftell_fail;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to rewind XML file \"%s\"", filename);
        
        goto fseek_set_fail;
    }

    data = malloc((size_t)fsize);
    if (!data) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to allocate memory for XML file \"%s\"", filename);
        
        goto malloc_fail;
    }

    if (fsize > 0 && fread(data, 1, (size_t)fsize, fp) != (size_t)fsize) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to read XML file \"%s\"", filename);
        
        goto read_fail;
    }

    fclose(fp);

    xml = st_xml_load_impl(xml_ctx, data, (size_t)fsize);
    if (!xml)
        free(data);

    return xml;

read_fail:
    free(data);

malloc_fail:
fseek_set_fail:
ftell_fail:
fseek_end_fail:
    fclose(fp);

    return NULL;
}

static st_xml_t *st_xml_memload(st_xmlctx_t *xml_ctx, const void *data,
 size_t size) {
    char *rwdata = malloc(size);
    st_xml_t *xml;

    if (!rwdata) {
        ST_LOGGERCTX_CALL(xml_ctx->logger_ctx, error,
         "xml_ezxml: Unable to allocate memory for XML raw data");

        return NULL;
    }

    memcpy(rwdata, data, size);

    xml = st_xml_load_impl(xml_ctx, rwdata, size);

    if (!xml)
        free(rwdata);

    return xml;
}

static void st_xml_destroy(st_xml_t *xml) {
    if (xml) {
        ezxml_free(xml->handle);
        free(xml->buffer);
        free(xml);
    }
}

static bool st_xml_first_child(st_xml_t *xml, st_xmlchilditer_t *dst) {
    st_xmlchilditer_t *iter;

    assert(xml);
    assert(dst);

    iter = (st_xmlchilditer_t *)st_object_placement_new(dst, 
     &xmlchilditer_funcs, st_object_fake_dtor, (st_object_t *)xml);

    iter->st_userdata = (uintptr_t)xml->handle->child;

    return !!iter->st_userdata;
}

static bool st_xml_first_named_child(st_xml_t *xml, 
 st_xmlnamedchilditer_t *dst, const char *tag_name) {
    st_xmlnamedchilditer_t *iter;

    assert(xml);
    assert(dst);

    iter = (st_xmlnamedchilditer_t *)st_object_placement_new(dst, 
     &xmlnamedchilditer_funcs, st_object_fake_dtor, (st_object_t *)xml);

    iter->st_userdata = (uintptr_t)ezxml_child(xml->handle, tag_name);

    return !!iter->st_userdata;
}

static const char *st_xml_get_tag_name(const st_xml_t *xml) {
    return ezxml_name(xml->handle);
}

static const char *st_xml_get_tag_text(const st_xml_t *xml) {
    const char *text = ezxml_txt(xml->handle);

    return strlen(text) > 0 
        ? text 
        : NULL;
}

static bool st_xml_tag_has_attribute(const st_xml_t *xml, 
 const char *attribute_name) {
    return !!ezxml_attr(xml->handle, attribute_name);
}

static int st_xml_get_tag_attributes_count(const st_xml_t *xml) {
    char **attributes = xml->handle->attr;
    int    attrs_count = 0;

    while (*attributes) {
        attributes += 2;
        attrs_count++;
    }

    return attrs_count;
}

static const char *st_xml_get_tag_attribute_name(const st_xml_t *xml, 
 int index) {
    char **attributes = xml->handle->attr;
    int    attrs_count = 0;

    while (*attributes) {
        if (attrs_count == index)
            return *attributes;

        attributes += 2;
        attrs_count++;
    }

    return NULL;
}

static const char *st_xml_get_tag_attribute_value(const st_xml_t *xml, 
 const char *attribute_name) {
    return ezxml_attr(xml->handle, attribute_name);
}

static bool st_xml_next_child(st_xmlchilditer_t *current, 
 st_xmlchilditer_t *dst) {
    struct ezxml *current_xml;
    
    if (!current || !dst)
        return false;
    
    current_xml = (struct ezxml *)current->st_userdata;

    if (!current_xml->ordered)
        return false;
    
    st_object_placement_new(dst, &xmlchilditer_funcs, st_object_fake_dtor, 
     st_object_get_owner_unsafe(current));
    dst->st_userdata = (uintptr_t)current_xml->ordered;

    return true;
}

static bool st_xml_get_iter_tag_impl(const st_object_t *iter, st_xml_t *dst) {
    struct ezxml *handle;
    st_object_t  *iter_owner;

    if (!iter || !dst)
        return false;

    iter_owner = st_object_get_owner_unsafe(iter);

    handle = (struct ezxml *)iter->st_userdata;

    st_object_placement_new(dst, &xml_funcs, st_object_fake_dtor, 
     st_object_get_owner_unsafe(iter_owner));
    
    dst->handle = handle;
    dst->buffer = NULL;

    return true;
}

static bool st_xml_get_iter_tag(const st_xmlchilditer_t *iter, st_xml_t *dst) {
    return st_xml_get_iter_tag_impl(iter, dst);
}

static bool st_xml_next_named_child(st_xmlnamedchilditer_t *current, 
 st_xmlnamedchilditer_t *dst) {
    struct ezxml *current_xml;
    
    if (!current || !dst)
        return false;
    
    current_xml = (struct ezxml *)current->st_userdata;

    if (!current_xml->next)
        return false;
    
    st_object_placement_new(dst, &xmlnamedchilditer_funcs, st_object_fake_dtor, 
     st_object_get_owner_unsafe(current));
    dst->st_userdata = (uintptr_t)current_xml->next;

    return true;
}

static bool st_xml_get_named_iter_tag(const st_xmlnamedchilditer_t *iter, 
 st_xml_t *dst) {
    return st_xml_get_iter_tag_impl(iter, dst);
}
