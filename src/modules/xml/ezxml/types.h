#pragma once

#include <ezxml.h>

#include "steroids/modctx.h"
#include "steroids/modules/logger.h"
#include "steroids/object.h"

typedef struct {
    st_modctx_t;
    st_loggerctx_t *logger_ctx;
} st_xmlctx_t;

typedef struct {
    st_object_t;
    // struct ezxml *handle;
    char *buffer; /* needed by struct ezxml */
} st_xml_t;

typedef st_object_t st_xmlchilditer_t;
typedef st_object_t st_xmlnamedchilditer_t;

#define ST_XMLCTX_T_DEFINED
#define ST_XML_T_DEFINED
#define ST_XMLCHILDITER_T_DEFINED
#define ST_XMLNAMEDCHILDITER_T_DEFINED
