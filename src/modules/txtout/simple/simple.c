#include "simple.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "steroids/moddata.h"
#include "steroids/modules/txtout.h"
#include "steroids/modsmgr.h"

static st_txtoutctx_t *st_txtout_init(const st_param_t params[]);
static void st_txtout_quit(st_txtoutctx_t *txtout_ctx);

static unsigned st_txtout_get_text_width(const st_txtoutctx_t *txtout_ctx, 
 const st_font_t *font, const char *text);
static const char *st_txtout_get_subtext_after_wrap(
 const st_txtoutctx_t *txtout_ctx, size_t *codepoints_before_wrap,
 const st_font_t *font, const char *text, unsigned max_width);
static ssize_t st_txtout_get_output_data(const st_txtoutctx_t *txtout_ctx, 
 st_txtoutentry_t *dst, const st_font_t *font, const char *text, 
 size_t codepoints, float x, float y, float hscale, float vscale, float pivot_x, 
 float pivot_y);

static st_txtoutctx_funcs_t txtoutctx_funcs = {
    ST_MODCTX_FUNCS,
    .get_text_width         = st_txtout_get_text_width,
    .get_subtext_after_wrap = st_txtout_get_subtext_after_wrap,
    .get_output_data        = st_txtout_get_output_data,
};

static const st_modprerq_t mod_prereqs[] = {
    { "font", NULL, },
    { "logger", NULL, },
    { "utf8", NULL, },
    {0},
};

st_moddata_t *st_module_txtout_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("txtout", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_txtout_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_txtout_simple_init(modsmgr);
}
#endif

static st_txtoutctx_t *st_txtout_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_fontctx_t   *font_ctx = (st_fontctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "font", NULL);
    st_utf8ctx_t   *utf8_ctx = ST_MODSMGR_CALL(modsmgr, have_singleton, "utf8", 
     NULL) 
        ? (st_utf8ctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, "utf8", NULL)
        : (st_utf8ctx_t *)ST_MODSMGR_CALL(modsmgr, create_singleton, "utf8", 
         NULL, (st_params_t){{0}});
    st_txtoutctx_t *txtout_ctx;
    
    if (!logger_ctx || !font_ctx || !utf8_ctx)
        return NULL;

    txtout_ctx = (st_txtoutctx_t *)st_modctx_new("txtout", "simple", 
     sizeof(st_txtoutctx_t), NULL, &txtoutctx_funcs, 
     (st_object_dtor_t)st_txtout_quit);
    if (!txtout_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "txtout_simple: Unable to create new txtout ctx object");

        return NULL;
    }

    txtout_ctx->logger_ctx = logger_ctx;
    txtout_ctx->font_ctx   = font_ctx;
    txtout_ctx->utf8_ctx   = utf8_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "txtout_simple: Text output utilities context initialized");

    return txtout_ctx;
}

static void st_txtout_quit(st_txtoutctx_t *txtout_ctx) {
    ST_LOGGERCTX_CALL(txtout_ctx->logger_ctx, info,
     "txtout_simple: Text output utilities context destroyed");
    free(txtout_ctx);
}

static unsigned st_txtout_get_text_width(const st_txtoutctx_t *txtout_ctx, 
 const st_font_t *font, const char *text) {
    unsigned           width = 0;
    const st_sprite_t *last_codepoint_sprite;
    int                last_codepoint_advance;

    if (!font || !text || !*text)
        return 0;

    do {
        int64_t codepoint = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx, to_codepoint, 
         text);
        if (codepoint == -1) {
            if (width == 0)
                return 0;
            else
                break;
        }

        last_codepoint_sprite = ST_FONT_CALL(font, get_sprite, codepoint);
        last_codepoint_advance = ST_FONT_CALL(font, get_xadvance, codepoint);

        width += ST_FONT_CALL(font, get_xadvance, codepoint);
        text = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx, str_advance, text, 1);
    } while (text && *text);

    if (last_codepoint_sprite)
        width += -last_codepoint_advance + ST_SPRITE_CALL(last_codepoint_sprite, 
         get_width);

    return width;
}

static const char *next_whitespace(const st_txtoutctx_t *txtout_ctx, 
 const char **after_whitespace, const char *text) {
    const char *whitespace = NULL;

    if (after_whitespace)
        *after_whitespace = NULL;
    
    while (text && *text) {
        if (isspace((unsigned char)*text)) {
            whitespace = text;
            break;
        }

        text = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx, str_advance, text, 1);
    }

    if (after_whitespace && text && whitespace) {
        while (text && *text) {
            if (!isspace((unsigned char)*text)) {
                *after_whitespace = text;
                break;
            }

            text = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx, str_advance, text, 1);
        }
    }

    return whitespace;
}

/* vibecoded */
static const char *st_txtout_get_subtext_after_wrap(
 const st_txtoutctx_t *txtout_ctx, size_t *codepoints_before_wrap,
 const st_font_t *font, const char *text, unsigned max_width) {
    const char *line_start;
    const char *next_ws;
    const char *after_next_ws;
    const char *last_ws = NULL;
    const char *after_last_ws = NULL;
    size_t      codepoints = 0;
    size_t      codepoints_before_last_ws = 0;
    unsigned    width = 0;

    if (codepoints_before_wrap)
        *codepoints_before_wrap = 0;

    if (!font || !text || !*text || !max_width)
        return NULL;

    line_start = text;
    next_ws = next_whitespace(txtout_ctx, &after_next_ws, line_start);

    while (text && *text) {
        int64_t            codepoint;
        const st_sprite_t *sprite;
        int                xadvance;
        unsigned           glyph_width;
        unsigned           glyph_right;
        const char        *next_char;

        codepoint = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx, to_codepoint, text);
        if (codepoint == -1) {
            if (codepoints_before_wrap)
                *codepoints_before_wrap = codepoints;

            return NULL;
        }

        sprite = ST_FONT_CALL(font, get_sprite, (uint32_t)codepoint);
        xadvance = ST_FONT_CALL(font, get_xadvance, (uint32_t)codepoint);
        glyph_width = sprite
            ? ST_SPRITE_CALL(sprite, get_width)
            : (unsigned)(xadvance > 0 ? xadvance : 0);
        glyph_right = width + glyph_width;

        if (glyph_right > max_width) {
            if (last_ws && after_last_ws) {
                if (codepoints_before_wrap)
                    *codepoints_before_wrap = codepoints_before_last_ws;

                return after_last_ws;
            }

            if (codepoints_before_wrap)
                *codepoints_before_wrap = codepoints;

            return text;
        }

        if (next_ws && text == next_ws) {
            last_ws = next_ws;
            after_last_ws = after_next_ws;
            codepoints_before_last_ws = codepoints;
            next_ws = next_whitespace(txtout_ctx, &after_next_ws,
             after_next_ws);
        }

        width += (unsigned)(xadvance > 0 ? xadvance : 0);
        codepoints++;
        next_char = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx, str_advance, text, 1);
        if (!next_char)
            break;
        text = next_char;
    }

    if (codepoints_before_wrap)
        *codepoints_before_wrap = codepoints;

    return NULL;
}

/* vibecoded */
static ssize_t st_txtout_get_output_data(const st_txtoutctx_t *txtout_ctx, 
 st_txtoutentry_t *dst, const st_font_t *font, const char *text, 
 size_t codepoints, float x, float y, float hscale, float vscale, float pivot_x, 
 float pivot_y) {
    size_t cursor_i = 0;
    float  base_x = x - pivot_x * hscale;
    float  base_y = y - pivot_y * vscale;
    float  cursor_x = base_x;

    if (!dst || !font || !text || !*text || !codepoints)
        return -1;

    while (cursor_i < codepoints && text && *text) {
        int64_t            codepoint = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx,
         to_codepoint, text);
        const st_sprite_t *sprite;
        int                xoffset;
        int                yoffset;
        int                xadvance;
        const char        *next_char;

        if (codepoint == -1)
            break;

        sprite = ST_FONT_CALL(font, get_sprite, (uint32_t)codepoint);
        xoffset = ST_FONT_CALL(font, get_xoffset, (uint32_t)codepoint);
        yoffset = ST_FONT_CALL(font, get_yoffset, (uint32_t)codepoint);
        xadvance = ST_FONT_CALL(font, get_xadvance, (uint32_t)codepoint);

        dst[cursor_i].sprite = sprite;
        dst[cursor_i].x = cursor_x + (float)xoffset * hscale;
        dst[cursor_i].y = base_y + (float)yoffset * vscale;
        dst[cursor_i].hscale = hscale;
        dst[cursor_i].vscale = vscale;

        cursor_x += (float)xadvance * hscale;
        cursor_i++;
        next_char = ST_UTF8CTX_CALL(txtout_ctx->utf8_ctx, str_advance, text, 1);
        if (!next_char)
            break;
        text = next_char;
    }

    return (ssize_t)cursor_i;
}
