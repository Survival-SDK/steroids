#pragma once

#include "config.h" // IWYU pragma: keep
#include "steroids/angle.h"
#include "steroids/modules/logger.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_angle_simple_logger_t;

typedef struct {
    st_angle_simple_logger_t logger;
} st_angle_simple_t;

static st_angle_funcs_t st_angle_simple_funcs = {
    .angle_init           = st_angle_init,
    .angle_quit           = st_angle_quit,
    .angle_rtod           = st_angle_rtod,
    .angle_dtor           = st_angle_dtor,
    .angle_rnormalize360  = st_angle_rnormalize360,
    .angle_rnormalized360 = st_angle_rnormalized360,
    .angle_dnormalize360  = st_angle_dnormalize360,
    .angle_dnormalized360 = st_angle_dnormalized360,
    .angle_rdsin          = st_angle_rdsin,
    .angle_dgsin          = st_angle_dgsin,
    .angle_rdcos          = st_angle_rdcos,
    .angle_dgcos          = st_angle_dgcos,
    .angle_rdtan          = st_angle_rdtan,
    .angle_dgtan          = st_angle_dgtan,
    .angle_rdacos         = st_angle_rdacos,
    .angle_dgacos         = st_angle_dgacos,
};

static st_modfuncentry_t st_module_angle_simple_funcs[] = {
    {"init",           st_angle_init},
    {"quit",           st_angle_quit},
    {"rtod",           st_angle_rtod},
    {"dtor",           st_angle_dtor},
    {"rnormalize360",  st_angle_rnormalize360},
    {"rnormalized360", st_angle_rnormalized360},
    {"dnormalize360",  st_angle_dnormalize360},
    {"dnormalized360", st_angle_dnormalized360},
    {"rdsin",          st_angle_rdsin},
    {"dgsin",          st_angle_dgsin},
    {"rdcos",          st_angle_rdcos},
    {"dgcos",          st_angle_dgcos},
    {"rdtan",          st_angle_rdtan},
    {"dgtan",          st_angle_dgtan},
    {"rdacos",         st_angle_rdacos},
    {"dgacos",         st_angle_dgacos},
    {NULL, NULL},
};
