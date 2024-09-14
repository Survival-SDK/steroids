#include "inih.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ini.h>

#define ERRMSGBUF_SIZE      128
#define SAVE_BUFFER_SIZE 131072

typedef struct {
    st_ini_t   *ini;
    const char *filename;
} st_userdata_t;

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static st_inictx_funcs_t inictx_funcs = {
    .quit    = st_ini_quit,
    .create  = st_ini_create,
    .load    = st_ini_load,
    .memload = st_ini_memload,
};

static st_ini_funcs_t ini_funcs = {
    .destroy        = st_ini_destroy,
    .section_exists = st_ini_section_exists,
    .key_exists     = st_ini_key_exists,
    .get_str        = st_ini_get_str,
    .fill_str       = st_ini_fill_str,
    .delete_section = st_ini_delete_section,
    .delete_key     = st_ini_delete_key,
    .clear_section  = st_ini_clear_section,
    .add_section    = st_ini_add_section,
    .add_key        = st_ini_add_key,
    .to_buffer      = st_ini_export,
    .save           = st_ini_save,
};

static void st_ini_free_section(void *ptr) {
    st_inisection_t *section = ptr;

    ST_HTABLE_CALL(section->data, destroy);
    free(section);
}

ST_MODULE_DEF_GET_FUNC(ini_inih)
ST_MODULE_DEF_INIT_FUNC(ini_inih)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_ini_inih_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "ini";
static const char *st_module_name = "inih";

static st_inictx_t *st_ini_init(struct st_loggerctx_s *logger_ctx) {
    st_inictx_t     *ini_ctx;
    st_fnv1a_init_t  fnv1a_init;
    st_htable_init_t htable_init;

    fnv1a_init = global_modsmgr_funcs.get_function(global_modsmgr,
     "fnv1a", NULL, "init");
    if (!fnv1a_init) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "ini_inih: Unable to load function \"init\" from module \"fnv1a\"");

        return NULL;
    }

    htable_init = global_modsmgr_funcs.get_function(global_modsmgr,
     "htable", NULL, "init");
    if (!htable_init) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "ini_inih: Unable to load function \"init\" from module \"htable\"");

        return NULL;
    }

    ini_ctx = st_modctx_new(st_module_subsystem, st_module_name,
     sizeof(st_inictx_t), NULL, &inictx_funcs);
    if (!ini_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "ini_inih: unable to create new ini ctx object");

        return NULL;
    }

    ini_ctx->fnv1a_ctx = fnv1a_init(logger_ctx);
    if (!ini_ctx->fnv1a_ctx)
        goto fnv1a_init_fail;

    ini_ctx->htable_ctx = htable_init(logger_ctx);
    if (!ini_ctx->htable_ctx)
        goto htable_init_fail;

    ini_ctx->logger_ctx = logger_ctx;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "ini_inih: INI-files manipulation module context initialized");

    return ini_ctx;

htable_init_fail:
    ST_FNV1ACTX_CALL(ini_ctx->fnv1a_ctx, quit);
fnv1a_init_fail:
    free(ini_ctx);

    return NULL;
}

static void st_ini_quit(st_inictx_t *ini_ctx) {
    ST_HTABLECTX_CALL(ini_ctx->htable_ctx, quit);
    ST_FNV1ACTX_CALL(ini_ctx->fnv1a_ctx, quit);

    ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, info,
     "ini_inih: INI-files manipulation module context destroyed");
    free(ini_ctx);
}

static bool st_keyeqfunc(const void *left, const void *right) {
    return strcmp(left, right) == 0;
}

static st_ini_t *st_ini_create(st_inictx_t *ini_ctx) {
    st_ini_t      *ini = malloc(sizeof(st_ini_t));

    if (!ini) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Unable to allocate memory for ini: %s", errbuf);

        return NULL;
    }

    st_object_make(ini, ini_ctx, &ini_funcs);

    _Static_assert(sizeof(unsigned int) == sizeof(uint32_t),
     "get_u32hashstr_func returned 32-bit unsigned int but we need 64-bit "
     "insigned instead");
    ini->sections = ST_HTABLECTX_CALL(ini_ctx->htable_ctx, create,
     (unsigned int (*)(const void *))ST_FNV1ACTX_CALL(
      ini_ctx->fnv1a_ctx, get_u32hashstr_func),
     st_keyeqfunc, free, st_ini_free_section);
    if (!ini->sections) {
        ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
         "ini_inih: Unable to create hash table for sections");

        free(ini);
        return NULL;
    }

    return ini;
}

#if INI_HANDLER_LINENO
static int st_ini_parse_handler(void *userdata, const char *section,
 const char *key, const char *value, int lineno) {
#else
static int st_ini_parse_handler(void *userdata, const char *section,
 const char *key, const char *value) {
#endif
    st_ini_t *ini = ((st_userdata_t *)userdata)->ini;

    if (!key && !value)
        return st_ini_add_section(ini, section) ? 1 : 0;

    if (!key || !value)
        return 0;

    return st_ini_add_key(ini, section, key, value) ? 1 : 0;
}

static void st_process_error(int parse_result, const char *filename,
 struct st_loggerctx_s *logger_ctx) {
    switch (parse_result) {
        case 0:
            break;
        case -1:
            ST_LOGGERCTX_CALL(logger_ctx, error,
             "ini_inih: Unable to open file \"%s\"", filename);
            break;
        case -2:
            ST_LOGGERCTX_CALL(logger_ctx, error,
             "ini_inih: Memory allocation error while parsing file \"%s\"",
             filename);
            break;
        default:
            ST_LOGGERCTX_CALL(logger_ctx, error,
             "ini_inih: Syntax error at %s:%d", filename, parse_result);
            break;
    }
}

static st_ini_t *st_ini_load(st_inictx_t *ini_ctx, const char *filename) {
    st_userdata_t userdata;
    int           ret;
    st_ini_t     *ini = st_ini_create(ini_ctx);

    if (!ini)
        return NULL;

    userdata.ini = ini;
    userdata.filename = filename;
    ret = ini_parse(filename, st_ini_parse_handler, &userdata);
    if (ret != 0) {
        st_process_error(ret, filename, ini_ctx->logger_ctx);
        st_ini_destroy(ini);

        return NULL;
    }

    return ini;
}

static st_ini_t *st_ini_memload(st_inictx_t *ini_ctx, const void *ptr,
 size_t size) {
    st_ini_t *ini = st_ini_create(ini_ctx);
    int       ret;
    char     *zero_terminated;

    if (!ini)
        return NULL;

    if (size == 0) {
        ret = ini_parse_string(ptr, st_ini_parse_handler, ini);
        if (ret != 0) {
            st_process_error(ret, NULL, ini_ctx->logger_ctx);

            goto ini_destroy;
        }
    } else {
        zero_terminated = malloc(size + 1);

        if (!zero_terminated) {
            char errbuf[ERRMSGBUF_SIZE];

            if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
                ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
                 "ini_inih: Unable to allocate memory for temporary ini "
                 "buffer: %s", errbuf);

            goto ini_destroy;
        }

        ret = snprintf(zero_terminated, size + 1, "%s", (const char *)ptr);
        if (ret < 0 || (size_t)ret == size + 1) {
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Unable to copy ini data to temp buffer");

            goto free_terminated_and_ini_destroy;
        }

        ret = ini_parse_string(zero_terminated, st_ini_parse_handler, ini);
        if (ret != 0) {
            st_process_error(ret, NULL, ini_ctx->logger_ctx);
            goto free_terminated_and_ini_destroy;
        }

        free(zero_terminated);
    }

    return ini;

free_terminated_and_ini_destroy:
    free(zero_terminated);
ini_destroy:
    st_ini_destroy(ini);
    return NULL;
}

static void st_ini_destroy(st_ini_t *ini) {
    ST_HTABLE_CALL(ini->sections, destroy);
    free(ini);
}

static bool st_ini_section_exists(const st_ini_t *ini, const char *section) {
    return !!ST_HTABLE_CALL(ini->sections, get, !!section ? section : "");
}

static bool st_ini_key_exists(const st_ini_t *ini, const char *section,
 const char *key) {
    return !!st_ini_get_str(ini, section, key);
}

static const char *st_ini_get_str(const st_ini_t *ini, const char *section_name,
 const char *key) {
    st_inisection_t *section = ST_HTABLE_CALL(ini->sections, get,
     !!section_name ? section_name : "");

    if (!section)
        return false;

    return ST_HTABLE_CALL(section->data, get, key);
}

static bool st_ini_fill_str(const st_ini_t *ini, char *dst, size_t dstsize,
 const char *section_name, const char *key) {
    const char *str = st_ini_get_str(ini, section_name, key);
    int         ret;

    if (!str)
        return false;

    ret = snprintf(dst, dstsize, "%s", str);

    return ret > 0 && (size_t)ret < dstsize;
}

static bool st_ini_delete_section(st_ini_t *ini, const char *section) {
    return ST_HTABLE_CALL(ini->sections, remove, section);
}

static bool st_ini_delete_key(st_ini_t *ini, const char *section_name,
 const char *key) {
    st_htiter_t      section_iter;
    st_inisection_t *section;

    if (!ST_HTABLE_CALL(ini->sections, find, &section_iter, section_name))
        return false;

    section = ST_HTITER_CALL(&section_iter, get_value);

    return ST_HTABLE_CALL(section->data, remove, key);
}

static bool st_ini_clear_section(st_ini_t *ini, const char *section_name) {
    st_htiter_t      section_iter;
    st_inisection_t *section;

    if (!ST_HTABLE_CALL(ini->sections, find, &section_iter, section_name))
        return false;

    section = ST_HTITER_CALL(&section_iter, get_value);
    ST_HTABLE_CALL(section->data, clear);

    return true;
}

static bool st_ini_add_section(st_ini_t *ini, const char *section_name) {
    st_inictx_t     *ini_ctx = st_object_get_owner(ini);
    st_inisection_t *section;
    char            *section_key;
    char             errbuf[ERRMSGBUF_SIZE];

    if (ST_HTABLE_CALL(ini->sections, contains, section_name))
        return true;

    _Static_assert(sizeof(unsigned int) == sizeof(uint32_t),
     "get_u32hashstr_func returned 32-bit unsigned int but we need 64-bit "
     "insigned instead");
    st_htable_t *section_ht = ST_HTABLECTX_CALL(ini_ctx->htable_ctx, create,
     (unsigned int (*)(const void *))ST_FNV1ACTX_CALL(
      ini_ctx->fnv1a_ctx, get_u32hashstr_func),
     st_keyeqfunc, free, free);
    if (!section_ht) {
        ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
         "ini_inih: Unable to create section \"%s\"", section_name);

        return false;
    }

    section = malloc(sizeof(st_inisection_t));
    if (!section) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Unable to allocate memory for section \"%s\": %s",
             section_name, errbuf);

        goto malloc_fail;
    }
    section->data = section_ht;
    /* TODO: crete weak_ptr */
    section->ctx = ini_ctx;

    section_key = strdup(section_name);
    if (!section_key) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Unable to allocate memory for section key: %s", errbuf);

        goto strdup_fail;
    }

    if (!ST_HTABLE_CALL(ini->sections, insert, NULL, section_key, section)) {
        ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
         "ini_inih: Unable to insert unnamed section to sections table");

        goto insert_fail;
    }

    return true;

insert_fail:
    free(section_key);
strdup_fail:
    free(section);
malloc_fail:
    ST_HTABLE_CALL(section_ht, destroy);

    return false;
}

static bool st_ini_add_key(st_ini_t *ini, const char *section_name,
 const char *key, const char *value) {
    st_inictx_t     *ini_ctx = st_object_get_owner(ini);
    st_inisection_t *section;
    char            *keydup;
    char            *valdup;
    char             errbuf[ERRMSGBUF_SIZE];

    if (!st_ini_add_section(ini, section_name))
        return false;

    section = ST_HTABLE_CALL(ini->sections, get, section_name);

    keydup = strdup(key);
    if (!keydup) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Unable to allocate memory for key: %s", errbuf);

        return false;
    }

    valdup = strdup(value);
    if (!valdup) {
        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Unable to allocate memory for value: %s", errbuf);

        goto valdup_fail;
    }

    if (!ST_HTABLE_CALL(section->data, insert, NULL, keydup, valdup)) {
        ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
         "ini_inih: Unable to insert key \"%s\" to section \"%s\"", key,
         section_name);

        goto insert_fail;
    }

    return true;

insert_fail:
    free(valdup);
valdup_fail:
    free(keydup);

    return false;
}

static bool st_ini_export(const st_ini_t *ini, char *buffer, size_t bufsize) {
    st_inictx_t *ini_ctx = st_object_get_owner(ini);
    st_htiter_t  section_it;
    size_t       bufoffset;

    if (!ST_HTABLE_CALL(ini->sections, get_first, &section_it))
        return true;

    do {
        const char      *sec_key = ST_HTITER_CALL(&section_it, get_key);
        st_inisection_t *section = ST_HTITER_CALL(&section_it, get_value);
        st_htiter_t      key_it;
        int              sec_ret = 0;

        if (sec_key[0] != '\n') { // if section is not unnamed
            sec_ret = snprintf(buffer, bufsize, "[%s]\n", sec_key);
            if (sec_ret < 0 || (size_t)sec_ret == bufsize) {
                ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
                 "ini_inih: Unable to construct section header for section: %s",
                 sec_key);

                return NULL;
            }
        }

        buffer += sec_ret;
        bufsize -= (size_t)sec_ret;

        if (ST_HTABLE_CALL(section->data, get_first, &key_it))
            continue;

        do {
            const char *key = ST_HTITER_CALL(&key_it, get_key);
            char       *value = ST_HTITER_CALL(&key_it, get_value);
            int         ret = snprintf(buffer, bufsize, "%s=%s\n", key, value);

            if (ret < 0 || (size_t)ret == bufsize) {
                ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
                 "ini_inih: Unable to construct key-value record for key: %s",
                 key);

                return NULL;
            }

            buffer += ret;
            bufsize -= (size_t)ret;
        } while (ST_HTITER_CALL(&key_it, get_next, &key_it));

        if (bufsize-- == 0) {
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Buffer overflow while trying to export section \"%s\"",
             sec_key);

            return NULL;
        }
        *buffer++ = '\n';
    } while (ST_HTITER_CALL(&section_it, get_next, &section_it));

    buffer[bufsize - 1] = '\n';

    return true;
}

static bool st_ini_save(const st_ini_t *ini, const char *filename) {
    st_inictx_t *ini_ctx = st_object_get_owner(ini);
    char         buffer[SAVE_BUFFER_SIZE];
    FILE        *file;
    size_t       buflen;
    size_t       writen;
    bool         buffer_filled = st_ini_export(ini, buffer, SAVE_BUFFER_SIZE);

    if (!buffer_filled)
        return false;

    file = fopen(filename, "wbe");

    if (!file) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            ST_LOGGERCTX_CALL(ini_ctx->logger_ctx, error,
             "ini_inih: Unable to open file \"%s\": %s\n", filename, errbuf);

        return false;
    }

    buflen = strlen(buffer);
    writen = fwrite(buffer, 1, buflen, file);

    fclose(file);

    return writen == buflen;
}
