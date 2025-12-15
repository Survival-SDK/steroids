#pragma once

#include <lua.h>

#include "steroids/modules/fnv1a.h"
#include "steroids/modules/htable.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"
#include "steroids/modsmgr.h"
#include "steroids/runnablectx.h"

#include "dlist.h"

typedef struct {
    st_runnablectx_t;
    st_modsmgr_t   *modsmgr;
    st_fnv1actx_t  *fnv1a_ctx;
    st_htablectx_t *htable_ctx;
    st_loggerctx_t *logger_ctx;
    st_htable_t    *states;
    st_dlist_t     *bindings;
} st_luajitctx_t;

typedef struct {
    st_object_t;
    lua_State *handle;
    char      *name;
} st_luajitstate_t;

#define ST_LUAJITCTX_T_DEFINED
#define ST_LUAJITSTATE_T_DEFINED
