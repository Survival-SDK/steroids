#ifndef ST_STEROIDS_FNV1_H
#define ST_STEROIDS_FNV1_H

#include <stddef.h>
#include <stdint.h>

#include "steroids/module.h"
#include "steroids/types/modules/fnv1.h"

#if defined(ST_MODULE_TYPE_internal)
    #define ST_VISIBILITY static
#else
    #define ST_VISIBILITY
#endif

ST_VISIBILITY st_modctx_t *st_fnv1_init(st_modctx_t *logger_ctx);
ST_VISIBILITY void st_fnv1_quit(st_modctx_t *so_ctx);
ST_VISIBILITY uint32_t st_fnv1_u32hash_string(st_modctx_t *so_ctx,
 const char *str);
ST_VISIBILITY uint32_t st_fnv1_u32hash_bytes(st_modctx_t *so_ctx,
 const void *ptr, size_t size);
ST_VISIBILITY st_u32hashstrfunc_t st_fnv1_get_u32hashstr_func(
 st_modctx_t *so_ctx);
ST_VISIBILITY st_u32hashbytesfunc_t st_fnv1_get_u32hashbytes_func(
 st_modctx_t *so_ctx);

#undef ST_VISIBILITY

#endif /* ST_STEROIDS_FNV1_H */
