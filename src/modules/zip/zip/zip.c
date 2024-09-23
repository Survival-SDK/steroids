#include "zip.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <zip/zip.h>

#define ERRMSGBUF_SIZE 128

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_zipctx_funcs_t zipctx_funcs = {
    .quit    = st_zip_quit,
    .open    = st_zip_open,
    .memopen = st_zip_memopen,
};

static st_zip_funcs_t zip_funcs = {
    .close             = st_zip_close,
    .get_entries_count = st_zip_get_entries_count,
    .get_entry_name    = st_zip_get_entry_name,
    .get_entry_type    = st_zip_get_entry_type,
    .extract_entry     = st_zip_extract_entry,
};

ST_MODULE_DEF_GET_FUNC(zip_zip)
ST_MODULE_DEF_INIT_FUNC(zip_zip)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_zip_zip_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "zip";
static const char *st_module_name = "zip";

static st_zipctx_t *st_zip_init(st_fsctx_t *fs_ctx,
 struct st_loggerctx_s *logger_ctx, st_pathtoolsctx_t *pathtools_ctx) {
    st_zipctx_t  *zip_ctx = st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_zipctx_t), NULL, &zipctx_funcs);

    if (!zip_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "zip_zip: unable to create new zip ctx object");

        return NULL;
    }

    zip_ctx->fs_ctx        = fs_ctx;
    zip_ctx->logger_ctx    = logger_ctx;
    zip_ctx->pathtools_ctx = pathtools_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "zip_zip: ZIP arhives processor context initialized");

    return zip_ctx;
}

static void st_zip_quit(st_zipctx_t *zip_ctx) {
    ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, info,
     "zip_zip: ZIP arhives processor context destroyed");
    free(zip_ctx);
}

static st_zip_t *st_zip_open(st_zipctx_t *zip_ctx, const char *filename) {
    struct zip_t *handle;
    st_zip_t     *zip;

    handle = zip_open(filename, 0, 'r');
    if (!handle) {
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to open file: %s", filename);

        return NULL;
    }

    zip = malloc(sizeof(st_zip_t));
    if (!zip) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
             "zip_zip: Unable to allocate memory for opened zip structure "
             "while opening file \"%s\": %s", filename, errbuf);

        zip_close(handle);

        return NULL;
    }

    st_object_make(zip, zip_ctx, &zip_funcs);
    zip->handle = handle;
    zip->type   = ST_ZT_FILE;

    return zip;
}

static st_zip_t *st_zip_memopen(st_zipctx_t *zip_ctx, const void *data,
 size_t size) {
    struct zip_t *handle;
    st_zip_t     *zip;

    handle = zip_stream_open(data, size, 0, 'r');
    if (!handle) {
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to open zip from memory");

        return NULL;
    }

    zip = malloc(sizeof(st_zip_t));
    if (!zip) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
             "zip_zip: Unable to allocate memory for opened zip structure: %s",
             errbuf);

        zip_close(handle);

        return NULL;
    }

    st_object_make(zip, zip_ctx, &zip_funcs);
    zip->handle = handle;
    zip->type   = ST_ZT_MEM;

    return zip;
}

static void st_zip_close(st_zip_t *zip) {
    if (zip->type == ST_ZT_FILE)
        zip_close(zip->handle);
    else
        zip_stream_close(zip->handle);

    free(zip);
}

static ssize_t st_zip_get_entries_count(st_zip_t *zip) {
    st_zipctx_t *zip_ctx = st_object_get_owner(zip);
    ssize_t      ret;

    if (!zip || !zip->handle)
        return 0;

    ret = zip_entries_total(zip->handle);
    if (ret < 0) {
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to get entries count: %s", zip_strerror((int)ret));

        return -1;
    }

    return ret;
}

static bool st_zip_get_entry_name(st_zip_t *zip, char *dst, size_t dstsize,
 size_t entrynum) {
    st_zipctx_t *zip_ctx = st_object_get_owner(zip);
    int          ret;
    const char  *entry_name;

    if (!zip || !zip->handle)
        return 0;

    ret = zip_entry_openbyindex(zip->handle, entrynum);
    if (ret < 0) {
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to open entry: %s", zip_strerror(ret));

        return false;
    }

    entry_name = zip_entry_name(zip->handle);
    if (!entry_name) {
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to get entry name");
        zip_entry_close(zip->handle);

        return false;
    }

    ret = snprintf(dst, dstsize, "%s", entry_name);
    if (ret < 0 || (size_t)ret == dstsize)
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to get entry name");

    zip_entry_close(zip->handle);

    return ret > 0 && (size_t)ret <= dstsize;
}

static st_zipentrytype_t st_zip_get_entry_type(st_zip_t *zip, size_t entrynum) {
    st_zipctx_t *zip_ctx = st_object_get_owner(zip);
    int          ret;

    if (!zip || !zip->handle)
        return 0;

    ret = zip_entry_openbyindex(zip->handle, entrynum);
    if (ret < 0) {
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to open entry: %s", zip_strerror(ret));

        return ST_ZET_UNKNOWN;
    }

    ret = zip_entry_isdir(zip->handle);
    if (ret < 0)
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to determine entry type: %s", zip_strerror(ret));

    zip_entry_close(zip->handle);

    return ret < 0 ? ST_ZET_UNKNOWN : (ret ? ST_ZET_DIR : ST_ZET_FILE);
}

static bool st_zip_extract_entry(st_zip_t *zip, size_t entrynum,
 const char *path) {
    st_zipctx_t *zip_ctx = st_object_get_owner(zip);
    char         result_filename[PATH_MAX];
    const char  *presult_filename;
    int          ret;

    if (!zip || !zip->handle)
        return 0;

    ret = zip_entry_openbyindex(zip->handle, entrynum);
    if (ret < 0) {
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to open entry: %s", zip_strerror(ret));

        return false;
    }

    if (ST_FSCTX_CALL(zip_ctx->fs_ctx, get_file_type, path) == ST_FT_DIR) {
        if (!ST_PATHTOOLSCTX_CALL(zip_ctx->pathtools_ctx, concat,
         result_filename, PATH_MAX, path, zip_entry_name(zip->handle))) {
            ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
             "zip_zip: Unable get output filename for entry: %s",
             zip_entry_name(zip->handle));

            zip_entry_close(zip->handle);

            return false;
        }
        presult_filename = result_filename;
    } else {
        presult_filename = path;
    }

    ret = zip_entry_fread(zip->handle, presult_filename);
    if (ret < 0)
        ST_LOGGERCTX_CALL(zip_ctx->logger_ctx, error,
         "zip_zip: Unable to extract entry: %s", zip_strerror(ret));

    zip_entry_close(zip->handle);

    return !ret;
}
