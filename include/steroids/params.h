#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ST_PARAMS_MAX 32

typedef struct {
    const char *key;
    uintptr_t   value;
} st_param_t;

typedef st_param_t st_params_t[ST_PARAMS_MAX];

static inline bool st_params_add(st_param_t params[], const char *key,
 uintptr_t value) {
    size_t count = 0;

    if (!params || !key || !*key)
        return false;

    while (memcmp(&params[count], &(st_param_t){0}, sizeof(st_param_t)) != 0)
        count++;

    if (count < ST_PARAMS_MAX - 1) {
        params[count] = (st_param_t){key, value};
        params[count + 1] = (st_param_t){0};

        return true;
    }

    return false;
}
