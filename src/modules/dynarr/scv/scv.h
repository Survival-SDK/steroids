#pragma once

#include "config.h" // IWYU pragma: keep
#include "types.h" // IWYU pragma: keep
#include "steroids/dynarr.h"

static st_dynarrctx_funcs_t st_dynarr_scv_funcs = {
    .quit   = st_dynarr_quit,
    .create = st_dynarr_create,
};

static st_modfuncentry_t st_module_dynarr_scv_funcs[] = {
    {"init",               st_dynarr_init},
    {"quit",               st_dynarr_quit},
    {"create",             st_dynarr_create},
    {"destroy",            st_dynarr_destroy},
    {"append",             st_dynarr_append},
    {"set",                st_dynarr_set},
    {"clear",              st_dynarr_clear},
    {"sort",               st_dynarr_sort},
    {"extract",            st_dynarr_extract},
    {"get",                st_dynarr_get},
    {"get_all",            st_dynarr_get_all},
    {"get_elements_count", st_dynarr_get_elements_count},
    {"is_empty",           st_dynarr_is_empty},
    {NULL, NULL},
};
