#pragma once

#include "steroids/modules/ini.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/opts.h"
#include "steroids/modules/pathtools.h"
#include "steroids/modules/plugin.h"
#include "steroids/runnablectx.h"

typedef struct {
    st_runnablectx_t;
    st_modsmgr_t          *modsmgr;
    st_inictx_t           *ini_ctx;
    struct st_loggerctx_s *logger_ctx;
    st_optsctx_t          *opts_ctx;
    st_pathtoolsctx_t     *pathtools_ctx;
    st_pluginctx_t        *plugin_ctx;
    const char            *default_configfile;
    const char            *default_directory;
} st_runnerctx_t;

#define ST_RUNNERCTX_T_DEFINED
