#include "bmfont.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_fontloaderctx_t *st_fontloader_init(const st_param_t params[]);
static void st_fontloader_quit(st_fontloaderctx_t *fontloader_ctx);

static st_font_t *st_fontloader_load(st_fontloaderctx_t *fontloader_ctx,
 const char *filename);
static st_font_t *st_fontloader_memload(st_fontloaderctx_t *fontloader_ctx,
 const void *data, size_t size);

static st_fontloaderctx_funcs_t fontloaderctx_funcs = {
    ST_MODCTX_FUNCS,
    .load    = st_fontloader_load,
    .memload = st_fontloader_memload,
};

static const st_modprerq_t mod_prereqs[] = {
    { "font", NULL, },
    { "logger", NULL, },
    { "xml", NULL, },
    {0},
};

st_moddata_t *st_module_fontloader_bmfont_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("fontloader", "bmfont", ST_MODULE_TYPE, mod_prereqs,
     st_fontloader_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_fontloader_bmfont_init(modsmgr);
}
#endif

static st_fontloaderctx_t *st_fontloader_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    /* We need get font_ctx via params because font_ctx is not available
     * yet as singleton when this context is created */
    st_fontctx_t   *font_ctx = st_modctx_get_param_as_ptr(params, 
     "font_ctx");
    st_xmlctx_t    *xml_ctx = ST_MODSMGR_CALL(modsmgr, have_singleton, "xml", 
     NULL)
        ? (st_xmlctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, "xml", NULL)
        : (st_xmlctx_t *)ST_MODSMGR_CALL(modsmgr, create_singleton, "xml", NULL,
         (st_params_t){{0}});

    st_fontloaderctx_t *fontloader_ctx = (st_fontloaderctx_t *)st_modctx_new(
     "fontloader", "bmfont", sizeof(st_fontloaderctx_t), NULL, 
     &fontloaderctx_funcs, (st_object_dtor_t)st_fontloader_quit);

    assert(logger_ctx);
    assert(font_ctx);
    assert(xml_ctx);

    if (!fontloader_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "fontloader_bmfont: unable to create new fontloader ctx object");

        return NULL;
    }

    fontloader_ctx->modsmgr    = modsmgr;
    fontloader_ctx->logger_ctx = logger_ctx;
    fontloader_ctx->font_ctx   = font_ctx;
    fontloader_ctx->xml_ctx    = xml_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info, 
     "fontloader_bmfont: BMFont loader initialized");

    return fontloader_ctx;
}

static void st_fontloader_quit(st_fontloaderctx_t *fontloader_ctx) {
    ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, info,
     "fontloader_bmfont: BMFont loader destroyed");
    free(fontloader_ctx);
}

static void check_ignored_attribute(st_fontloaderctx_t *fontloader_ctx, 
 const st_xml_t *xml, const char *attribute_name) {
    if (ST_XML_CALL(xml, has_attribute, attribute_name))
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, info,
         "fontloader_bmfont: Attribute \"%s\" is ignored by implementation",
         attribute_name);
}

static st_font_t *load_font_data_from_xml(st_fontloaderctx_t *fontloader_ctx,
 st_xml_t *xml) {
    st_xmlnamedchilditer_t info_iter;
    st_xml_t               info_tag;
    st_xmlnamedchilditer_t common_iter;
    st_xml_t               common_tag;
    const char            *face;
    const char            *charset;
    int                    is_unicode = 1;
    int                    line_height;
    int                    base;
    int                    scale_w;
    int                    scale_h;
    int                    pages_count;
    
    if (!ST_XML_CALL(xml, first_named_child, &info_iter, 
     "info"))
        return NULL;

    ST_XMLNAMEDCHILDITER_CALL(&info_iter, get_tag, &info_tag);

    face = ST_XML_CALL(&info_tag, get_attribute_value_str, "face");

    ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, info,
     "fontloader_bmfont: Loading font \"%s\"", 
     face ? face : "(missing face attribute)");

    check_ignored_attribute(fontloader_ctx, &info_tag, "size");
    check_ignored_attribute(fontloader_ctx, &info_tag, "bold");
    check_ignored_attribute(fontloader_ctx, &info_tag, "italic");

    charset = ST_XML_CALL(&info_tag, get_attribute_value_str, "charset");
    if (charset && strlen(charset) > 0) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Unsupported attribute \"charset\"");

        return NULL;
    }

    if (!ST_XML_CALL(&info_tag, get_attribute_value_int, &is_unicode, 
     "unicode"))
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, warning,
         "fontloader_bmfont: Missing \"unicode\" attribute");
    
    if (is_unicode != 1) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Attribute \"unicode\" must be 1, got %d", 
         is_unicode);

        return NULL;
    }

    check_ignored_attribute(fontloader_ctx, &info_tag, "stretchH");
    check_ignored_attribute(fontloader_ctx, &info_tag, "smooth");
    check_ignored_attribute(fontloader_ctx, &info_tag, "aa");
    check_ignored_attribute(fontloader_ctx, &info_tag, "padding");
    check_ignored_attribute(fontloader_ctx, &info_tag, "spacing");
    check_ignored_attribute(fontloader_ctx, &info_tag, "outline");

    if (!ST_XML_CALL(xml, first_named_child, &common_iter, "common")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"common\" tag");

        return NULL;
    }

    ST_XMLNAMEDCHILDITER_CALL(&common_iter, get_tag, &common_tag);

    if (!ST_XML_CALL(&common_tag, get_attribute_value_int, &line_height, 
     "lineHeight")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"lineHeight\" attribute");

        return NULL;
    }

    if (!ST_XML_CALL(&common_tag, get_attribute_value_int, &base, "base"))
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, warning,
         "fontloader_bmfont: Missing \"base\" attribute");
    
    if (!ST_XML_CALL(&common_tag, get_attribute_value_int, &scale_w, 
     "scaleW")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"scaleW\" attribute");

        return NULL;
    }

    if (!ST_XML_CALL(&common_tag, get_attribute_value_int, &scale_h, 
     "scaleH")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"scaleH\" attribute");

        return NULL;
    }

    if (!ST_XML_CALL(&common_tag, get_attribute_value_int, &pages_count, 
     "pages")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"pages\" attribute");

        return NULL;
    }

    check_ignored_attribute(fontloader_ctx, &common_tag, "packed");

    return ST_FONTCTX_CALL(fontloader_ctx->font_ctx, create_empty, 
     line_height, base, scale_w, scale_h, pages_count);
}

static bool load_font_pages_from_xml(st_fontloaderctx_t *fontloader_ctx, 
 st_font_t *font, st_xml_t *xml) {
    st_xmlnamedchilditer_t pages_iter;
    st_xml_t               pages_tag;
    st_xmlnamedchilditer_t page_iter;

    if (!ST_XML_CALL(xml, first_named_child, &pages_iter, "pages")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"pages\" tag");

        return false;
    }

    ST_XMLNAMEDCHILDITER_CALL(&pages_iter, get_tag, &pages_tag);
    if (!ST_XML_CALL(&pages_tag, first_named_child, &page_iter, "page")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"page\" tag");

        return false;
    }
    
    do {
        st_xml_t    page_tag;
        int         page_index;
        const char *page_filename;

        ST_XMLNAMEDCHILDITER_CALL(&page_iter, get_tag, &page_tag);
        page_filename = ST_XML_CALL(&page_tag, get_attribute_value_str, "file");
        if (!page_filename) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"file\" attribute");

            return false;
        }

        if (!ST_XML_CALL(&page_tag, get_attribute_value_int, &page_index, 
         "id")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"id\" attribute");

            return false;
        }

        if (!ST_FONT_CALL(font, add_page, page_index, page_filename))
            return false;
    } while (ST_XMLNAMEDCHILDITER_CALL(&page_iter, get_next, &page_iter));

    return true;
}

static bool load_font_chars_from_xml(st_fontloaderctx_t *fontloader_ctx,
 st_font_t *font, st_xml_t *xml) {
    st_xmlnamedchilditer_t chars_iter;
    st_xml_t               chars_tag;
    st_xmlnamedchilditer_t char_iter;
    st_xml_t               char_tag;

    if (!ST_XML_CALL(xml, first_named_child, &chars_iter, "chars")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"chars\" tag");

        return false;
    }

    ST_XMLNAMEDCHILDITER_CALL(&chars_iter, get_tag, &chars_tag);

    check_ignored_attribute(fontloader_ctx, &chars_tag, "count");

    if (!ST_XML_CALL(&chars_tag, first_named_child, &char_iter, "char")) {
        ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
         "fontloader_bmfont: Missing \"char\" tag");

        return false;
    }

    do {
        st_xml_t char_tag;
        int      char_id;
        int      subimage_x;
        int      subimage_y;
        int      subimage_width = 0;
        int      subimage_height;
        int      xoffset = 0;
        int      yoffset = 0;
        int      xadvance;
        int      char_page;
        int      chnl;
        bool     skip_char = false;

        ST_XMLNAMEDCHILDITER_CALL(&char_iter, get_tag, &char_tag);
        if (!ST_XML_CALL(&char_tag, get_attribute_value_int, &char_id, "id")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"id\" attribute");

            skip_char = true;
        }
        if (
         !ST_XML_CALL(&char_tag, get_attribute_value_int, &subimage_x, "x")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"x\" attribute");

            skip_char = true;
        }
        if (
         !ST_XML_CALL(&char_tag, get_attribute_value_int, &subimage_y, "y")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"y\" attribute");

            skip_char = true;
        }
        if (!ST_XML_CALL(&char_tag, get_attribute_value_int, &subimage_width, 
         "width")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"width\" attribute");

            skip_char = true;
        }
        if (!ST_XML_CALL(&char_tag, get_attribute_value_int, &subimage_height, 
         "height")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"height\" attribute");

            skip_char = true;
        }
        if (
         !ST_XML_CALL(&char_tag, get_attribute_value_int, &xoffset, "xoffset"))
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, warning,
             "fontloader_bmfont: Missing \"xoffset\" attribute");
        if (
         !ST_XML_CALL(&char_tag, get_attribute_value_int, &yoffset, "yoffset"))
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, warning,
             "fontloader_bmfont: Missing \"yoffset\" attribute");
        if (!ST_XML_CALL(&char_tag, get_attribute_value_int, &xadvance, 
         "xadvance")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, warning,
             "fontloader_bmfont: Missing \"xadvance\" attribute");

            xadvance = subimage_width;
        }
        if (
         !ST_XML_CALL(&char_tag, get_attribute_value_int, &char_page, "page")) {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
             "fontloader_bmfont: Missing \"page\" attribute");

            skip_char = true;
        }
        if (
         ST_XML_CALL(&char_tag, get_attribute_value_int, &chnl, "chnl")) {
            if (chnl != 15) {
                ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, error,
                 "fontloader_bmfont: Attribute \"chnl\" must be 15, got %d", 
                 chnl);

                skip_char = true;
            }
        } else {
            ST_LOGGERCTX_CALL(fontloader_ctx->logger_ctx, warning,
             "fontloader_bmfont: Missing \"chnl\" attribute");
        }
        
        if (!skip_char)
            ST_FONT_CALL(font, add_char, char_id, subimage_x, subimage_y, 
             subimage_width, subimage_height, xoffset, yoffset, xadvance, 
             char_page);
    } while (ST_XMLNAMEDCHILDITER_CALL(&char_iter, get_next, &char_iter));

    return true;
}

static st_font_t *st_fontloader_load_xml_impl(
 st_fontloaderctx_t *fontloader_ctx, st_xml_t *xml) {
    st_font_t *font;

    if (strcmp(ST_XML_CALL(xml, get_name), "font") != 0)
        return NULL;

    font = load_font_data_from_xml(fontloader_ctx, xml);
    if (!font)
        return NULL;


    if (!load_font_pages_from_xml(fontloader_ctx, font, xml) 
     || !load_font_chars_from_xml(fontloader_ctx, font, xml))
        goto fail;

    return font;
fail:
    ST_FONT_CALL(font, destroy);

    return NULL;
}

static st_font_t *st_fontloader_load(st_fontloaderctx_t *fontloader_ctx,
 const char *filename) {
    st_xml_t *xml = ST_XMLCTX_CALL(fontloader_ctx->xml_ctx, load, filename);
    
    if (xml) {
        st_font_t *font = st_fontloader_load_xml_impl(fontloader_ctx, xml);

        ST_XML_CALL(xml, destroy);

        return font;
    }

    return NULL;
}

static st_font_t *st_fontloader_memload(st_fontloaderctx_t *fontloader_ctx,
 const void *data, size_t size) {
    return NULL; /* Not supported */
}
