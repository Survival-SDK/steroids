#include "simple.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/fontloader.h"

#define LOADER_NAME_SIZE 32

static st_fontctx_t *st_font_init(const st_param_t params[]);
static void st_font_quit(st_fontctx_t *font_ctx);

static st_font_t *st_font_load(st_fontctx_t *font_ctx, const char *filename);
static st_font_t *st_font_memload(st_fontctx_t *font_ctx, const void *data, 
 size_t size);
static st_font_t *st_font_create_empty(st_fontctx_t *font_ctx, 
 unsigned line_height, unsigned base, unsigned texture_width, 
 unsigned texture_height, unsigned pages_count);
static bool st_font_add_page(st_font_t *font, unsigned index, 
 const char *filename);
static bool st_font_memadd_page(st_font_t *font, unsigned index, 
 const void *data, size_t size);
static bool st_font_add_char(st_font_t *font, uint32_t ucs4code, 
 unsigned subimage_x, unsigned subimage_y, unsigned subimage_width, 
 unsigned subimage_height, int xoffset, int yoffset, int xadvance, 
 unsigned page);

static unsigned st_font_get_line_height(const st_font_t *font);
static unsigned st_font_get_base(const st_font_t *font);
static const st_sprite_t *st_font_get_sprite(const st_font_t *font, 
 uint32_t ucs4code);
static int st_font_get_xoffset(const st_font_t *font, uint32_t ucs4code);
static int st_font_get_yoffset(const st_font_t *font, uint32_t ucs4code);
static int st_font_get_xadvance(const st_font_t *font, uint32_t ucs4code);

static st_fontctx_funcs_t fontctx_funcs = {
    ST_MODCTX_FUNCS,
    .load         = st_font_load,
    .memload      = st_font_memload,
    .create_empty = st_font_create_empty,
};

static st_font_funcs_t font_funcs = {
    ST_OBJECT_FUNCS,
    .add_page        = st_font_add_page,
    .memadd_page     = st_font_memadd_page,
    .add_char        = st_font_add_char,
    .get_line_height = st_font_get_line_height,
    .get_base        = st_font_get_base,
    .get_sprite      = st_font_get_sprite,
    .get_xoffset     = st_font_get_xoffset,
    .get_yoffset     = st_font_get_yoffset,
    .get_xadvance    = st_font_get_xadvance,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "htable", NULL, },
    { "sprite", NULL, },
    { "texture", NULL, },
    {0},
};

st_moddata_t *st_module_font_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("font", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_font_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_font_simple_init(modsmgr);
}
#endif

static void st_font_init_loaders(st_fontctx_t *font_ctx) {
    char  loader_names[ST_FONT_LOADERS_MAX][LOADER_NAME_SIZE] = {0};
    char *ploader_names[ST_FONT_LOADERS_MAX];

    for (size_t i = 0; i < ST_FONT_LOADERS_MAX; i++)
        ploader_names[i] = loader_names[i];

    font_ctx->font_loaders_count = 0;

    ST_LOGGERCTX_CALL(font_ctx->logger_ctx, info,
     "font_simple: Searching font loaders");

    ST_MODSMGR_CALL(font_ctx->modsmgr, get_module_names, ploader_names,
     ST_FONT_LOADERS_MAX, LOADER_NAME_SIZE, "fontloader");

    for (size_t i = 0; i < ST_FONT_LOADERS_MAX; i++) {
        st_ctx_ctor_t       ctx_ctor;
        st_fontloaderctx_t *ctx;
        char               *loader_name = ploader_names[i];

        if (!*loader_name)
            break;

        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, info,
         "font_simple: Found module \"fontloader_%s\"", loader_name);

        ctx_ctor = ST_MODSMGR_CALL(font_ctx->modsmgr, get_ctor,
         "fontloader", loader_name);

        if (!ctx_ctor) {
            ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
             "font_simple: Unable to get ctor from module \"fontloader_%s\"",
             loader_name);

            continue;
        }

        ctx = ctx_ctor((st_params_t){
            {"modsmgr", (uintptr_t)font_ctx->modsmgr},
            {"font_ctx", (uintptr_t)font_ctx},
        });
        if (!ctx)
            continue;

        font_ctx->font_loaders[font_ctx->font_loaders_count++] = ctx;
    }
}

static st_fontctx_t *st_font_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_htablectx_t *htable_ctx = (st_htablectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "htable", NULL);
    st_texturectx_t *texture_ctx = (st_texturectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "texture", NULL);
    st_spritectx_t *sprite_ctx = (st_spritectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "sprite", NULL);
    st_fontctx_t  *font_ctx = (st_fontctx_t *)st_modctx_new("font",
     "simple", sizeof(st_fontctx_t), NULL, &fontctx_funcs,
     (st_object_dtor_t)st_font_quit);

    if (!font_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "font_simple: unable to create new font ctx object");

        return NULL;
    }

    font_ctx->modsmgr     = modsmgr;
    font_ctx->logger_ctx  = logger_ctx;
    font_ctx->htable_ctx  = htable_ctx;
    font_ctx->texture_ctx = texture_ctx;
    font_ctx->sprite_ctx  = sprite_ctx;

    st_font_init_loaders(font_ctx);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "font_simple: Font mgr initialized.");

    return font_ctx;
}

static void st_font_quit(st_fontctx_t *font_ctx) {
    for (size_t i = 0; i < font_ctx->font_loaders_count; i++)
        ST_FONTLOADERCTX_CALL(font_ctx->font_loaders[i], destroy);

    ST_LOGGERCTX_CALL(font_ctx->logger_ctx, info,
     "font_simple: Font mgr destroyed");
    free(font_ctx);
}

static st_font_t *st_font_load(st_fontctx_t *font_ctx, const char *filename) {
    for (size_t i = 0; i < font_ctx->font_loaders_count; i++) {
        st_font_t *font = ST_FONTLOADERCTX_CALL(font_ctx->font_loaders[i], 
         load, filename);
        if (font)
            return font;
    }

    ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
     "font_simple: No suitable loader for loading font \"%s\"", 
     filename);

    return NULL;
}

static st_font_t *st_font_memload(st_fontctx_t *font_ctx,
 const void *data, size_t size) {
    for (size_t i = 0; i < font_ctx->font_loaders_count; i++) {
        st_font_t *font = ST_FONTLOADERCTX_CALL(font_ctx->font_loaders[i], 
         memload, data, size);
        if (font)
            return font;
    }

    ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
     "font_simple: No suitable loader for loading font");

    return NULL;
}

static void st_font_destroy(st_font_t *font) {
    ST_HTABLE_CALL(font->chars, destroy);
    
    for (size_t i = 0; i < font->pages_count; i++) {
        if (font->pages[i])
            ST_TEXTURE_CALL(font->pages[i], destroy);
    }

    free(font);
}

static uint32_t hash_codepoint(const void *ptr) {
    /* Workaround: keys are stored as (ucs4code + 1) to avoid NULL key for
     * U+0000, because hash_table asserts key != NULL. */
    return (uint32_t)(uintptr_t)ptr;
}

static bool codepoints_equal(const void *left, const void *right) {
    return (uint32_t)(uintptr_t)left == (uint32_t)(uintptr_t)right;
}

static const void *codepoint_to_key(uint32_t ucs4code) {
    /* Workaround: shift codepoint by +1 so U+0000 does not become NULL key. */
    return (void *)(uintptr_t)(ucs4code + 1u);
}

static void fontchar_destroy(void *ptr) {
    st_fontchar_t *fontchar = (st_fontchar_t *)ptr;
    if (fontchar->handle)
        ST_SPRITE_CALL(fontchar->handle, destroy);
    free(fontchar);
}

static st_font_t *st_font_create_empty(st_fontctx_t *font_ctx, 
 unsigned line_height, unsigned base, unsigned texture_width, 
 unsigned texture_height, unsigned pages_count) {
    st_font_t *font;

    font = (st_font_t *)st_object_new(
     sizeof(st_font_t) + sizeof(st_fontpage_t *) * pages_count, &font_funcs, 
     (st_object_dtor_t)st_font_destroy, (st_object_t *)font_ctx);
    if (!font)
        return NULL;

    font->line_height = line_height;
    font->base = base;
    font->texture_width = texture_width;
    font->texture_height = texture_height;
    font->pages_count = pages_count;

    for (unsigned i = 0; i < pages_count; i++)
        font->pages[i] = NULL;

    font->chars = ST_HTABLECTX_CALL(font_ctx->htable_ctx, create, 
     hash_codepoint, codepoints_equal, NULL, fontchar_destroy);
    if (!font->chars) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Unable to create hash table for chars");

        free(font);

        return NULL;
    }

    return font;
}

static bool st_font_add_page(st_font_t *font, unsigned index, 
 const char *filename) {
    const st_fontctx_t *font_ctx = (const st_fontctx_t *)ST_FONT_CALL(font, 
     get_owner);

    if (index >= font->pages_count) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Page index %u is out of range", index);

        return false;
    }

    if (font->pages[index]) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Page %u already exists", index);

        return false;
    }

    font->pages[index] = ST_TEXTURECTX_CALL(font_ctx->texture_ctx, load, 
     filename);

    if (!font->pages[index]) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Unable to load texture \"%s\" for page %u", filename, 
         index);

        return false;
    }

    return true;
}

static bool st_font_memadd_page(st_font_t *font, unsigned index, 
 const void *data, size_t size) {
    const st_fontctx_t *font_ctx = (const st_fontctx_t *)ST_FONT_CALL(font, 
     get_owner);

    if (index >= font->pages_count) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Page index %u is out of range", index);

        return false;
    }

    if (font->pages[index]) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Page %u already exists", index);

        return false;
    }

    font->pages[index] = ST_TEXTURECTX_CALL(font_ctx->texture_ctx, memload, 
     data, size);

    if (!font->pages[index]) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Unable to load texture for page %u", index);

        return false;
    }

    return true;
}

static bool st_font_add_char(st_font_t *font, uint32_t ucs4code, 
 unsigned subimage_x, unsigned subimage_y, unsigned subimage_width, 
 unsigned subimage_height, int xoffset, int yoffset, int xadvance, 
 unsigned page) {
    st_fontchar_t      *fontchar;
    const st_fontctx_t *font_ctx = (const st_fontctx_t *)ST_FONT_CALL(font, 
     get_owner);

    if (page >= font->pages_count) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Page index %u is out of range", page);

        return false;
    }

    if (!font->pages[page]) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Page %u is not loaded", page);

        return false;
    }

    fontchar = malloc(sizeof(st_fontchar_t));
    if (!fontchar) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Unable to allocate memory for fontchar");

        return false;
    }

    fontchar->handle = ST_SPRITECTX_CALL(font_ctx->sprite_ctx, 
     from_texture_region, font->pages[page], subimage_x, subimage_y, 
     subimage_width, subimage_height);
    if (!fontchar->handle) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Unable to create sprite from texture region for char %u", 
         ucs4code);

        goto sprite_create_fail;
    }

    fontchar->xoffset = xoffset;
    fontchar->yoffset = yoffset;
    fontchar->xadvance = xadvance;

    if (!ST_HTABLE_CALL(font->chars, insert, NULL, codepoint_to_key(ucs4code), 
     fontchar)) {
        ST_LOGGERCTX_CALL(font_ctx->logger_ctx, error,
         "font_simple: Unable to insert char %u into hash table", ucs4code);

        goto htable_insert_fail;
    }

    return true;

htable_insert_fail:
    ST_SPRITE_CALL(fontchar->handle, destroy);

sprite_create_fail:
    free(fontchar);

    return false;
}

static unsigned st_font_get_line_height(const st_font_t *font) {
    return font->line_height;
}

static unsigned st_font_get_base(const st_font_t *font) {
    return font->base;
}

static const st_sprite_t *st_font_get_sprite(const st_font_t *font, 
 uint32_t ucs4code) {
    st_fontchar_t *fontchar = ST_HTABLE_CALL(font->chars, get, 
     codepoint_to_key(ucs4code));
    
    if (!fontchar)
        return NULL;

    return fontchar->handle;
}

static int st_font_get_xoffset(const st_font_t *font, uint32_t ucs4code) {
    st_fontchar_t *fontchar = ST_HTABLE_CALL(font->chars, get, 
    codepoint_to_key(ucs4code));
    
    if (!fontchar)
        return 0;

    return fontchar->xoffset;
}

static int st_font_get_yoffset(const st_font_t *font, uint32_t ucs4code) {
    st_fontchar_t *fontchar = ST_HTABLE_CALL(font->chars, get, 
    codepoint_to_key(ucs4code));
    
    if (!fontchar)
        return 0;

    return fontchar->yoffset;
}

static int st_font_get_xadvance(const st_font_t *font, uint32_t ucs4code) {
    st_fontchar_t *fontchar = ST_HTABLE_CALL(font->chars, get, 
    codepoint_to_key(ucs4code));
    
    if (!fontchar)
        return 0;

    return fontchar->xadvance;
}
