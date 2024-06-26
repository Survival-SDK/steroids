#include "libpng.h"

#include <errno.h>
#include <stdio.h>

#include <png.h>

#define ERRMSGBUF_SIZE 128
#define CODEC_PRIORITY 100

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

ST_MODULE_DEF_GET_FUNC(bmcodec_libpng)
ST_MODULE_DEF_INIT_FUNC(bmcodec_libpng)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_bmcodec_libpng_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_bmcodec_import_functions(st_modctx_t *bmcodec_ctx,
 st_modctx_t *bitmap_ctx, st_modctx_t *logger_ctx) {
    st_bmcodec_libpng_t *module = bmcodec_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "bmcodec_libpng: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("bmcodec_libpng", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("bmcodec_libpng", logger, info);
    ST_LOAD_FUNCTION_FROM_CTX("bmcodec_libpng", logger, warning);

    ST_LOAD_FUNCTION_FROM_CTX("bmcodec_libpng", bitmap, import);

    return true;
}

static st_modctx_t *st_bmcodec_init(st_modctx_t *bitmap_ctx,
 st_modctx_t *logger_ctx) {
    st_modctx_t          *bmcodec_ctx;
    st_bmcodec_libpng_t  *module;

    bmcodec_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_bmcodec_libpng_data, sizeof(st_bmcodec_libpng_t));

    if (!bmcodec_ctx)
        return NULL;

    bmcodec_ctx->funcs = &st_bmcodec_libpng_funcs;

    module = bmcodec_ctx->data;
    module->bitmap.ctx = bitmap_ctx;
    module->logger.ctx = logger_ctx;

    if (!st_bmcodec_import_functions(bmcodec_ctx, bitmap_ctx, logger_ctx))
        goto fail;

    module->logger.info(module->logger.ctx,
     "bmcodec_libpng: libpng codec initialized");

    return bmcodec_ctx;

fail:
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, bmcodec_ctx);

    return NULL;
}

static void st_bmcodec_quit(st_modctx_t *bmcodec_ctx) {
    st_bmcodec_libpng_t *module = bmcodec_ctx->data;

    module->logger.info(module->logger.ctx,
     "bmcodec_libpng: libpng codec destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, bmcodec_ctx);
}

static int st_bmcodec_get_priority(st_modctx_t *bmcodec_ctx) {
    return CODEC_PRIORITY;
}

static st_bitmap_t *st_bmcodec_load(st_modctx_t *bmcodec_ctx,
 const char *filename) {
    st_bmcodec_libpng_t *module = bmcodec_ctx->data;
    png_image            image = {0};
    png_bytep            buffer;
    st_bitmap_t         *result = NULL;
    char                 errbuf[ERRMSGBUF_SIZE];

    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_file(&image, filename))
        return NULL;

    image.format = PNG_FORMAT_RGBA;

    buffer = malloc(PNG_IMAGE_SIZE(image)); // NOLINT(hicpp-signed-bitwise)
    if (!buffer) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.info(module->logger.ctx,
             "bmcodec_libpng: Unable to allocate memory for read buffer: %s",
             errbuf);

        goto malloc_fail;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.warning(module->logger.ctx,
             "bmcodec_libpng: Unable to decode PNG file \"%s\": %s", filename,
             errbuf);

        goto read_fail;
    }

    result = module->bitmap.import(module->bitmap.ctx, buffer, image.width,
     image.height, PF_RGBA);

read_fail:
    free(buffer);
malloc_fail:
    png_image_free(&image);

    return result;
}

static st_bitmap_t *st_bmcodec_memload(st_modctx_t *bmcodec_ctx,
 const void *data, size_t size) {
    st_bmcodec_libpng_t *module = bmcodec_ctx->data;
    png_image            image = {0};
    png_bytep            buffer;
    st_bitmap_t         *result = NULL;
    char                 errbuf[ERRMSGBUF_SIZE];

    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&image, data, size))
        return NULL;

    image.format = PNG_FORMAT_RGBA;

    buffer = malloc(PNG_IMAGE_SIZE(image)); // NOLINT(hicpp-signed-bitwise)
    if (!buffer) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.info(module->logger.ctx,
             "bmcodec_libpng: Unable to allocate memory for read buffer: %s",
             errbuf);

        goto malloc_fail;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "bmcodec_libpng: Unable to decode PNG data: %s", errbuf);

        goto read_fail;
    }

    result = module->bitmap.import(module->bitmap.ctx, buffer, image.width,
     image.height, PF_RGBA);

read_fail:
    free(buffer);
malloc_fail:
    png_image_free(&image);

    return result;
}

static bool st_bmcodec_save(st_modctx_t *bmcodec_ctx, const st_bitmap_t *bitmap,
 const char *filename, const char *format) {
    _Static_assert(PF_MAX == PF_RGBA + 1, "New pixel format available");

    st_bmcodec_libpng_t *module;
    png_image            image;

    if (!bmcodec_ctx || !bitmap || !filename || !format
     || strcmp(format, "png") != 0)
        return false;

    module = bmcodec_ctx->data;
    image  = (png_image){
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

static bool st_bmcodec_memsave(st_modctx_t *bmcodec_ctx, void *dst,
 size_t *size, const st_bitmap_t *bitmap, const char *format) {
    _Static_assert(PF_MAX == PF_RGBA + 1, "New pixel format available");

    st_bmcodec_libpng_t *module;
    png_image            image;

    if (!bmcodec_ctx || !dst || !size || !bitmap || strcmp(format, "png") != 0)
        return false;

    module = bmcodec_ctx->data;
    image  = (png_image){
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
