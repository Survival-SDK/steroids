#pragma once

#include "steroids/modules/angle.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t      *ctx;
    st_logger_debug_t debug;
    st_logger_info_t  info;
    st_logger_error_t error;
} st_matrix3x3_simple_logger_t;

typedef struct {
    st_modctx_t             *ctx;
    st_angle_dtor_t          dtor;
    st_angle_rnormalize360_t rnormalize360;
    st_angle_rdsin_t         rdsin;
    st_angle_rdcos_t         rdcos;
    st_angle_rdtan_t         rdtan;
} st_matrix3x3_simple_angle_t;

typedef struct {
    st_matrix3x3_simple_logger_t logger;
    st_matrix3x3_simple_angle_t  angle;
} st_matrix3x3_simple_t;

ST_CLASS (
    float r1c1, r1c2, r1c3;
    float r2c1, r2c2, r2c3;
    //       0,    0,    1
) st_matrix3x3_t;

#define ST_MATRIX3X3_T_DEFINED
