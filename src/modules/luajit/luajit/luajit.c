#include "luajit.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "embedded_luajit.h"
#include "lua_utils.h"
#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/luajitbind.h"

#define ERRMSGBUF_SIZE   128
#define BINDING_NAME_SIZE 32
#define BINDINGS_COUNT   256

typedef struct {
    const char *code;
    const char *name;
} st_luajit_named_string_t;

static st_luajitctx_t *st_luajit_init(const st_param_t params[]);
static void st_luajit_quit(st_luajitctx_t *luajit_ctx);

static void st_luajit_state_dtor_for_htable(st_luajitstate_t *luajit_state);
static void st_luajit_state_destroy(st_luajitstate_t *luajit_state);

static bool st_luajit_run(st_runnablectx_t *luajit_ctx,
 const st_param_t params[]);
static st_luajitstate_t *st_luajit_newstate(st_luajitctx_t *luajit_ctx,
 const char *name);
static st_luajitstate_t *st_luajit_getstate(st_luajitctx_t *luajit_ctx,
 const char *name);

static st_luajitstate_t *st_luajit_newthread(st_luajitstate_t *luajit_state,
 const char *name);
static bool st_luajit_run_named_string(st_luajitstate_t *state,
 const char *chunkname, const char *string);
static bool st_luajit_run_string(st_luajitstate_t *state, const char *string);
static bool st_luajit_run_file(st_luajitstate_t *state, const char *filename);

static st_luajitctx_funcs_t luajitctx_funcs = {
    ST_MODCTX_FUNCS,
    .run       = st_luajit_run,
    .new_state = st_luajit_newstate,
    .get_state = st_luajit_getstate,
};

static st_luajitstate_funcs_t luajitstate_funcs = {
    ST_OBJECT_FUNCS,
    .new_thread       = st_luajit_newthread,
    .run_named_string = st_luajit_run_named_string,
    .run_string       = st_luajit_run_string,
    .run_file         = st_luajit_run_file,
};

static const st_modprerq_t mod_prereqs[] = {
    { "fnv1a",  NULL, },
    { "htable", NULL, },
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_luajit_luajit_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("luajit", "luajit", ST_MODULE_TYPE, mod_prereqs,
     st_luajit_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_luajit_luajit_init(modsmgr);
}
#endif

static bool st_keyeqfunc(const void *left, const void *right) {
    return strcmp(left, right) == 0;
}

static void st_luajit_init_bindings(st_luajitctx_t *luajit_ctx) {
    char             bindings_names[BINDING_NAME_SIZE][BINDINGS_COUNT] = {0};
    char            *pbindingsnames[BINDINGS_COUNT];

    for (size_t i = 0; i < BINDINGS_COUNT; i++)
        pbindingsnames[i] = bindings_names[i];

    luajit_ctx->bindings = st_dlist_create(sizeof(st_luajitbindctx_t *), 
     st_object_free_by_ptr);
    if (!luajit_ctx->bindings) {
        ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, error,
         "luajit_luajit: Unable to create list of luabind contexts");

        return;
    }

    ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, info,
     "luajit_luajit: Searching luabind modules");

    ST_MODSMGR_CALL(luajit_ctx->modsmgr, get_module_names, pbindingsnames,
     BINDINGS_COUNT, BINDING_NAME_SIZE, "luajitbind");

    for (size_t i = 0; i < BINDINGS_COUNT; i++) {
        st_ctx_ctor_t       ctx_ctor;
        st_luajitbindctx_t *ctx;
        char               *binding_name = pbindingsnames[i];

        if (!*binding_name)
            break;

        ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, info,
         "luajit_luajit: Found module \"luajitbind_%s\"", binding_name);

        ctx_ctor = ST_MODSMGR_CALL(luajit_ctx->modsmgr, get_ctor,
         "luajitbind", binding_name);

        if (!ctx_ctor) {
            ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, error,
             "luajit_luajit: Unable to get ctor from module \"luajitbind_%s\"",
             binding_name);

            continue;
        }

        ctx = ctx_ctor((st_params_t){
            {"modsmgr", (uintptr_t)luajit_ctx->modsmgr},
            {"luajit_ctx", (uintptr_t)luajit_ctx}
        });
        if (!ctx)
            continue;

        if (!st_dlist_push_back(luajit_ctx->bindings, &ctx)) {
            ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, error,
             "luajit_luajit: Unable to create entry node for module "
             "\"luajitbind_%s\"", binding_name);
            ST_LUAJITBINDCTX_CALL(ctx, destroy);

            continue;
        }
    }
}

static st_luajitctx_t *st_luajit_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    st_fnv1actx_t  *fnv1a_ctx = (st_fnv1actx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "fnv1a", NULL);
    st_htablectx_t *htable_ctx = (st_htablectx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "htable", NULL);
    st_luajitctx_t *luajit_ctx = (st_luajitctx_t *)st_modctx_new("luajit",
     "luajit", sizeof(st_luajitctx_t), NULL, &luajitctx_funcs,
     (st_object_dtor_t)st_luajit_quit);

    if (!luajit_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "luajit_luajit: unable to create new luajit ctx object");

        return NULL;
    }

    luajit_ctx->modsmgr    = modsmgr;
    luajit_ctx->logger_ctx = logger_ctx;
    luajit_ctx->fnv1a_ctx  = fnv1a_ctx;
    luajit_ctx->htable_ctx = htable_ctx;

    luajit_ctx->states = ST_HTABLECTX_CALL(htable_ctx, create,
     (unsigned int (*)(const void *))ST_FNV1ACTX_CALL(
      fnv1a_ctx, get_u32hashstr_func),
     st_keyeqfunc, NULL, (st_freefunc_t)st_luajit_state_dtor_for_htable);
    if (!luajit_ctx->states) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "luajit_luajit: Unable to create hash table for lua states");

        free(luajit_ctx);
        return NULL;
    }

    st_luajit_init_bindings(luajit_ctx);

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "luajit_luajit: LuaJIT VM initialized.");

    return luajit_ctx;
}

static void st_luajit_quit(st_luajitctx_t *luajit_ctx) {
    st_dlist_destroy(luajit_ctx->bindings);

    ST_HTABLE_CALL(luajit_ctx->states, destroy);

    ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, info,
     "luajit_luajit: LuaJIT VM destroyed.");
    free(luajit_ctx);
}

static bool st_luajit_state_import_basic_ffi_cdefs(st_luajitstate_t *state) {
    st_luajitctx_t *luajit_ctx = (st_luajitctx_t *)ST_OBJECT_CALL(state,
     get_owner);

    if  (!ST_LUAJITSTATE_CALL(state, run_named_string, 
     "src/modules/luajit/luajit/embedded.luajit", EMBEDDED_LUAJIT))
        return false;

    /* Now call require("ModsMgr") and set __instance */
    lua_getfield(state->handle, LUA_GLOBALSINDEX, "require");
    lua_pushstring(state->handle, "ModsMgr");
    lua_call(state->handle, 1, 1);  /* Module is on stack */
    st_lua_set_pointer_to_field(state->handle, "__instance",
     luajit_ctx->modsmgr);
    lua_pop(state->handle, 1);

    return true;
}

static st_luajitstate_t *st_luajit_newstate(st_luajitctx_t *luajit_ctx,
 const char *name) {
    lua_State        *handle;
    st_luajitstate_t *new_state;
    char             *namedup;

    if (ST_HTABLE_CALL(luajit_ctx->states, contains, name))
        return NULL;

    handle = luaL_newstate();
    if (!handle)
        return NULL;

    new_state = (st_luajitstate_t *)st_object_new(sizeof(st_luajitstate_t),
     &luajitstate_funcs, (st_object_dtor_t)st_luajit_state_destroy,
     (st_object_t *)luajit_ctx);
    if (!new_state)
        goto obj_new_fail;

    namedup = strdup(name);
    if (!namedup)
        goto strdup_fail;

    if (!ST_HTABLE_CALL(luajit_ctx->states, insert, NULL, namedup, new_state))
        goto insert_fail;

    luaL_openlibs(handle);

    new_state->handle = handle;
    new_state->name = namedup;

    st_lua_save_ptr_in_registry_by_ptr(handle, handle, new_state);

    if (!st_luajit_state_import_basic_ffi_cdefs(new_state)) {
        ST_LUAJITSTATE_CALL(new_state, destroy);

        return NULL;
    }

    if (luajit_ctx->bindings) {
        st_dlnode_t *node = st_dlist_get_head(luajit_ctx->bindings);

        while (node) {
            st_luajitbindctx_t *bind_ctx = st_dlist_export_ptr(node);

            if (!ST_LUAJITBINDCTX_CALL(bind_ctx, bind, name)) {
                ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, error,
                 "luajit_luajit: Unable to bind module to state \"%s\"", name);
            }

            node = st_dlist_get_next(node);
        }
    }

    return new_state;

insert_fail:
    free(namedup);

strdup_fail:
    free(new_state);

obj_new_fail:
    lua_close(handle);

    return NULL;
}

static st_luajitstate_t *st_luajit_getstate(st_luajitctx_t *luajit_ctx,
 const char *name) {
    st_htiter_t iter;

    return ST_HTABLE_CALL(luajit_ctx->states, find, &iter, name)
        ? ST_HTITER_CALL(&iter, get_value)
        : NULL;
}

static st_luajitstate_t *st_luajit_newthread(st_luajitstate_t *luajit_state,
 const char *name) {
    return NULL; // Not implemented yet
}

static void st_luajit_state_dtor_for_htable(st_luajitstate_t *luajit_state) {
    lua_close(luajit_state->handle);
    free(luajit_state->name);
    free(luajit_state);
}

static void st_luajit_state_destroy(st_luajitstate_t *luajit_state) {
    st_luajitctx_t *luajit_ctx = (st_luajitctx_t *)ST_OBJECT_CALL(luajit_state,
     get_owner);

    ST_HTABLE_CALL(luajit_ctx->states, remove, luajit_state->name);
}

int lua_load_named_string_wrapper(lua_State *L, const void *arg) {
    const st_luajit_named_string_t *named_string = arg;

    return luaL_loadbuffer(L, named_string->code, strlen(named_string->code), 
     named_string->name);
}

int lua_load_string_wrapper(lua_State *L, const void *arg) {
    return luaL_loadstring(L, arg);
}

int lua_load_file_wrapper(lua_State *L, const void *arg) {
    return luaL_loadfile(L, arg);
}

static bool st_luajit_run_impl(st_luajitstate_t *state,
 int (*func)(lua_State *, const void *), const void *arg) {
    int             pcall_result;
    st_luajitctx_t *luajit_ctx = (st_luajitctx_t *)ST_OBJECT_CALL(state,
     get_owner);

    lua_pushcfunction(state->handle, st_lua_traceback);

    if (func(state->handle, arg) != LUA_OK) {
        ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, error,
         "lua_luajit: Unable to load Lua script: %s",
         lua_tostring(state->handle, -1));

        return false;
    }

    pcall_result = lua_pcall(state->handle, 0, 0,
     lua_gettop(state->handle) - 1);

    if (pcall_result == LUA_OK) {
        lua_pop(state->handle, lua_gettop(state->handle));
    } else {
        const char *error = lua_tostring(state->handle,
         lua_gettop(state->handle));

        ST_LOGGERCTX_CALL(luajit_ctx->logger_ctx, error, "lua_luajit: %s",
         error);
    }

    return pcall_result == LUA_OK;
}

static bool st_luajit_run_named_string(st_luajitstate_t *state,
 const char *chunkname, const char *string) {
    st_luajit_named_string_t named_string = {
        .code = string,
        .name = chunkname
    };
    return st_luajit_run_impl(state, lua_load_named_string_wrapper,
     &named_string);
}

static bool st_luajit_run_string(st_luajitstate_t *state, const char *string) {
    return st_luajit_run_impl(state, lua_load_string_wrapper, string);
}

static bool st_luajit_run_file(st_luajitstate_t *state, const char *filename) {
    return st_luajit_run_impl(state, lua_load_file_wrapper, filename);
}

static bool st_luajit_run(st_runnablectx_t *luajit_ctx,
 const st_param_t params[]) {
    const char       *script = st_modctx_get_param_as_ptr(params, "script");
    st_luajitstate_t *main_state = st_luajit_getstate(
     (st_luajitctx_t *)luajit_ctx, "main");

    if (!main_state)
        main_state = st_luajit_newstate((st_luajitctx_t *)luajit_ctx, "main");

    return (script && main_state)
        ? st_luajit_run_file(main_state, script)
        : false;
}
