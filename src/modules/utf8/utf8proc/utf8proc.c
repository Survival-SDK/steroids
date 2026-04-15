#include "utf8proc.h"

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include <utf8proc.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

static st_utf8ctx_t *st_utf8_init(const st_param_t params[]);
static void st_utf8_quit(st_utf8ctx_t *utf8_ctx);

static ssize_t st_utf8_str_codepoints(const st_utf8ctx_t *utf8_ctx,
 const char *str, size_t codepoints_max);
static const char *st_utf8_str_advance(const st_utf8ctx_t *utf8_ctx,
 const char *str, size_t codepoints_count);
static int64_t st_utf8_to_codepoint(const st_utf8ctx_t *utf8_ctx,
 const char *utf8char);
static ssize_t st_utf8_str_to_codepoints(const st_utf8ctx_t *utf8_ctx,
 const char *str, uint32_t *dst, size_t codepoints_max);

static st_utf8ctx_funcs_t utf8ctx_funcs = {
    ST_MODCTX_FUNCS,
    .str_codepoints    = st_utf8_str_codepoints,
    .str_advance       = st_utf8_str_advance,
    .to_codepoint      = st_utf8_to_codepoint,
    .str_to_codepoints = st_utf8_str_to_codepoints,
};

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_utf8_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("utf8", "utf8proc", ST_MODULE_TYPE, mod_prereqs,
     st_utf8_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_utf8_init(modsmgr);
}
#endif

static st_utf8ctx_t *st_utf8_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_utf8ctx_t   *utf8_ctx = (st_utf8ctx_t *)st_modctx_new("utf8", "utf8proc",
     sizeof(st_utf8ctx_t), NULL, &utf8ctx_funcs,
     (st_object_dtor_t)st_utf8_quit);

    if (!utf8_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "utf8_utf8proc: unable to create new utf8 utilities ctx object");

        return NULL;
    }

    utf8_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "utf8_utf8proc: UTF-8 utilities module context initialized");

    return utf8_ctx;
}

static void st_utf8_quit(st_utf8ctx_t *utf8_ctx) {
    ST_LOGGERCTX_CALL(utf8_ctx->logger_ctx, info,
     "utf8_utf8proc: UTF-8 utilities module context destroyed");
    free(utf8_ctx);
}

// static void st_vec2_add(__attribute__((unused)) const st_vec2ctx_t *vec2_ctx,
//  float *vec_x, float *vec_y, float add_x, float add_y) {
//     *vec_x += add_x;
//     *vec_y += add_y;
// }

static ssize_t st_utf8_str_codepoints(
 __attribute__((unused)) const st_utf8ctx_t *utf8_ctx, const char *str, 
 size_t codepoints_max) {
    ssize_t          codepoints = 0;
    utf8proc_int32_t codepoint_ref;
    utf8proc_ssize_t codepoint_size;
    
    if (!str)
        return -1;

    if (!*str)
        return 0;
    
    if (codepoints_max == 0)
        codepoints_max = SIZE_MAX;

    while (codepoints < codepoints_max) {
        codepoint_size = utf8proc_iterate(str, -1, &codepoint_ref);

        if (codepoint_size == 0)
            return codepoints;

        if (codepoint_size < 0 || codepoint_ref == -1)
            return codepoints > 0 
                ? codepoints 
                : -1;
                
        codepoints++;
        str += codepoint_size;
    }

    return codepoints;
}

static const char *st_utf8_str_advance(
 __attribute__((unused)) const st_utf8ctx_t *utf8_ctx, const char *str, 
 size_t codepoints_advance_value) {
    size_t           codepoints = 0;
    utf8proc_int32_t codepoint_ref = 0;
    utf8proc_ssize_t codepoint_size = 1; /* nonzero */
    
    if (!str || !*str)
        return NULL;

    if (codepoints_advance_value == 0)
        return str;
    
    while (codepoints < codepoints_advance_value
     && codepoint_size > 0 && codepoint_ref != -1) {
        codepoint_size = utf8proc_iterate(str, -1, &codepoint_ref);
        if (codepoint_size > 0 && codepoint_ref != -1) {
            codepoints++;
            str += codepoint_size;
        }
    }

    return ((codepoints == codepoints_advance_value) && *str)
        ? str 
        : NULL;
}

static int64_t st_utf8_to_codepoint(
 __attribute__((unused)) const st_utf8ctx_t *utf8_ctx, const char *utf8char) {
    utf8proc_int32_t codepoint_ref = 0;
    utf8proc_ssize_t codepoint_size;
    
    if (!utf8char || !*utf8char)
        return -1;

    codepoint_size = utf8proc_iterate(utf8char, -1, 
     &codepoint_ref);

    return codepoint_size > 0
        ? codepoint_ref
        : -1;
}

static ssize_t st_utf8_str_to_codepoints(
 __attribute__((unused)) const st_utf8ctx_t *utf8_ctx, const char *str, 
 uint32_t *dst, size_t codepoints_max) {
    utf8proc_int32_t codepoint_ref = 0;
    utf8proc_ssize_t codepoint_size;
    size_t           codepoint_index = 0;
    
    if (!str || !dst)
        return -1;

    if (!*str || codepoints_max == 0)
        return 0;
    
    do {
        codepoint_size = utf8proc_iterate(str, -1, 
         &codepoint_ref);
        
        if (codepoint_size > 0 && codepoint_ref != -1) {
            dst[codepoint_index++] = codepoint_ref;
            str += codepoint_size;
        } else if (codepoint_index == 0) {
            return -1;
        }
    } while (codepoint_size > 0 && codepoint_ref != -1 
     && codepoint_index < codepoints_max && *str);

    return codepoint_index;
}
