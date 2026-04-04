#pragma once

#include "dlist.h"
#include "steroids/object.h"

typedef struct {
    st_object_t;
    st_dlist_t *modsdata;
    st_dlist_t *singletons;
} st_modsmgr_t;

#define ST_MODSMGR_T_DEFINED
