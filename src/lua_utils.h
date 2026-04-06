#pragma once

#include <lua.h>
#if defined __has_include
#  if __has_include (<luajit.h>)
#    include <luajit.h>
#  endif
#endif
#include <lauxlib.h>

static inline int st_lua_traceback(lua_State *lua_state) {
    lua_getfield(lua_state, LUA_GLOBALSINDEX, "debug");
    lua_getfield(lua_state, -1, "traceback");
    lua_pushvalue(lua_state, 1);
    lua_pushinteger(lua_state, 2);
    lua_call(lua_state, 2, 1);

    return 1;
}

static inline void *st_lua_create_userdata(lua_State *lua_state, size_t size) {
    return lua_newuserdata(lua_state, size);
}

static inline void st_lua_create_metatable(lua_State *lua_state,
 const char *name) {
    luaL_newmetatable(lua_state, name);
}

static inline void st_lua_create_module(lua_State *lua_state, 
 const char *name) {
    lua_getfield(lua_state, LUA_GLOBALSINDEX, "package");
    lua_getfield(lua_state, -1, "loaded");
    lua_newtable(lua_state);
    lua_setfield(lua_state, -2, name);
    lua_getfield(lua_state, -1, name);
}

static inline void st_lua_set_metatable(lua_State *lua_state, 
 const char *name) {
    #if LUA_VERSION_NUM >= 502 || defined(LUAJIT_VERSION)
        luaL_setmetatable((lua_State *)lua_state, name);
    #else
        /* https://github.com/keplerproject/lua-compat-5.2/blob/master/c-api/compat-5.2.c#L133 */
        luaL_checkstack(lua_state, 1, "Not enough stack slots");
        luaL_getmetatable(lua_state, name);
        lua_setmetatable(lua_state, -2);
    #endif
}

static inline void st_lua_push_bool(lua_State *lua_state, bool val) {
    lua_pushboolean(lua_state, val);
}

static inline void st_lua_push_integer(lua_State *lua_state, ptrdiff_t val) {
    lua_pushinteger(lua_state, val);
}

static inline void st_lua_push_double(lua_State *lua_state, double val) {
    lua_pushnumber(lua_state, val);
}

static inline void st_lua_push_nil(lua_State *lua_state) {
    lua_pushnil(lua_state);
}

static inline void st_lua_push_string(lua_State *lua_state, const char *str) {
    lua_pushstring(lua_state, str);
}

static inline void st_lua_set_nil_to_field(lua_State *lua_state,
 const char *name) {
    lua_pushnil(lua_state);
    lua_setfield(lua_state, -2, name);
}

static inline void st_lua_set_integer_to_field(lua_State *lua_state,
 const char *name, ptrdiff_t integer) {
    lua_pushinteger(lua_state, integer);
    lua_setfield(lua_state, -2, name);
}

static inline void st_lua_set_cfunction_to_field(lua_State *lua_state,
 const char *name, void *cfunction) {
    lua_pushcfunction(lua_state, cfunction);
    lua_setfield(lua_state, -2, name);
}

static inline void st_lua_set_pointer_to_field(lua_State *lua_state,
 const char *name, void *pointer) {
    lua_pushlightuserdata(lua_state, pointer);
    lua_setfield(lua_state, -2, name);
}

static inline void st_lua_set_ffifunction_to_field(lua_State *lua_state,
 const char *name, void *ffifunction) {
    st_lua_set_pointer_to_field(lua_state, name, ffifunction);
}

static inline void st_lua_set_copy_to_field(lua_State *lua_state,
 const char *name, int index) {
    lua_pushvalue(lua_state, index);
    lua_setfield(lua_state, -2, name);
}

static inline bool st_lua_get_bool(lua_State *lua_state, int index) {
    return lua_toboolean(lua_state, index);
}

static inline char st_lua_get_char(lua_State *lua_state, int index) {
    size_t      len;
    const char *str = lua_tolstring(lua_state, index, &len);

    return len == 1
        ? str[0]
        : '\0';
}

static inline ptrdiff_t st_lua_get_integer(lua_State *lua_state, int index) {
    return luaL_checkinteger(lua_state, index);
}

static inline double st_lua_get_double(lua_State *lua_state, int index) {
    return luaL_checknumber(lua_state, index);
}

static inline const char *st_lua_get_lstring_or_null(lua_State *lua_state, 
 int index, size_t *len) {
    return lua_tolstring(lua_state, index, len);
}

static inline const char *st_lua_get_string(lua_State *lua_state, int index) {
    return luaL_checkstring(lua_state, index);
}

static inline const char *st_lua_get_string_or_null(lua_State *lua_state, 
 int index) {
    return lua_tostring(lua_state, index);
}

static inline void *st_lua_get_userdata(lua_State *lua_state, int index) {
    return lua_touserdata(lua_state, index);
}

static inline void *st_lua_get_named_userdata(lua_State *lua_state, int index,
 const char *name) {
    return luaL_checkudata(lua_state, index, name);
}

static inline void *st_lua_get_named_userdata_or_null(lua_State *lua_state, 
 int index, const char *name) {
    #if LUA_VERSION_NUM >= 502 || defined(LUAJIT_VERSION)
        return luaL_testudata((lua_State *)lua_state, index, name);
    #else
        /* https://github.com/keplerproject/lua-compat-5.2/blob/master/c-api/compat-5.2.c#L40 */
        void *userdata = lua_touserdata(lua_state, index);

        luaL_checkstack(lua_state, 2, "Not enough stack slots");
        if (userdata == NULL || !lua_getmetatable(lua_state, index)) {
            return NULL;
        } else {
            int res = 0;

            luaL_getmetatable(lua_state, name);
            res = lua_rawequal(lua_state, -1, -2);
            lua_pop(lua_state, 2);
            if (!res)
                userdata = NULL;
        }
        return userdata;
    #endif
}

static inline void *st_lua_get_global_userdata(lua_State *lua_state,
 const char *name) {
    void *userdata;

    lua_getglobal(lua_state, name);
    userdata = lua_touserdata(lua_state, -1);
    lua_pop(lua_state, 1);

    return userdata;
}

static inline void st_lua_register_cfunction(lua_State *lua_state, 
 const char *name, void *cfunction) {
    lua_register(lua_state, name, cfunction);
}

static inline void st_lua_pop(lua_State *lua_state, size_t elements_count) {
    lua_pop(lua_state, (int)elements_count);
}

static inline void st_lua_raise_error(lua_State *lua_state, const char *msg) {
    luaL_error(lua_state, "%s", msg);
}

static inline void st_lua_save_ptr_in_registry_by_ptr(lua_State *lua_state, 
 void *key, void *data) {
    lua_pushlightuserdata(lua_state, key);
    lua_pushlightuserdata(lua_state, data);
    lua_settable(lua_state, LUA_REGISTRYINDEX);
}

static inline void *st_lua_get_ptr_from_registry_by_ptr(lua_State *lua_state,
 void *key) {
    lua_pushlightuserdata(lua_state, key);
    lua_gettable(lua_state, LUA_REGISTRYINDEX);

    return lua_touserdata(lua_state, -1);
}
