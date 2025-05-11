#include "window.h"

#include <stdio.h>

#include "steroids/modules/window.h"

#define CTX_METATABLE_NAME    "window_ctx"
#define WINDOW_METATABLE_NAME "window"

static st_modsmgr_t                   *global_modsmgr;
static st_modsmgr_funcs_t              global_modsmgr_funcs;

static st_window_init_t                st_window_init;
static st_window_quit_t                st_window_quit;
static st_window_create_t              st_window_create;
static st_window_process_t             st_window_process;

static st_lua_get_state_t              st_lua_get_state;
static st_lua_create_userdata_t        st_lua_create_userdata;
static st_lua_create_metatable_t       st_lua_create_metatable;
static st_lua_create_module_t          st_lua_create_module;
static st_lua_set_metatable_t          st_lua_set_metatable;
static st_lua_set_cfunction_to_field_t st_lua_set_cfunction_to_field;
static st_lua_set_copy_to_field_t      st_lua_set_copy_to_field;
static st_lua_get_named_userdata_t     st_lua_get_named_userdata;
static st_lua_get_bool_t               st_lua_get_bool;
static st_lua_get_integer_t            st_lua_get_integer;
static st_lua_get_string_t             st_lua_get_string;
static st_lua_push_bool_t              st_lua_push_bool;
static st_lua_push_integer_t           st_lua_push_integer;
static st_lua_pop_t                    st_lua_pop;

static void st_luabind_bind_all(st_modctx_t *luabind_ctx);

ST_MODULE_DEF_GET_FUNC(luabind_window)
ST_MODULE_DEF_INIT_FUNC(luabind_window)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_luabind_window_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_luabind_import_functions(st_modctx_t *luabind_ctx,
 st_modctx_t *lua_ctx, st_modctx_t *logger_ctx) {
    st_luabind_window_t *module = luabind_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "luabind_window: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("luabind_window", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("luabind_window", logger, info);

    ST_LOAD_GLOBAL_FUNCTION("luabind_window", window, init);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", window, quit);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", window, create);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", window, process);

    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, get_state);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, create_userdata);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, create_metatable);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, create_module);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, set_metatable);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, set_cfunction_to_field);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, set_copy_to_field);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, get_named_userdata);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, get_bool);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, get_integer);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, get_string);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, push_bool);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, push_integer);
    ST_LOAD_GLOBAL_FUNCTION("luabind_window", lua, pop);

    return true;
}

static st_modctx_t *st_luabind_init(st_modctx_t *logger_ctx,
 st_modctx_t *lua_ctx) {
    st_modctx_t         *luabind_ctx;
    st_luabind_window_t *luabind;

    luabind_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_luabind_window_data, sizeof(st_luabind_window_t));

    if (!luabind_ctx)
        return NULL;

    luabind_ctx->funcs = &st_luabind_window_funcs;

    luabind = luabind_ctx->data;
    luabind->lua.ctx = lua_ctx;
    luabind->logger.ctx = logger_ctx;

    if (!st_luabind_import_functions(luabind_ctx, lua_ctx, logger_ctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, luabind_ctx);

        return NULL;
    }

    st_luabind_bind_all(luabind_ctx);

    luabind->logger.info(luabind->logger.ctx,
     "luabind_window: Window funcs binding initialized");

    return luabind_ctx;
}

static void st_luabind_quit(st_modctx_t *luabind_ctx) {
    st_luabind_window_t *module = luabind_ctx->data;

    module->logger.info(module->logger.ctx,
     "luabind_window: Window funcs binding destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, luabind_ctx);
}

static int st_window_init_bind(st_luastate_t *lua_state) {
    void        *userdata = st_lua_create_userdata(lua_state,
     sizeof(st_modctx_t *));
    st_modctx_t *events_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 1, "events_ctx");
    st_modctx_t *logger_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 2, "logger_ctx");
    st_modctx_t *monitor_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 3, "monitor_ctx");

    *(st_modctx_t **)userdata = st_window_init(events_ctx, logger_ctx,
     monitor_ctx);
    st_lua_set_metatable(lua_state, CTX_METATABLE_NAME);

    return 1;
}

static int st_window_quit_bind(st_luastate_t *lua_state) {
    st_modctx_t *window_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 1, CTX_METATABLE_NAME);

    st_window_quit(window_ctx);

    return 0;
}

static int st_window_create_bind(st_luastate_t *lua_state) {
    st_modctx_t  *window_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 1, CTX_METATABLE_NAME);
    st_monitor_t *monitor = *(st_monitor_t **)st_lua_get_named_userdata(
     lua_state, 2, "monitor");
    ptrdiff_t     x = st_lua_get_integer(lua_state, 3);
    ptrdiff_t     y = st_lua_get_integer(lua_state, 4);
    ptrdiff_t     width = st_lua_get_integer(lua_state, 5); // NOLINT(readability-magic-numbers)
    ptrdiff_t     height = st_lua_get_integer(lua_state, 6); // NOLINT(readability-magic-numbers)
    bool          fullscreen = st_lua_get_bool(lua_state, 7); // NOLINT(readability-magic-numbers)
    const char   *title = st_lua_get_string(lua_state, 8); // NOLINT(readability-magic-numbers)
    void         *userdata = st_lua_create_userdata(lua_state,
     sizeof(st_window_t *));

    *(st_window_t **)userdata = st_window_create(window_ctx, monitor, (int)x,
     (int)y, (unsigned)width, (unsigned)height, fullscreen, title);
    st_lua_set_metatable(lua_state, WINDOW_METATABLE_NAME);

    return 1;
}

static int st_window_process_bind(st_luastate_t *lua_state) {
    st_modctx_t *window_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 1, CTX_METATABLE_NAME);

    st_window_process(window_ctx);

    return 0;
}

static int st_window_destroy_bind(st_luastate_t *lua_state) {
    st_window_t *window = *(st_window_t **)st_lua_get_named_userdata(lua_state,
     1, WINDOW_METATABLE_NAME);

    ST_WINDOW_CALL(window, destroy);

    return 0;
}

static int st_window_xed_bind(st_luastate_t *lua_state) {
    st_window_t *window = *(st_window_t **)st_lua_get_named_userdata(lua_state,
     1, WINDOW_METATABLE_NAME);

    st_lua_push_bool(lua_state, ST_WINDOW_CALL(window, xed));

    return 1;
}

static int st_window_get_width_bind(st_luastate_t *lua_state) {
    st_window_t *window = *(st_window_t **)st_lua_get_named_userdata(lua_state,
     1, WINDOW_METATABLE_NAME);
    unsigned width = ST_WINDOW_CALL(window, get_width);

    st_lua_push_integer(lua_state, (ptrdiff_t)width);

    return 1;
}

static int st_window_get_height_bind(st_luastate_t *lua_state) {
    st_window_t *window = *(st_window_t **)st_lua_get_named_userdata(lua_state,
     1, WINDOW_METATABLE_NAME);
    unsigned height = ST_WINDOW_CALL(window, get_height);

    st_lua_push_integer(lua_state, (ptrdiff_t)height);

    return 1;
}

static void st_luabind_bind_all(st_modctx_t *luabind_ctx) {
    st_luabind_window_t *module = luabind_ctx->data;
    st_luastate_t       *lua_state = st_lua_get_state(module->lua.ctx);

    st_lua_create_module(lua_state, "Window");
    st_lua_set_cfunction_to_field(lua_state, "new_ctx", st_window_init_bind);

    st_lua_pop(lua_state, 3);

    st_lua_create_metatable(lua_state, CTX_METATABLE_NAME);

    st_lua_set_cfunction_to_field(lua_state, "__gc", st_window_quit_bind);
    st_lua_set_cfunction_to_field(lua_state, "destroy", st_window_quit_bind);
    st_lua_set_copy_to_field(lua_state, "__index", -1);
    st_lua_set_cfunction_to_field(lua_state, "create", st_window_create_bind);
    st_lua_set_cfunction_to_field(lua_state, "process", st_window_process_bind);

    st_lua_pop(lua_state, 1);

    st_lua_create_metatable(lua_state, WINDOW_METATABLE_NAME);
    st_lua_set_cfunction_to_field(lua_state, "__gc", st_window_destroy_bind);
    st_lua_set_cfunction_to_field(lua_state, "destroy", st_window_destroy_bind);
    st_lua_set_cfunction_to_field(lua_state, "xed", st_window_xed_bind);
    st_lua_set_cfunction_to_field(lua_state, "get_width",
     st_window_get_width_bind);
    st_lua_set_cfunction_to_field(lua_state, "get_height",
     st_window_get_height_bind);
    st_lua_set_copy_to_field(lua_state, "__index", -1);

    st_lua_pop(lua_state, 1);
}
