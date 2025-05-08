#pragma once

#include "steroids/module.h"

st_modsmgr_t *st_modsmgr_init(void);
void st_modsmgr_destroy(st_modsmgr_t *modsmgr);
void *st_modsmgr_get_ctor(const st_modsmgr_t *modsmgr, const char *subsystem, 
 const char *module_name);
void *st_modsmgr_get_ctor_from_ctx(const st_modsmgr_t *modsmgr,
 const st_modctx_t *ctx);
