#include "cwalk.h"

#include <ctype.h>
#include <stdio.h>

#include <cwalk.h>

typedef enum {
    ST_PT_IRI,
    ST_PT_ABSOLUTE,
    ST_PT_RELATIVE,
    ST_PT_NOT_PATH,
} st_pathtype_t;

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static void st_pathtools_quit(st_pathtoolsctx_t *pathtools_ctx);

static bool st_pathtools_resolve(st_pathtoolsctx_t *pathtools_ctx, char *dst,
 size_t dstsize, const char *path);
static bool st_pathtools_get_parent_dir(st_pathtoolsctx_t *pathtools_ctx,
 char *dst, size_t dstsize, const char *path);
static bool st_pathtools_concat(st_pathtoolsctx_t *pathtools_ctx, char *dst,
 size_t dstsize, const char *path, const char *append);

static st_pathtoolsctx_funcs_t pathtoolsctx_funcs = {
    st_modctx_funcs,
    .resolve        = st_pathtools_resolve,
    .get_parent_dir = st_pathtools_get_parent_dir,
    .concat         = st_pathtools_concat,
};

static st_moddata_t st_module_pathtools_cwalk_data = {
    .name = "cwalk",
    .type = ST_MODULE_TYPE,
    .subsystem = "pathtools",
    .prereqs = (st_modprerq_t[]){ 
        { "logger", NULL, },
        {0}, 
    },
    .ctor = st_pathtools_init,
};

ST_MODULE_DEF_INIT_FUNC(pathtools_cwalk)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_pathtools_cwalk_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "pathtools";
static const char *st_module_name = "cwalk";

static st_pathtoolsctx_t *st_pathtools_init(struct st_loggerctx_s *logger_ctx) {
    st_pathtoolsctx_t *pathtools_ctx = (st_pathtoolsctx_t *)st_modctx_new(
     "pathtools", "cwalk", sizeof(st_pathtoolsctx_t), NULL, &pathtoolsctx_funcs, 
     (st_object_dtor_t)st_pathtools_quit);

    if (!pathtools_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "pathtools_cwalk: unable to create new pathtools ctx object");

        return NULL;
    }

    pathtools_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "pathtools_cwalk: Path tools initialized");

    return pathtools_ctx;
}

static void st_pathtools_quit(st_pathtoolsctx_t *pathtools_ctx) {
    ST_LOGGERCTX_CALL(pathtools_ctx->logger_ctx, info,
     "pathtools_cwalk: Path tools destroyed");
    free(pathtools_ctx);
}

static bool st_pathtools_is_iri(const char *path) {
    const char *ppath = path;

    if (!path)
        return false;

    while (isalnum(*++ppath)) {
        if (ppath[0] == ':' && ppath[1] == '/' && (ppath - path) > 1)
            return true;
    }

    return false;
}

static void st_pathtools_to_unix(char *dst, size_t dstsize, const char *path) {
    bool scheme_skipped = !st_pathtools_is_iri(path);

    if (st_pathtools_is_iri(path)) {
        bool scheme_ending_begin = false;
        bool first_slashes_begin = false;

        while (path && *path && dstsize--) {
            if (*path == ':')
                scheme_ending_begin = true;
            *dst++ = *path;
            if (scheme_ending_begin && isalnum(*path++))
                break;
        }
    }

    while (path && *path && dstsize--) {
        *dst++ = *path == '\\' ? '/' : *path;
        path++;
    }

    if (dstsize == 0)
        *dst == '\0';
    else
        *++dst == '\0';
}

static bool st_pathtools_is_absolute(const char *path) {
    return path && !st_pathtools_is_iri(path)
     && (path[0] == '/' || path[0] == '~'
      || (isalpha(path[0]) && path[1] == ':'));
}

static bool st_pathtools_is_relative(const char *path) {
    return path
        && !st_pathtools_is_iri(path)
        && !st_pathtools_is_absolute(path);
}

static st_pathtype_t st_pathtools_get_type(const char *path) {
    if (st_pathtools_is_iri(path))
        return ST_PT_IRI;
    else if (st_pathtools_is_absolute(path))
        return ST_PT_ABSOLUTE;
    else if (st_pathtools_is_relative(path))
        return ST_PT_RELATIVE;
    else
        return ST_PT_NOT_PATH;
}

static bool st_pathtools_resolve(st_pathtoolsctx_t *pathtools_ctx, char *dst,
 size_t dstsize, const char *path) {
    char          unix_path[dstsize];
    st_pathtype_t type = st_pathtools_get_type(path);

    if (type != ST_PT_NOT_PATH)
        st_pathtools_to_unix(unix_path, dstsize, path);

    switch (type) {
        case ST_PT_NOT_PATH:
            return false;
        case ST_PT_IRI: {
            ST_LOGGERCTX_CALL(pathtools_ctx->logger_ctx, error,
             "pathtools_cwalk: IRI normalizing is not implemented yet");
            return false;
        }
        case ST_PT_ABSOLUTE:
        case ST_PT_RELATIVE:
            return cwk_path_normalize(path, dst, dstsize) < dstsize;
        default:
            break;
    }

    return false;
}

static bool st_pathtools_get_parent_dir(st_pathtoolsctx_t *pathtools_ctx, char *dst,
 size_t dstsize, const char *path) {
    return st_pathtools_concat(pathtools_ctx, dst, dstsize, path, "..");
}

static bool st_pathtools_concat(st_pathtoolsctx_t *pathtools_ctx, char *dst,
 size_t dstsize, const char *path, const char *append) {
    char concatenated[dstsize];
    int  ret;

    if (st_pathtools_is_iri(append) || st_pathtools_is_absolute(append))
        ret = snprintf(concatenated, dstsize, "%s", append);
    else
        ret = snprintf(concatenated, dstsize, "%s/%s", path, append);

    return ret > 0 && (size_t)ret < dstsize
        && st_pathtools_resolve(pathtools_ctx, dst, dstsize, concatenated);
}
