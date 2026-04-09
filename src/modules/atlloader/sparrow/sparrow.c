#include "sparrow.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_atlloaderctx_t *st_atlloader_init(const st_param_t params[]);
static void st_atlloader_quit(st_atlloaderctx_t *atlloader_ctx);

static st_atlas_t *st_atlloader_load(st_atlloaderctx_t *atlloader_ctx,
 const char *filename);
static st_atlas_t *st_atlloader_memload(st_atlloaderctx_t *atlloader_ctx,
 const void *data, size_t size);

static st_atlloaderctx_funcs_t atlloaderctx_funcs = {
    ST_MODCTX_FUNCS,
    .load    = st_atlloader_load,
    .memload = st_atlloader_memload,
};

static const st_modprerq_t mod_prereqs[] = {
    { "atlas", NULL, },
    { "logger", NULL, },
    { "xml", NULL, },
    {0},
};

st_moddata_t *st_module_atlloader_sparrow_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("atlloader", "sparrow", ST_MODULE_TYPE, mod_prereqs,
     st_atlloader_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_atlloader_sparrow_init(modsmgr);
}
#endif

static st_atlloaderctx_t *st_atlloader_init(const st_param_t params[]) {
    st_modsmgr_t      *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t    *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    /* We need get atlas_ctx via params because atlas_ctx is not available
     * yet as singleton when this context is created */
    st_atlasctx_t     *atlas_ctx = st_modctx_get_param_as_ptr(params, 
     "atlas_ctx");
    st_xmlctx_t       *xml_ctx = ST_MODSMGR_CALL(modsmgr, have_singleton, "xml", 
     NULL)
        ? (st_xmlctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, "xml", NULL)
        : (st_xmlctx_t *)ST_MODSMGR_CALL(modsmgr, create_singleton, "xml", NULL,
         (st_params_t){{0}});

    st_atlloaderctx_t *atlloader_ctx = (st_atlloaderctx_t *)st_modctx_new(
     "atlloader", "sparrow", sizeof(st_atlloaderctx_t), NULL, 
     &atlloaderctx_funcs, (st_object_dtor_t)st_atlloader_quit);

    assert(logger_ctx);
    assert(atlas_ctx);
    assert(xml_ctx);

    if (!atlloader_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "atlloader_sparrow: unable to create new atlloader ctx object");

        return NULL;
    }

    atlloader_ctx->modsmgr    = modsmgr;
    atlloader_ctx->logger_ctx = logger_ctx;
    atlloader_ctx->atlas_ctx  = atlas_ctx;
    atlloader_ctx->xml_ctx    = xml_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "atlloader_sparrow: Sparrow / Starling XML format atlas loader "
     "initialized");

    return atlloader_ctx;
}

static void st_atlloader_quit(st_atlloaderctx_t *atlloader_ctx) {
    ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, info,
     "atlloader_sparrow: Sparrow / Starling XML format atlas loader "
     "destroyed");
    free(atlloader_ctx);
}

static st_atlas_t *st_atlloader_load_impl(st_atlloaderctx_t *atlloader_ctx,
 st_xml_t *xml) {
    const char            *image_path;
    st_xmlnamedchilditer_t subtexture_iter;
    bool                   has_subtextures;
    st_atlas_t            *atlas;

    if (strcmp(ST_XML_CALL(xml, get_name), "TextureAtlas") != 0)
        return NULL;

    if (!ST_XML_CALL(xml, has_attribute, "imagePath"))
        return NULL;

    has_subtextures = ST_XML_CALL(xml, first_named_child, &subtexture_iter, 
     "SubTexture");

    if (!has_subtextures)
        return NULL;

    image_path = ST_XML_CALL(xml, get_attribute_value_str, "imagePath");
    atlas = ST_ATLASCTX_CALL(atlloader_ctx->atlas_ctx, create_empty, 
     image_path);

    if (!atlas) {
        ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, error,
         "atlloader_sparrow: Unable to create new atlas object");

        return NULL;
    }

    while (has_subtextures) {
        st_xml_t    subtexture_tag;
        const char *name;
        int         x;
        int         y;
        int         width;
        int         height;

        ST_XMLNAMEDCHILDITER_CALL(&subtexture_iter, get_tag, &subtexture_tag);

        name = ST_XML_CALL(&subtexture_tag, get_attribute_value_str, "name");
        if (!name) {
            ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, error,
             "atlloader_sparrow: SubTexture tag of texture atlas \"%s\" has "
             "no \"name\" attribute", image_path);
            
            goto missing_attribute;
        }

        if (!ST_XML_CALL(&subtexture_tag, has_attribute, "name")) {
            ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, error,
             "atlloader_sparrow: SubTexture tag of texture atlas \"%s\" has "
             "no \"name\" attribute", image_path);
            
            goto missing_attribute;
        }

        if (!ST_XML_CALL(&subtexture_tag, has_attribute, "x")) {
            ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, error,
             "atlloader_sparrow: SubTexture tag with name \"%s\" of texture "
             "atlas \"%s\" has no \"x\" attribute", name, image_path);
            
            goto missing_attribute;
        }

        if (!ST_XML_CALL(&subtexture_tag, has_attribute, "y")) {
            ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, error,
             "atlloader_sparrow: SubTexture tag with name \"%s\" of texture "
             "atlas \"%s\" has no \"y\" attribute", name, image_path);
            
            goto missing_attribute;
        }

        if (!ST_XML_CALL(&subtexture_tag, has_attribute, "width")) {
            ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, error,
             "atlloader_sparrow: SubTexture tag with name \"%s\" of texture "
             "atlas \"%s\" has no \"width\" attribute", name, image_path);
            
            goto missing_attribute;
        }

        if (!ST_XML_CALL(&subtexture_tag, has_attribute, "height")) {
            ST_LOGGERCTX_CALL(atlloader_ctx->logger_ctx, error,
             "atlloader_sparrow: SubTexture tag with name \"%s\" of texture "
             "atlas \"%s\" has no \"height\" attribute", name, image_path);
            
            goto missing_attribute;
        }

        
        ST_XML_CALL(&subtexture_tag, get_attribute_value_int, &x, "x");
        ST_XML_CALL(&subtexture_tag, get_attribute_value_int, &y, "y");
        ST_XML_CALL(&subtexture_tag, get_attribute_value_int, &width, "width");
        ST_XML_CALL(&subtexture_tag, get_attribute_value_int, &height, "height");

        ST_ATLAS_CALL(atlas, add_subimage, name, x, y, width, height);

        has_subtextures = ST_XMLNAMEDCHILDITER_CALL(&subtexture_iter, get_next, 
         &subtexture_iter);
    }

    return atlas;

missing_attribute:
    ST_ATLAS_CALL(atlas, destroy);

    return NULL;
}

static st_atlas_t *st_atlloader_load(st_atlloaderctx_t *atlloader_ctx,
 const char *filename) {
    st_xml_t   *xml = ST_XMLCTX_CALL(atlloader_ctx->xml_ctx, load, filename);
    
    if (xml) {
        st_atlas_t *atlas = st_atlloader_load_impl(atlloader_ctx, xml);

        ST_XML_CALL(xml, destroy);

        return atlas;
    }

    return NULL;
}

static st_atlas_t *st_atlloader_memload(st_atlloaderctx_t *atlloader_ctx,
 const void *data, size_t size) {
    st_xml_t *xml = ST_XMLCTX_CALL(atlloader_ctx->xml_ctx, memload, data, size);
    if (xml) {
        st_atlas_t *atlas = st_atlloader_load_impl(atlloader_ctx, xml);

        ST_XML_CALL(xml, destroy);

        return atlas;
    }

    return NULL;
}
