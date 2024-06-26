#include "luajit.h"

#include <errno.h>
#include <stdio.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#define ERRMSGBUF_SIZE   128
#define BINDING_NAME_SIZE 32
#define BINDINGS_COUNT   256

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static void st_lua_bind_all(st_modctx_t *lua_ctx);

ST_MODULE_DEF_GET_FUNC(lua_luajit)
ST_MODULE_DEF_INIT_FUNC(lua_luajit)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_lua_luajit_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_lua_import_functions(st_modctx_t *lua_ctx,
 st_modctx_t *logger_ctx) {
    st_lua_luajit_t *module = lua_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "lua_luajit: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("lua_luajit", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("lua_luajit", logger, info);

    return true;
}

static void st_lua_init_bindings(st_modctx_t *logger_ctx,
 st_modctx_t *lua_ctx) {
    st_lua_luajit_t *module = lua_ctx->data;
    char             bindings_names[BINDING_NAME_SIZE][BINDINGS_COUNT] = {0};
    char            *pbindingsnames[BINDINGS_COUNT];
    char            *binding_name;

    for (size_t i = 0; i < BINDINGS_COUNT; i++)
        pbindingsnames[i] = bindings_names[i];

    module->bindings = st_slist_create(sizeof(st_lua_luajit_binding_t));
    if (!module->bindings) {
        module->logger.info(module->logger.ctx,
         "lua_luajit: Unable to create list of luabind modules");

        return;
    }

    module->logger.info(module->logger.ctx,
     "lua_luajit: Searching luabind modules");

    global_modsmgr_funcs.get_module_names(global_modsmgr, pbindingsnames,
     BINDINGS_COUNT, BINDING_NAME_SIZE, "luabind");

    for (size_t i = 0; i < BINDINGS_COUNT; i++) {
        st_luabind_init_t        init_func;
        st_luabind_quit_t        quit_func;
        st_modctx_t             *ctx;
        st_lua_luajit_binding_t *binding;

        binding_name = pbindingsnames[i];

        if (!*binding_name)
            break;

        module->logger.info(module->logger.ctx,
         "lua_luajit: Found module \"luabind_%s\"", binding_name);

        init_func = global_modsmgr_funcs.get_function(global_modsmgr,
         "luabind", binding_name, "init");
        if (!init_func) {
            module->logger.error(module->logger.ctx,
             "lua_luajit: Unable to get function \"init\" from module "
             "\"luabind_%s\"", binding_name);

            continue;
        }

        quit_func = global_modsmgr_funcs.get_function(global_modsmgr,
         "luabind", binding_name, "quit");
        if (!quit_func) {
            module->logger.error(module->logger.ctx,
             "lua_luajit: Unable to get function \"quit\" from module "
             "\"luabind_%s\"", binding_name);

            continue;
        }

        ctx = init_func(logger_ctx, lua_ctx);

        binding = malloc(sizeof(st_lua_luajit_binding_t));
        if (!binding) {
            char errbuf[ERRMSGBUF_SIZE];

            if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
                module->logger.error(module->logger.ctx,
                 "lua_luajit: Unable to allocate memory for binding entry of "
                 "module \"luabind_%s\": %s", binding_name, errbuf);

            quit_func(ctx);

            continue;
        }
        binding->ctx = ctx;
        binding->quit = quit_func;

        if (!st_slist_insert_head(module->bindings, binding)) {
            module->logger.error(module->logger.ctx,
             "lua_luajit: Unable to create entry node for module "
             "\"luabind_%s\"", binding_name);
            free(binding);
            quit_func(ctx);

            continue;
        }
    }
}

static st_modctx_t *st_lua_init(st_modctx_t *logger_ctx,
 st_modctx_t *opts_ctx) {
    st_modctx_t     *lua_ctx;
    st_lua_luajit_t *module;

    lua_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_lua_luajit_data, sizeof(st_lua_luajit_t));

    if (!lua_ctx)
        return NULL;

    lua_ctx->funcs = &st_lua_luajit_funcs;

    module = lua_ctx->data;
    module->logger.ctx = logger_ctx;
    module->opts.ctx = opts_ctx;

    if (!st_lua_import_functions(lua_ctx, logger_ctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, lua_ctx);

        return NULL;
    }

    module->state = luaL_newstate();
    luaL_openlibs(module->state);

    st_lua_bind_all(lua_ctx);
    st_lua_init_bindings(logger_ctx, lua_ctx);

    module->logger.info(module->logger.ctx, "lua_luajit: Lua initialized.");

    return lua_ctx;
}

static void st_lua_quit(st_modctx_t *lua_ctx) {
    st_lua_luajit_t *module = lua_ctx->data;

    lua_close(module->state);

    while (!st_slist_empty(module->bindings)) {
        st_slnode_t             *node = st_slist_get_first(module->bindings);
        st_lua_luajit_binding_t *binding = st_slist_get_data(node);

        st_slist_remove_head(module->bindings);
        binding->quit(binding->ctx);
        free(binding);
    }

    st_slist_destroy(module->bindings);

    module->logger.info(module->logger.ctx, "lua_luajit: Lua destroyed.");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, lua_ctx);
}

static void st_lua_bind_all(st_modctx_t *lua_ctx) {
    st_lua_luajit_t *module = lua_ctx->data;

    lua_pushlightuserdata(module->state, lua_ctx);
    lua_setglobal(module->state, "__st_lua_ctx");
    lua_pushlightuserdata(module->state, module->logger.ctx);
    lua_setglobal(module->state, "__st_logger_ctx");
    lua_pushlightuserdata(module->state, module->opts.ctx);
    lua_setglobal(module->state, "__st_opts_ctx");
}

static int traceback(lua_State *lua_state) {
    lua_getfield(lua_state, LUA_GLOBALSINDEX, "debug");
    lua_getfield(lua_state, -1, "traceback");
    lua_pushvalue(lua_state, 1);
    lua_pushinteger(lua_state, 2);
    lua_call(lua_state, 2, 1);

    return 1;
}

static bool st_lua_run(st_modctx_t *lua_ctx,
 int (*func)(lua_State *, const char *), const char *arg) {
    st_lua_luajit_t *lua = lua_ctx->data;
    bool             loaded;
    bool             success_call;

    lua_pushcfunction(lua->state, traceback);

    loaded = func(lua->state, arg) == LUA_OK;
    if (!loaded) {
        lua->logger.error(lua->logger.ctx,
         "lua_luajit: Unable to load Lua script: %s",
         lua_tostring(lua->state, -1));

        return false;
    }

    success_call = lua_pcall(lua->state, 0, 0, lua_gettop(lua->state) - 1) ==
     LUA_OK;

    if (success_call) {
        lua_pop(lua->state, lua_gettop(lua->state));
    } else {
        const char *error = lua_tostring(lua->state, lua_gettop(lua->state));

        lua->logger.error(lua->logger.ctx, "lua_luajit: %s", error);
    }

    return success_call;
}

static bool st_lua_run_string(st_modctx_t *lua_ctx, const char *string) {
    return st_lua_run(lua_ctx, luaL_loadstring, string);
}

static bool st_lua_run_file(st_modctx_t *lua_ctx, const char *filename) {
    return st_lua_run(lua_ctx, luaL_loadfile, filename);
}

static st_luastate_t *st_lua_get_state(st_modctx_t *lua_ctx) {
    st_lua_luajit_t *lua = lua_ctx->data;

    return (st_luastate_t *)lua->state;
}

static void *st_lua_create_userdata(st_luastate_t *lua_state, size_t size) {
    return lua_newuserdata((lua_State *)lua_state, size);
}

static void st_lua_create_metatable(st_luastate_t *lua_state,
 const char *name) {
    luaL_newmetatable((lua_State *)lua_state, name);
}

static void st_lua_create_module(st_luastate_t *lua_state, const char *name) {
    lua_getfield((lua_State *)lua_state, LUA_GLOBALSINDEX, "package");
    lua_getfield((lua_State *)lua_state, -1, "loaded");
    lua_newtable((lua_State *)lua_state);
    lua_setfield((lua_State *)lua_state, -2, name);
    lua_getfield((lua_State *)lua_state, -1, name);
}

static void st_lua_set_metatable(st_luastate_t *lua_state, const char *name) {
    luaL_setmetatable((lua_State *)lua_state, name);
}

static void st_lua_push_bool(st_luastate_t *lua_state, bool val) {
    lua_pushboolean((lua_State *)lua_state, val);
}

static void st_lua_push_integer(st_luastate_t *lua_state, ptrdiff_t val) {
    lua_pushinteger((lua_State *)lua_state, val);
}

static void st_lua_push_double(st_luastate_t *lua_state, double val) {
    lua_pushnumber((lua_State *)lua_state, val);
}

static void st_lua_push_nil(st_luastate_t *lua_state) {
    lua_pushnil((lua_State *)lua_state);
}

static void st_lua_push_string(st_luastate_t *lua_state, const char *str) {
    lua_pushstring((lua_State *)lua_state, str);
}

static void st_lua_set_nil_to_field(st_luastate_t *lua_state,
 const char *name) {
    lua_pushnil((lua_State *)lua_state);
    lua_setfield((lua_State *)lua_state, -2, name);
}

static void st_lua_set_integer_to_field(st_luastate_t *lua_state,
 const char *name, ptrdiff_t integer) {
    lua_pushinteger((lua_State *)lua_state, integer);
    lua_setfield((lua_State *)lua_state, -2, name);
}

static void st_lua_set_cfunction_to_field(st_luastate_t *lua_state,
 const char *name, void *cfunction) {
    lua_pushcfunction((lua_State *)lua_state, cfunction);
    lua_setfield((lua_State *)lua_state, -2, name);
}

static void st_lua_set_copy_to_field(st_luastate_t *lua_state,
 const char *name, int index) {
    lua_pushvalue((lua_State *)lua_state, index);
    lua_setfield((lua_State *)lua_state, -2, name);
}

static bool st_lua_get_bool(st_luastate_t *lua_state, int index) {
    return lua_toboolean((lua_State *)lua_state, index);
}

static char st_lua_get_char(st_luastate_t *lua_state, int index) {
    size_t      len;
    const char *str = lua_tolstring((lua_State *)lua_state, index, &len);

    if (len != 1)
        return '\0';

    return str[0];
}

static ptrdiff_t st_lua_get_integer(st_luastate_t *lua_state, int index) {
    return luaL_checkinteger((lua_State *)lua_state, index);
}

static double st_lua_get_double(st_luastate_t *lua_state, int index) {
    return luaL_checknumber((lua_State *)lua_state, index);
}

static const char *st_lua_get_lstring_or_null(st_luastate_t *lua_state,
 int index, size_t *len) {
    return lua_tolstring((lua_State *)lua_state, index, len);
}

static const char *st_lua_get_string(st_luastate_t *lua_state, int index) {
    return luaL_checkstring((lua_State *)lua_state, index);
}

static const char *st_lua_get_string_or_null(st_luastate_t *lua_state,
 int index) {
    return lua_tostring((lua_State *)lua_state, index);
}

static void *st_lua_get_userdata(st_luastate_t *lua_state, int index) {
    return lua_touserdata((lua_State *)lua_state, index);
}

static void *st_lua_get_named_userdata(st_luastate_t *lua_state, int index,
 const char *name) {
    return luaL_checkudata((lua_State *)lua_state, index, name);
}

static void *st_lua_get_named_userdata_or_null(st_luastate_t *lua_state,
 int index, const char *name) {
    return luaL_testudata((lua_State *)lua_state, index, name);
}

static void *st_lua_get_global_userdata(st_luastate_t *lua_state,
 const char *name) {
    void *userdata;

    lua_getglobal((lua_State *)lua_state, name);
    userdata = lua_touserdata((lua_State *)lua_state, -1);
    lua_pop((lua_State *)lua_state, 1);

    return userdata;
}

static void st_lua_register_cfunction(st_luastate_t *lua_state,
 const char *name, void *cfunction) {
    lua_register((lua_State *)lua_state, name, cfunction);
}

static void st_lua_pop(st_luastate_t *lua_state, size_t elements_count) {
    lua_pop((lua_State *)lua_state, (int)elements_count);
}

static void st_lua_raise_error(st_luastate_t *lua_state, const char *msg) {
    luaL_error((lua_State *)lua_state, "%s", msg);
}
