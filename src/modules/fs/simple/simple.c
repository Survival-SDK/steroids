#include "simple.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_fsctx_t *st_fs_init(struct st_loggerctx_s *logger_ctx,
 st_pathtoolsctx_t *pathtools_ctx);
static void st_fs_quit(st_fsctx_t *fs_ctx);

static st_filetype_t st_fs_get_file_type(st_fsctx_t *fs_ctx,
 const char *filename);
static bool st_fs_mkdir(st_fsctx_t *fs_ctx, const char *dirname);

static st_fsctx_funcs_t fsctx_funcs = {
    st_modctx_funcs,
    .get_file_type = st_fs_get_file_type,
    .mkdir         = st_fs_mkdir,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    { "pathtools", NULL, },
    {0},
};

st_moddata_t *st_module_fs_simple_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("fs", "simple", ST_MODULE_TYPE, mod_prereqs,
     st_fs_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_fs_simple_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "fs";
static const char *st_module_name = "simple";

static st_fsctx_t *st_fs_init(struct st_loggerctx_s *logger_ctx,
 st_pathtoolsctx_t *pathtools_ctx) {
    st_fsctx_t *fs_ctx = (st_fsctx_t *)st_modctx_new("fs", "simple",
    sizeof(st_fsctx_t), NULL, &fsctx_funcs, (st_object_dtor_t)st_fs_quit);

    if (!fs_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "fs_simple: unable to create new fs ctx object");

        return NULL;
    }

    fs_ctx->logger_ctx = logger_ctx;
    fs_ctx->pathtools_ctx = pathtools_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "fs_simple: file system manager context initialized");

    return fs_ctx;
}

static void st_fs_quit(st_fsctx_t *fs_ctx) {
    ST_LOGGERCTX_CALL(fs_ctx->logger_ctx, info,
     "fs_simple: file system manager context destroyed");
    free(fs_ctx);
}

static st_filetype_t st_fs_get_file_type(
 __attribute__((unused)) st_fsctx_t *fs_ctx, const char *filename) {
    struct stat path_stat;

    if (stat(filename, &path_stat) == -1)
        return ST_FT_UNKNOWN;

    switch ((unsigned)path_stat.st_mode & (unsigned)S_IFMT) {
        case S_IFREG:
            return ST_FT_REG;
        case S_IFDIR:
            return ST_FT_DIR;
        case S_IFCHR:
            return ST_FT_CHR;
        case S_IFBLK:
            return ST_FT_BLK;
        case S_IFIFO:
            return ST_FT_FIFO;
        case S_IFLNK:
            return ST_FT_LINK;
        case S_IFSOCK:
            return ST_FT_SOCK;
        default:
            return ST_FT_UNKNOWN;
    }
}

static bool st_fs_mkdir(st_fsctx_t *fs_ctx, const char *dirname) {
    char  path[PATH_MAX] = "";
    char *ch = path;
    bool  last_is_slash;

    if (!ST_PATHTOOLSCTX_CALL(fs_ctx->pathtools_ctx, resolve, path, PATH_MAX,
     dirname)) {
        ST_LOGGERCTX_CALL(fs_ctx->logger_ctx, error,
         "fs_simple: Unable to get resolved path for directory \"%s\"",
         dirname);

        return false;
    }

    if (path[0] && !path[1]) {
        ST_LOGGERCTX_CALL(fs_ctx->logger_ctx, error,
         "fs_simple: Unable to create directory with name \"%s\"", path);

        return false;
    }

    if (*ch == '/')
        ch++;

    do {
        struct stat unused;

        while(*ch && *ch != '/')
            ch++;

        last_is_slash = *ch == '/';

        *ch = '\0';

        if (path[0] != '.' && path[1] && stat(path, &unused) != 0 &&
         mkdir(path, S_IRWXU | S_IRGRP | S_IROTH) == -1) { // NOLINT(hicpp-signed-bitwise)
            ST_LOGGERCTX_CALL(fs_ctx->logger_ctx, error,
             "fs_simple: Unable to create directory \"%s\"", path);

            return false;
        }

        if (last_is_slash)
            *ch = '/';

        ch++;
    } while (last_is_slash);

    return true;
}
