#include "ketopt.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"

#define ERRMSGBUF_SIZE               128
#define LONG_OPT_NUM_TO_INDEX_OFFSET 300
#define SHORT_OPTS_FMT_SIZE          100
#define LONGOPTS_COUNT                64
#define OPTS_COLUMNS_MAX            1024
#define HELP_COLUMNS_MIN               8

static st_optsctx_t *st_opts_init(const st_param_t params[]);
static void st_opts_quit(st_optsctx_t *opts_ctx);

static bool st_opts_add_option(st_optsctx_t *opts_ctx, char short_option,
 const char *long_option, st_opt_arg_t arg, const char *arg_fmt,
 const char *option_descr);
static bool st_opts_get_str(st_optsctx_t *opts_ctx, const char *opt, char *dst,
 size_t dstsize);
static bool st_opts_get_help(st_optsctx_t *opts_ctx, char *dst, size_t dstsize,
 size_t columns);

static st_optsctx_funcs_t optsctx_funcs = {
    st_modctx_funcs,
    .add_option = st_opts_add_option,
    .get_str    = st_opts_get_str,
    .get_help   = st_opts_get_help,
};

typedef enum {
    ST_OT_SHORT,
    ST_OT_LONG,
} st_opttype_t;

static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};

st_moddata_t *st_module_opts_ketopt_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("opts", "ketopt", ST_MODULE_TYPE, mod_prereqs,
     st_opts_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_opts_ketopt_init(modsmgr, modsmgr_funcs);
}
#endif

static const char *st_module_subsystem = "opts";
static const char *st_module_name = "ketopt";

static st_optsctx_t *st_opts_init(const st_param_t params[]) {
    st_modsmgr_t   *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
     get_singleton, "logger", NULL);
    int             argc = st_modctx_get_param_as_int(params, "argc");
    char          **argv = st_modctx_get_param_as_ptr(params, "argv");
    st_optsctx_t   *opts_ctx = (st_optsctx_t *)st_modctx_new("opts", "ketopt",
     sizeof(st_optsctx_t), NULL, &optsctx_funcs,
     (st_object_dtor_t)st_opts_quit);

    if (!opts_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "opts_ketopt: Unable to create new htable ctx object");

        return NULL;
    }

    opts_ctx->logger_ctx = logger_ctx;

    opts_ctx->argc = argc;
    opts_ctx->argv = argv;
    memset(opts_ctx->opts, '\0', sizeof(st_opt_t) * ST_OPTS_OPTS_MAX);
    opts_ctx->opts_count = 0;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "opts_ketopt: Command line options processor initialized");

    return opts_ctx;
}

static void st_opts_quit(st_optsctx_t *opts_ctx) {
    for (unsigned i = 0; i < opts_ctx->opts_count; i++) {
        free(opts_ctx->opts[i].longopt);
        free(opts_ctx->opts[i].arg_fmt);
        free(opts_ctx->opts[i].opt_descr);
    }

    ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, info,
     "opts_ketopt: Command line options processor destroyed");
    free(opts_ctx);
}

static bool st_opts_add_option(st_optsctx_t *opts_ctx, char short_option,
 const char *long_option, st_opt_arg_t arg, const char *arg_fmt,
 const char *option_descr) {
    st_opt_t *opt = &opts_ctx->opts[opts_ctx->opts_count];
    bool      opt_set = false;

    if (!long_option || long_option[0] == '\0' || long_option[1] == '\0') {
        ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, error,
         "opts_ketopt: Incorrect long option: %s", long_option);
    } else {
        opt->longopt = strdup(long_option);
        if (!opt->longopt) {
            char errbuf[ERRMSGBUF_SIZE];

            if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
                ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, error,
                 "opts_ketopt: Unable to allocate memory for long option "
                 "\"%s\": %s", long_option, errbuf);
        } else {
            opt->longopt_index = (int)opts_ctx->opts_count +
             LONG_OPT_NUM_TO_INDEX_OFFSET;
            opt_set = true;
        }
    }

    if (short_option) {
        if (isalnum(short_option)) {
            opt->shortopt = short_option;
            opt_set = true;
        } else {
            ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, error,
             "opts_ketopt: Incorrect character for short option \"%c\"",
             short_option);
        }
    }

    if (!opt_set)
        return false;

    opt->arg = arg;

    if (arg_fmt) {
        opt->arg_fmt = strdup(arg_fmt);
        if (!opt->arg_fmt)
            ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, warning,
             "opts_ketopt: Unable to allocate memory for option argument "
             "format");
    }
    if (option_descr) {
        opt->opt_descr = strdup(option_descr);
        if (!opt->opt_descr)
            ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, warning,
             "opts_ketopt: Unable to allocate memory for option description");
    }

    opts_ctx->opts_count++;

    return true;
}

static bool st_opts_get_str(st_optsctx_t *opts_ctx, const char *opt, char *dst,
 size_t dstsize) {
    char         short_opts_fmt[SHORT_OPTS_FMT_SIZE] = "";
    char        *pshortopts_fmt = short_opts_fmt;
    ko_longopt_t longopts[LONGOPTS_COUNT] = {0};
    size_t       longopts_count = 0;
    int          longopt_index = -1;
    char         shortopt = '\0';
    ketopt_t     kopt = KETOPT_INIT;

    if (!dst || !dstsize)
        return false;

    if (!opt || opt[0] == '\0') {
        ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, error,
         "opts_ketopt: Empty option");

        return false;
    }

    for (unsigned i = 0; i < opts_ctx->opts_count; i++) {
        if (opts_ctx->opts[i].shortopt) {
            ptrdiff_t opt_fmt_size = opts_ctx->opts[i].arg == ST_OA_NO ? 1 : 2;

            if (pshortopts_fmt - short_opts_fmt
             >= SHORT_OPTS_FMT_SIZE - opt_fmt_size) {
                ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, error,
                 "opts_ketopt: Short opts format overflow");
            } else {
                *pshortopts_fmt++ = opts_ctx->opts[i].shortopt;

                if (opts_ctx->opts[i].arg != ST_OA_NO) {
                    *pshortopts_fmt++ = opts_ctx->opts[i].arg == ST_OA_REQUIRED
                        ?':'
                        : '?';
                }
            }
        }

        if (opts_ctx->opts[i].longopt) {
            if (strcmp(opts_ctx->opts[i].longopt, opt) == 0 ||
             (opt[1] == '\0' && opts_ctx->opts[i].shortopt == opt[0])) {
                longopt_index = opts_ctx->opts[i].longopt_index;
                shortopt = opts_ctx->opts[i].shortopt;
            }

            longopts[longopts_count].name = opts_ctx->opts[i].longopt;
            longopts[longopts_count].has_arg = (int)opts_ctx->opts[i].arg;
            longopts[longopts_count].val = opts_ctx->opts[i].longopt_index;
            longopts_count++;
        }
    }

    while (true) {
        int parse_result = ketopt(&kopt, opts_ctx->argc, opts_ctx->argv, true,
         short_opts_fmt, longopts);

        if (parse_result == '?' || parse_result == ':')
            continue;

        if (parse_result < 0)
            break;

        if (parse_result == longopt_index ||
         (opt[1] == '\0' && parse_result == opt[0]) ||
         parse_result == shortopt) {
            int ret;

            if (!kopt.arg) {
                dst[0] = '\0';

                return true;
            }

            ret = snprintf(dst, dstsize, "%s", kopt.arg);
            if (ret < 0 || (size_t)ret == dstsize) {
                ST_LOGGERCTX_CALL(opts_ctx->logger_ctx, error,
                 "opts_ketopt: Unable to get option argument");

                return false;
            }

            return true;
        }
    }

    return false;
}

static bool st_opts_get_help(st_optsctx_t *opts_ctx, char *dst, size_t dstsize,
 size_t columns) {
    size_t block_size;
    size_t opts_columns;
    size_t descr_columns;
    int    ret;

    if (columns <= HELP_COLUMNS_MIN)
        columns = HELP_COLUMNS_MIN;

    opts_columns = columns / 2 - 1;
    descr_columns = columns / 2 - 1;

    ret = snprintf(dst, dstsize, "Usage:\n%s [OPTS]\n\n", opts_ctx->argv[0]);
    if (ret < 0 || (size_t)ret == dstsize)
        return false;
    dst += ret;
    dstsize -= (size_t)ret;

    for (unsigned i = 0; i < opts_ctx->opts_count; i++) {
        char        opts[OPTS_COLUMNS_MAX] = {0};
        char       *popts = opts + 2;
        size_t      columns = OPTS_COLUMNS_MAX - 2;
        size_t      opts_len;
        const char *descr = opts_ctx->opts[i].opt_descr;
        size_t      descr_len = strlen(descr);
        int         ret;

        if (opts_ctx->opts[i].shortopt != ST_OPTS_SHORT_UNSPEC) {
            opts[0] = '-';
            opts[1] = opts_ctx->opts[i].shortopt;
        } else {
            opts[0] = ' ';
            opts[1] = ' ';
        }

        if (opts_ctx->opts[i].longopt) {
            opts[2] = opts_ctx->opts[i].shortopt == ST_OPTS_SHORT_UNSPEC
             ? ' '
             : ',';
            opts[3] = ' ';
            opts[4] = '-';
            opts[5] = '-'; // NOLINT(readability-magic-numbers)

            popts += 4;
            columns -= 4;

            ret = snprintf(popts, columns, "%s", opts_ctx->opts[i].longopt);
            if (ret < 0 || (size_t)ret == columns)
                return false;
            popts += ret;
            columns -= (size_t)ret;
        }

        if (opts_ctx->opts[i].arg != ST_OA_NO) {
            if (opts_ctx->opts[i].arg == ST_OA_OPTIONAL) {
                if (columns-- == 0)
                    return false;
                *popts++ = '[';
            }

            if (columns-- == 0)
                return false;
            *popts++ = opts_ctx->opts[i].longopt ? '=' : ' ';

            ret = snprintf(popts, columns, "%s", opts_ctx->opts[i].arg_fmt);
            if (ret < 0 || (size_t)ret == columns)
                return false;
            popts += ret;
            columns -= (size_t)ret;

            if (opts_ctx->opts[i].arg == ST_OA_OPTIONAL) {
                if (columns-- == 0)
                    return false;
                *popts++ = ']';
            }
        }

        ret = snprintf(dst, dstsize, "%s", opts);
        if (ret < 0 || (size_t)ret == dstsize)
            return false;
        dst += ret;
        dstsize -= (size_t)ret;

        opts_len = strlen(opts);

        if (opts_len > opts_columns) {
            if (dstsize-- == 0)
                return false;
            *dst++ = '\n';
        }

        ret = snprintf(dst, dstsize, "%*s",
         (opts_len > opts_columns) ? opts_columns : (opts_columns - opts_len),
         "");
        if (ret < 0 || (size_t)ret == dstsize)
            return false;
        dst += ret;
        dstsize -= (size_t)ret;

        if (dstsize-- == 0)
            return false;
        *dst++ = ' ';

        while (descr_len > 0) {
            size_t      to_copy_len;
            const char *space = NULL;

            if (descr_len > descr_columns) {
                space = descr + descr_columns;

                while (*space != ' ' && space != descr)
                    space--;

                to_copy_len = (space == descr)
                    ? descr_columns
                    : (size_t)(space - descr);
            } else {
                to_copy_len = descr_len;
            }

            if (dstsize < to_copy_len)
                return false;
            ret = snprintf(dst, to_copy_len + 1, "%s", descr);
            if (ret < 0 || (size_t)ret == dstsize)
                return false;
            dst += to_copy_len;
            dstsize -= to_copy_len;

            if (to_copy_len < descr_len) {
                if (dstsize-- == 0)
                    return false;
                *dst++ = '\n';

                ret = snprintf(dst, dstsize, "%*s", opts_columns, "");
                if (ret < 0 || (size_t)ret == dstsize)
                    return false;
                dst += ret;
                dstsize -= (size_t)ret;
            }

            if (descr == space)
                to_copy_len++;

            descr_len -= to_copy_len;
            descr += to_copy_len;
        }

        if (dstsize-- == 0)
            return false;
        *dst++ = '\n';
    }

    return true;
}
