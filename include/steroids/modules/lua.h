#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"

typedef struct st_luastate_s st_luastate_t;

typedef st_modctx_t *(*st_lua_init_t)(st_modctx_t *logger_ctx,
 st_modctx_t *opts_ctx);
typedef void (*st_lua_quit_t)(st_modctx_t *lua_ctx);
typedef bool (*st_lua_run_string_t)(st_modctx_t *lua_ctx, const char *string);
typedef bool (*st_lua_run_file_t)(st_modctx_t *lua_ctx, const char *filename);

typedef st_luastate_t *(*st_lua_get_state_t)(st_modctx_t *lua_ctx);
typedef void *(*st_lua_create_userdata_t)(st_luastate_t *lua_state,
 size_t size);
typedef void (*st_lua_create_metatable_t)(st_luastate_t *lua_state,
 const char *name);
typedef void (*st_lua_create_module_t)(st_luastate_t *lua_state,
 const char *name);
typedef void (*st_lua_set_metatable_t)(st_luastate_t *lua_state,
 const char *name);
typedef void (*st_lua_push_bool_t)(st_luastate_t *lua_state, bool val);
typedef void (*st_lua_push_integer_t)(st_luastate_t *lua_state, ptrdiff_t val);
typedef void (*st_lua_push_double_t)(st_luastate_t *lua_state, double val);
typedef void (*st_lua_push_nil_t)(st_luastate_t *lua_state);
typedef void (*st_lua_push_string_t)(st_luastate_t *lua_state, const char *str);
typedef void (*st_lua_set_nil_to_field_t)(st_luastate_t *lua_state,
 const char *name);
typedef void (*st_lua_set_integer_to_field_t)(st_luastate_t *lua_state,
 const char *name, ptrdiff_t integer);
typedef void (*st_lua_set_cfunction_to_field_t)(st_luastate_t *lua_state,
 const char *name, void *cfunction);
typedef void (*st_lua_set_copy_to_field_t)(st_luastate_t *lua_state,
 const char *name, int index);
typedef bool (*st_lua_get_bool_t)(st_luastate_t *lua_state, int index);
typedef char (*st_lua_get_char_t)(st_luastate_t *lua_state, int index);
typedef ptrdiff_t (*st_lua_get_integer_t)(st_luastate_t *lua_state, int index);
typedef double (*st_lua_get_double_t)(st_luastate_t *lua_state, int index);
typedef const char *(*st_lua_get_lstring_or_null_t)(st_luastate_t *lua_state,
 int index, size_t *len);
typedef const char *(*st_lua_get_string_t)(st_luastate_t *lua_state, int index);
typedef const char *(*st_lua_get_string_or_null_t)(st_luastate_t *lua_state,
 int index);
typedef void *(*st_lua_get_userdata_t)(st_luastate_t *lua_state, int index);
typedef void *(*st_lua_get_named_userdata_t)(st_luastate_t *lua_state,
 int index, const char *name);
typedef void *(*st_lua_get_named_userdata_or_null_t)(st_luastate_t *lua_state,
 int index, const char *name);
typedef void *(*st_lua_get_global_userdata_t)(st_luastate_t *lua_state,
 const char *name);
typedef void (*st_lua_register_cfunction_t)(st_luastate_t *lua_state,
 const char *name, void *cfunction);
typedef void (*st_lua_pop_t)(st_luastate_t *lua_state, size_t elements_count);
typedef void (*st_lua_raise_error_t)(st_luastate_t *lua_state, const char *msg);

typedef struct {
    st_lua_init_t                       lua_init;
    st_lua_quit_t                       lua_quit;
    st_lua_run_string_t                 lua_run_string;
    st_lua_run_file_t                   lua_run_file;

    st_lua_get_state_t                  lua_get_state;
    st_lua_create_userdata_t            lua_create_userdata;
    st_lua_create_metatable_t           lua_create_metatable;
    st_lua_create_module_t              lua_create_module;
    st_lua_set_metatable_t              lua_set_metatable;
    st_lua_push_bool_t                  lua_push_bool;
    st_lua_push_integer_t               lua_push_integer;
    st_lua_push_double_t                lua_push_double;
    st_lua_push_nil_t                   lua_push_nil;
    st_lua_push_string_t                lua_push_string;
    st_lua_set_nil_to_field_t           lua_set_nil_to_field;
    st_lua_set_integer_to_field_t       lua_set_integer_to_field;
    st_lua_set_cfunction_to_field_t     lua_set_cfunction_to_field;
    st_lua_set_copy_to_field_t          lua_set_copy_to_field;
    st_lua_get_bool_t                   lua_get_bool;
    st_lua_get_char_t                   lua_get_char;
    st_lua_get_integer_t                lua_get_integer;
    st_lua_get_double_t                 lua_get_double;
    st_lua_get_lstring_or_null_t        lua_get_lstring_or_null;
    st_lua_get_string_t                 lua_get_string;
    st_lua_get_string_or_null_t         lua_get_string_or_null;
    st_lua_get_userdata_t               lua_get_userdata;
    st_lua_get_named_userdata_t         lua_get_named_userdata;
    st_lua_get_named_userdata_or_null_t lua_get_named_userdata_or_null;
    st_lua_get_global_userdata_t        lua_get_global_userdata;
    st_lua_register_cfunction_t         lua_register_cfunction;
    st_lua_pop_t                        lua_pop;
    st_lua_raise_error_t                lua_raise_error;
} st_lua_funcs_t;
