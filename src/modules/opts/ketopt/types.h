#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <klib/ketopt.h>
#pragma GCC diagnostic pop

#include "steroids/types/modctx.h"
#include "steroids/types/modules/logger.h"

#define ST_OPTS_OPTS_MAX 128

typedef struct {
    char *longopt;
    int   longopt_index;
    char  shortopt;
    int   arg;
    char *arg_fmt;
    char *opt_descr;
} st_opt_t;

typedef struct {
    st_modctx_t;
    struct st_loggerctx_s *logger_ctx;
    int                    argc;
    char                 **argv;
    st_opt_t               opts[ST_OPTS_OPTS_MAX];
    unsigned               opts_count;
} st_optsctx_t;

#define ST_OPTSCTX_T_DEFINED
