#include "simple.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/bmcodec.h"

#define ERRMSGBUF_SIZE  128
#define CODEC_NAME_SIZE 32
#define CODECS_COUNT    256

static st_bitmapctx_t *st_bitmap_init(const st_param_t params[]);
static void st_bitmap_quit(st_bitmapctx_t *bitmap_ctx);

static st_bitmap_t *st_bitmap_load(st_bitmapctx_t *bitmap_ctx,
 const char *filename);
static st_bitmap_t *st_bitmap_memload(st_bitmapctx_t *bitmap_ctx,
 const void *data, size_t size);
static st_bitmap_t *st_bitmap_import(st_bitmapctx_t *bitmap_ctx,
 const void *data, unsigned width, unsigned height, st_pxfmt_t pixel_format);

static bool st_bitmap_save(const st_bitmap_t *bitmap, const char *filename,
 const char *format);
static bool st_bitmap_memsave(void *dst, size_t *size,
 const st_bitmap_t *bitmap, const char *format);
static const void *st_bitmap_get_data(const st_bitmap_t *bitmap);
static unsigned st_bitmap_get_width(const st_bitmap_t *bitmap);
static unsigned st_bitmap_get_height(const st_bitmap_t *bitmap);
static st_pxfmt_t st_bitmap_get_pixel_format(const st_bitmap_t *bitmap);

static st_bitmapctx_funcs_t bitmapctx_funcs = {
    ST_MODCTX_FUNCS,
    .load    = st_bitmap_load,
    .memload = st_bitmap_memload,
    .import  = st_bitmap_import,
};

static st_bitmap_funcs_t bitmap_funcs = {
    ST_OBJECT_FUNCS,
    .save             = st_bitmap_save,
    .memsave          = st_bitmap_memsave,
    .get_data         = st_bitmap_get_data,
    .get_width        = st_bitmap_get_width,
    .get_height       = st_bitmap_get_height,
    .get_pixel_format = st_bitmap_get_pixel_format,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_bitmap_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("bitmap", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_bitmap_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_bitmap_simple_init(modsmgr);
}
#endif

static void st_bitmap_init_codecs(st_bitmapctx_t *bitmap_ctx) {
    char  codecs_names[CODEC_NAME_SIZE][CODECS_COUNT] = {0};
    char *pcodecsnames[CODECS_COUNT];

    for (size_t i = 0; i < CODECS_COUNT; i++)
        pcodecsnames[i] = codecs_names[i];

    bitmap_ctx->codecs = st_dlist_create(sizeof(st_bmcodecctx_t *),
     st_object_free_by_ptr);
    if (!bitmap_ctx->codecs) {
        ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
         "bitmap_simple: Unable to create list of bitmap codecs");

        return;
    }

    ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, info,
     "bitmap_simple: Searching bitmap codecs");

    ST_MODSMGR_CALL(bitmap_ctx->modsmgr, get_module_names, pcodecsnames,
     CODECS_COUNT, CODEC_NAME_SIZE, "bmcodec");

    for (size_t i = 0; i < CODECS_COUNT; i++) {
        st_ctx_ctor_t    ctx_ctor;
        st_bmcodecctx_t *ctx;
        char            *codec_name = pcodecsnames[i];

        if (!*codec_name)
            break;

        ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, info,
         "bitmap_simple: Found module \"bmcodec_%s\"", codec_name);

        ctx_ctor = ST_MODSMGR_CALL(bitmap_ctx->modsmgr, get_ctor,
         "bmcodec", codec_name);

        if (!ctx_ctor) {
            ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
             "bitmap_simple: Unable to get ctor from module \"bmcodec_%s\"",
             codec_name);

            continue;
        }

        ctx = ctx_ctor((st_params_t){
            {"modsmgr", (uintptr_t)bitmap_ctx->modsmgr},
            {"bitmap_ctx", (uintptr_t)bitmap_ctx},
        });
        if (!ctx)
            continue;

        if (!st_dlist_push_back(bitmap_ctx->codecs, &ctx)) {
            ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
             "bitmap_simple: Unable to create entry node for module "
             "\"bmcodec_%s\"", codec_name);
            ST_BMCODECCTX_CALL(ctx, destroy);

            continue;
        }
    }
}

static st_bitmapctx_t *st_bitmap_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_bitmapctx_t *bitmap_ctx = (st_bitmapctx_t *)st_modctx_new("bitmap",
     "simple", sizeof(st_bitmapctx_t), NULL, &bitmapctx_funcs,
     (st_object_dtor_t)st_bitmap_quit);

    if (!bitmap_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "bitmap_simple: unable to create new bitmap ctx object");

        return NULL;
    }

    bitmap_ctx->modsmgr    = modsmgr;
    bitmap_ctx->logger_ctx = logger_ctx;

    st_bitmap_init_codecs(bitmap_ctx);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "bitmap_simple: Bitmaps mgr initialized.");

    return bitmap_ctx;
}

static void st_bitmap_quit(st_bitmapctx_t *bitmap_ctx) {
    st_dlist_destroy(bitmap_ctx->codecs);

    ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, info,
     "bitmap_simple: Bitmaps mgr destroyed");
    free(bitmap_ctx);
}

static st_bitmap_t *st_bitmap_load(st_bitmapctx_t *bitmap_ctx,
 const char *filename) {
    st_dlnode_t *node = st_dlist_get_head(bitmap_ctx->codecs);

    while (node) {
        st_bmcodecctx_t *codec = st_dlist_export_ptr(node);
        st_bitmap_t     *bitmap = ST_BMCODECCTX_CALL(codec, load, filename);

        if (bitmap)
            return bitmap;

        node = st_dlist_get_next(node);
    }

    ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
     "bitmap_simple: No suitable codec for loading bitmap \"%s\"", filename);

    return NULL;
}

static st_bitmap_t *st_bitmap_memload(st_bitmapctx_t *bitmap_ctx,
 const void *data, size_t size) {
    st_dlnode_t *node = st_dlist_get_head(bitmap_ctx->codecs);

    while (node) {
        st_bmcodecctx_t *codec = st_dlist_export_ptr(node);
        st_bitmap_t     *bitmap = ST_BMCODECCTX_CALL(codec, memload, data,
         size);

        if (bitmap)
            return bitmap;

        node = st_dlist_get_next(node);
    }

    ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
     "bitmap_simple: No suitable codec for loading bitmap");

    return NULL;
}

static bool st_bitmap_save(const st_bitmap_t *bitmap, const char *filename,
 const char *format) {
    st_bitmapctx_t *bitmap_ctx = (st_bitmapctx_t *)st_object_get_owner(
     (const st_object_t *)bitmap);
    st_dlnode_t    *node = st_dlist_get_head(bitmap_ctx->codecs);

    while (node) {
        st_bmcodecctx_t *codec = st_dlist_export_ptr(node);

        if (ST_BMCODECCTX_CALL(codec, save, bitmap, filename, format))
            return true;

        node = st_dlist_get_next(node);
    }

    return false;
}

static bool st_bitmap_memsave(void *dst, size_t *size,
 const st_bitmap_t *bitmap, const char *format) {
    st_bitmapctx_t *bitmap_ctx = (st_bitmapctx_t *)st_object_get_owner(
     (const st_object_t *)bitmap);
    st_dlnode_t    *node = st_dlist_get_head(bitmap_ctx->codecs);

    while (node) {
        st_bmcodecctx_t *codec = st_dlist_export_ptr(node);

        if (ST_BMCODECCTX_CALL(codec, memsave, dst, size, bitmap, format))
            return true;

        node = st_dlist_get_next(node);
    }

    return false;
}

static unsigned pxfmt_to_bytes_per_pixel(st_pxfmt_t pixel_format) {
    switch (pixel_format) {
        case PF_RGBA:
            return 4;
        case PF_UNKNOWN:
        default:
            return 0;
    }
}

static st_bitmap_t *st_bitmap_import(st_bitmapctx_t *bitmap_ctx,
 const void *data, unsigned width, unsigned height, st_pxfmt_t pixel_format) {
    unsigned     bytes_per_pixel;
    size_t       data_size;
    st_bitmap_t *bitmap;

    if (width == 0 || height == 0) {
        ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
         "bitmap_simple: Unable to create bitmap. Incorrect sizes");

        return NULL;
    }

    bytes_per_pixel = pxfmt_to_bytes_per_pixel(pixel_format);
    if (bytes_per_pixel == 0) {
        ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
         "bitmap_simple: Unable to create bitmap. Unsupported pixel format");

        return NULL;
    }

    data_size = bytes_per_pixel * width * height;

    bitmap = (st_bitmap_t *)st_object_new(sizeof(st_bitmap_t) + data_size, 
     &bitmap_funcs, NULL, (st_object_t *)bitmap_ctx);
    if (!bitmap) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(bitmap_ctx->logger_ctx, error,
             "bitmap_simple: Unable to allocate memory for bitmap pixels: %s",
             errbuf);

        return NULL;
    }

    memcpy(bitmap->data, data, data_size);
    bitmap->width = width;
    bitmap->height = height;
    bitmap->pixel_format = (int)pixel_format;

    return bitmap;
}

static const void *st_bitmap_get_data(const st_bitmap_t *bitmap) {
    return bitmap ? bitmap->data : NULL;
}

static unsigned st_bitmap_get_width(const st_bitmap_t *bitmap) {
    return bitmap ? bitmap->width : 0;
}

static unsigned st_bitmap_get_height(const st_bitmap_t *bitmap) {
    return bitmap ? bitmap->height : 0;
}

static st_pxfmt_t st_bitmap_get_pixel_format(const st_bitmap_t *bitmap) {
    return bitmap ? (st_pxfmt_t)bitmap->pixel_format : PF_UNKNOWN;
}
