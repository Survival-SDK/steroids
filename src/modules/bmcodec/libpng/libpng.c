#include "libpng.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ERRMSGBUF_SIZE 128
#define CODEC_PRIORITY 100

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

st_moddata_t *st_module_bmcodec_libpng_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("bmcodec", "libpng", ST_MODULE_TYPE, mod_prereqs,
     st_bmcodec_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_bmcodec_libpng_init(modsmgr);
}
#endif

static st_bmcodecctx_t *st_bmcodec_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    /* We need get bitmap_ctx via params because bitmap_ctx is not available
     * yet as singleton when this context is created */
     st_bitmapctx_t *bitmap_ctx = st_modctx_get_param_as_ptr(params, 
     "bitmap_ctx");
    st_bmcodecctx_t *bmcodec_ctx = (st_bmcodecctx_t *)st_modctx_new("bmcodec",
     "libpng", sizeof(st_bmcodecctx_t), NULL, &bmcodecctx_funcs,
     (st_object_dtor_t)st_bmcodec_quit);

    if (!bmcodec_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "bmcodec_libpng: unable to create new bmcodec ctx object");

        return NULL;
    }

    bmcodec_ctx->modsmgr    = modsmgr;
    bmcodec_ctx->logger_ctx = logger_ctx;
    bmcodec_ctx->bitmap_ctx = bitmap_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "bmcodec_libpng: libpng codec initialized");

    return bmcodec_ctx;
}

static void st_bmcodec_quit(st_bmcodecctx_t *bmcodec_ctx) {
    ST_LOGGERCTX_CALL(bmcodec_ctx->logger_ctx, info,
     "bmcodec_libpng: libpng codec destroyed");
    free(bmcodec_ctx);
}

static int st_bmcodec_get_priority(
 __attribute__((unused)) const st_bmcodecctx_t *bmcodec_ctx) {
    return CODEC_PRIORITY;
}

static st_bitmap_t *st_bmcodec_load(st_bmcodecctx_t *bmcodec_ctx,
 const char *filename) {
    png_image  image = {0};
    png_bytep  buffer;
    st_bitmap_t *result = NULL;
    char       errbuf[ERRMSGBUF_SIZE];

    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, filename))
        return NULL;

    image.format = PNG_FORMAT_RGBA;

    buffer = malloc(PNG_IMAGE_SIZE(image)); // NOLINT(hicpp-signed-bitwise)
    if (!buffer) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(bmcodec_ctx->logger_ctx, info,
             "bmcodec_libpng: Unable to allocate memory for read buffer: %s",
             errbuf);

        goto malloc_fail;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(bmcodec_ctx->logger_ctx, warning,
             "bmcodec_libpng: Unable to decode PNG file \"%s\": %s", filename,
             errbuf);

        goto read_fail;
    }

    result = ST_BITMAPCTX_CALL(bmcodec_ctx->bitmap_ctx, import, buffer,
     image.width, image.height, PF_RGBA);

read_fail:
    free(buffer);
malloc_fail:
    png_image_free(&image);

    return result;
}

static st_bitmap_t *st_bmcodec_memload(st_bmcodecctx_t *bmcodec_ctx,
 const void *data, size_t size) {
    png_image   image = {0};
    png_bytep   buffer;
    st_bitmap_t *result = NULL;
    char        errbuf[ERRMSGBUF_SIZE];

    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&image, data, size))
        return NULL;

    image.format = PNG_FORMAT_RGBA;

    buffer = malloc(PNG_IMAGE_SIZE(image)); // NOLINT(hicpp-signed-bitwise)
    if (!buffer) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(bmcodec_ctx->logger_ctx, info,
             "bmcodec_libpng: Unable to allocate memory for read buffer: %s",
             errbuf);

        goto malloc_fail;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(bmcodec_ctx->logger_ctx, error,
             "bmcodec_libpng: Unable to decode PNG data: %s", errbuf);

        goto read_fail;
    }

    result = ST_BITMAPCTX_CALL(bmcodec_ctx->bitmap_ctx, import, buffer,
     image.width, image.height, PF_RGBA);

read_fail:
    free(buffer);
malloc_fail:
    png_image_free(&image);

    return result;
}

static bool st_bmcodec_save(st_bmcodecctx_t *bmcodec_ctx,
 const st_bitmap_t *bitmap, const char *filename, const char *format) {
    _Static_assert(PF_MAX == PF_RGBA + 1, "New pixel format available");

    png_image image;

    if (!bmcodec_ctx || !bitmap || !filename || !format
     || strcmp(format, "png") != 0)
        return false;

    image = (png_image){
        .version          = PNG_IMAGE_VERSION,
        .opaque           = NULL,
        .width            = ST_BITMAP_CALL(bitmap, get_width),
        .height           = ST_BITMAP_CALL(bitmap, get_height),
        .format           = PNG_FORMAT_RGBA,
        .flags            = 0,
        .colormap_entries = 0,
    };

    return png_image_write_to_file(&image, filename, true,
     ST_BITMAP_CALL(bitmap, get_data), 0, NULL);
}

static bool st_bmcodec_memsave(st_bmcodecctx_t *bmcodec_ctx, void *dst,
 size_t *size, const st_bitmap_t *bitmap, const char *format) {
    _Static_assert(PF_MAX == PF_RGBA + 1, "New pixel format available");

    png_image image;

    if (!bmcodec_ctx || !dst || !size || !bitmap || strcmp(format, "png") != 0)
        return false;

    image = (png_image){
        .version          = PNG_IMAGE_VERSION,
        .opaque           = NULL,
        .width            = ST_BITMAP_CALL(bitmap, get_width),
        .height           = ST_BITMAP_CALL(bitmap, get_height),
        .format           = PNG_FORMAT_RGBA,
        .flags            = 0,
        .colormap_entries = 0,
    };

    return png_image_write_to_memory (&image, dst, size, true,
     ST_BITMAP_CALL(bitmap, get_data), 0, NULL);
}
