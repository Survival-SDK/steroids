#include "stb_image.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include <stb_image.h>
#pragma GCC diagnostic pop

#define REQ_CHANNELS   4
#define CODEC_PRIORITY 50

static st_bmcodecctx_t *st_bmcodec_init(const st_param_t params[]);
static void st_bmcodec_quit(st_bmcodecctx_t *bmcodec_ctx);

static int st_bmcodec_get_priority(const st_bmcodecctx_t *bmcodec_ctx);
static st_bitmap_t *st_bmcodec_load(st_bmcodecctx_t *bmcodec_ctx,
 const char *filename);
static st_bitmap_t *st_bmcodec_memload(st_bmcodecctx_t *bmcodec_ctx,
 const void *data, size_t size);
static bool st_bmcodec_save(st_bmcodecctx_t *bmcodec_ctx,
 const st_bitmap_t *bitmap, const char *filename, const char *format);
static bool st_bmcodec_memsave(st_bmcodecctx_t *bmcodec_ctx, void *dst,
 size_t *size, const st_bitmap_t *bitmap, const char *format);

static st_bmcodecctx_funcs_t bmcodecctx_funcs = {
    ST_MODCTX_FUNCS,
    .get_priority = st_bmcodec_get_priority,
    .load         = st_bmcodec_load,
    .memload      = st_bmcodec_memload,
    .save         = st_bmcodec_save,
    .memsave      = st_bmcodec_memsave,
};

static const st_modprerq_t mod_prereqs[] = {
    { "bitmap", NULL, },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_bmcodec_stb_image_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("bmcodec", "stb_image", ST_MODULE_TYPE, mod_prereqs,
     st_bmcodec_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_bmcodec_stb_image_init(modsmgr);
}
#endif

static st_bmcodecctx_t *st_bmcodec_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_bitmapctx_t *bitmap_ctx = (st_bitmapctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "bitmap", NULL);
    st_bmcodecctx_t *bmcodec_ctx = (st_bmcodecctx_t *)st_modctx_new("bmcodec",
     "stb_image", sizeof(st_bmcodecctx_t), NULL, &bmcodecctx_funcs,
     (st_object_dtor_t)st_bmcodec_quit);

    if (!bmcodec_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "bmcodec_stb_image: unable to create new bmcodec ctx object");

        return NULL;
    }

    bmcodec_ctx->modsmgr    = modsmgr;
    bmcodec_ctx->logger_ctx = logger_ctx;
    bmcodec_ctx->bitmap_ctx = bitmap_ctx;

    stbi_set_unpremultiply_on_load(true);
    stbi_convert_iphone_png_to_rgb(true);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "bmcodec_stb_image: stb_image codec initialized");

    return bmcodec_ctx;
}

static void st_bmcodec_quit(st_bmcodecctx_t *bmcodec_ctx) {
    ST_LOGGERCTX_CALL(bmcodec_ctx->logger_ctx, info,
     "bmcodec_stb_image: stb_image codec destroyed");
    free(bmcodec_ctx);
}

static int st_bmcodec_get_priority(
 __attribute__((unused)) const st_bmcodecctx_t *bmcodec_ctx) {
    return CODEC_PRIORITY;
}

static st_bitmap_t *st_bmcodec_load(st_bmcodecctx_t *bmcodec_ctx,
 const char *filename) {
    st_bitmap_t *result = NULL;
    int          width;
    int          height;
    void        *buffer = stbi_load(filename, &width, &height, NULL,
     REQ_CHANNELS);

    if (!buffer)
        return NULL;

    result = ST_BITMAPCTX_CALL(bmcodec_ctx->bitmap_ctx, import, buffer,
     (unsigned)width, (unsigned)height, PF_RGBA);

    stbi_image_free(buffer);

    return result;
}

static st_bitmap_t *st_bmcodec_memload(st_bmcodecctx_t *bmcodec_ctx,
 const void *data, size_t size) {
    st_bitmap_t *result = NULL;
    int          width;
    int          height;
    void        *buffer = stbi_load_from_memory(data, (int)size, &width,
     &height, NULL, REQ_CHANNELS);

    if (!buffer)
        return NULL;

    result = ST_BITMAPCTX_CALL(bmcodec_ctx->bitmap_ctx, import, buffer,
     (unsigned)width, (unsigned)height, PF_RGBA);

    stbi_image_free(buffer);

    return result;
}

static bool st_bmcodec_save(__attribute__((unused)) st_bmcodecctx_t *bmcodec_ctx,
 __attribute__((unused)) const st_bitmap_t *bitmap,
 __attribute__((unused)) const char *filename,
 __attribute__((unused)) const char *format) {
    return false; /* Not supported by implementation */
}

static bool st_bmcodec_memsave(__attribute__((unused)) st_bmcodecctx_t *bmcodec_ctx,
 __attribute__((unused)) void *dst,
 __attribute__((unused)) size_t *size,
 __attribute__((unused)) const st_bitmap_t *bitmap,
 __attribute__((unused)) const char *format) {
    return false; /* Not supported by implementation */
}
