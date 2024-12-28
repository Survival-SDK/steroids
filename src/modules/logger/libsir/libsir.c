#include "libsir.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <sir.h>
#pragma GCC diagnostic pop
#include <sir/errors.h>

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

static void st_logger_quit(st_loggerctx_t *logger_ctx);
static bool st_logger_enable_events(st_loggerctx_t *logger_ctx,
 struct st_eventsctx_s *events_ctx);
static bool st_logger_set_stdout_levels(st_loggerctx_t *logger_ctx,
 st_loglvl_t levels);
static bool st_logger_set_stderr_levels(st_loggerctx_t *logger_ctx,
 st_loglvl_t levels);
static bool st_logger_set_log_file(st_loggerctx_t *logger_ctx,
 const char *filename, st_loglvl_t levels);
static bool st_logger_set_callback(st_loggerctx_t *logger_ctx, st_logcbk_t func,
 void *userdata, st_loglvl_t levels);
static void st_logger_debug(const st_loggerctx_t *logger_ctx,
 const char* format, ...);
static void st_logger_info(const st_loggerctx_t *logger_ctx, const char* format,
 ...);
static void st_logger_warning(const st_loggerctx_t *logger_ctx,
 const char* format, ...);
static void st_logger_error(const st_loggerctx_t *logger_ctx,
 const char* format, ...);
static void st_logger_set_postmortem_msg(st_loggerctx_t *logger_ctx,
 const char *msg);

static st_loggerctx_funcs_t loggerctx_funcs = {
    st_modctx_funcs,
    .enable_events      = st_logger_enable_events,
    .set_stdout_levels  = st_logger_set_stdout_levels,
    .set_stderr_levels  = st_logger_set_stderr_levels,
    .set_log_file       = st_logger_set_log_file,
    .set_callback       = st_logger_set_callback,
    .debug              = st_logger_debug,
    .info               = st_logger_info,
    .warning            = st_logger_warning,
    .error              = st_logger_error,
    .set_postmortem_msg = st_logger_set_postmortem_msg,
};

static st_moddata_t st_module_logger_libsir_data = {
    .name = "libsir",
    .type = ST_MODULE_TYPE,
    .subsystem = "logger",
    .prereqs = (st_modprerq_t[]){ {0}, },
    .ctor = st_logger_init,
};

ST_MODULE_DEF_INIT_FUNC(logger_libsir)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_logger_libsir_init(modsmgr, modsmgr_funcs);
}
#endif

static void log_file_destroy(void *plog_file) {
    st_logger_libsir_log_file_t *log_file = plog_file;

    sir_remfile(log_file->file);
}

static st_loggerctx_t *st_logger_init(struct st_eventsctx_s *events_ctx) {
    st_loggerctx_t *logger_ctx;
    sirinit         init_options = {
        .d_stdout = {
            .levels = ST_LL_NONE,
            .opts = (sir_options)SIRO_NOHOST | (sir_options)SIRO_NONAME |
             (sir_options)SIRO_NOPID | (sir_options)SIRO_NOTID,
        },
        .d_stderr = {
            .levels = ST_LL_ALL,
            .opts = (sir_options)SIRO_NOHOST | (sir_options)SIRO_NONAME |
             (sir_options)SIRO_NOPID | (sir_options)SIRO_NOTID,
        },
        .d_syslog = {
            .levels = ST_LL_NONE,
            .opts = (sir_options)SIRO_NOHOST,
            .identity = "",
            .category = "",
        },
        .name = "steroids", /* TODO(edomin): move name management to module */
    };

    if (sir_isinitialized())
        return NULL;

    logger_ctx = (st_loggerctx_t *)st_modctx_new("logger", "libsir", 
     sizeof(st_loggerctx_t), NULL, (st_object_dtor_t)st_logger_quit, 
     &loggerctx_funcs);
    if (!logger_ctx) {
        fprintf(stderr,
         "logger_libsir: unable to create new logger ctx object\n");

        return NULL;
    }

    if (!sir_init(&init_options)) {
        fprintf(stderr, "logger_libsir: unable to initialize libsir\n");
        free(logger_ctx);

        return NULL;
    }

    logger_ctx->events_ctx = events_ctx 
        ? st_weakptr_create((st_object_t *)events_ctx) 
        : 0;
    logger_ctx->callbacks = st_dlist_create(sizeof(st_logger_libsir_callback_t),
     NULL);
    if (!logger_ctx->callbacks)
        st_logger_warning(logger_ctx, "logger_libsir: Unable to initialize "
         "dlist for callbacks. Logging callbacks is not available on this run");

    logger_ctx->log_files = st_dlist_create(sizeof(st_logger_libsir_log_file_t),
     log_file_destroy);
    if (!logger_ctx->log_files)
        st_logger_warning(logger_ctx, "logger_libsir: Unable to initialize "
         "dlist for log files. Logging to file is not available on this run");

    if (events_ctx && !st_logger_enable_events(logger_ctx, events_ctx))
        logger_ctx->events_ctx = 0;

    st_logger_info(logger_ctx, "logger_libsir: Logger initialized");

    return logger_ctx;
}

static void st_logger_quit(st_loggerctx_t *logger_ctx) {
    if (!sir_isinitialized())
        return;

    st_logger_info(logger_ctx, "logger_libsir: Destroying logger");

    if (logger_ctx->callbacks)
        st_dlist_destroy(logger_ctx->callbacks);

    if (logger_ctx->log_files)
        st_dlist_destroy(logger_ctx->log_files);

    sir_cleanup();

    printf("%s", logger_ctx->postmortem_msg);

    free(logger_ctx);
    fprintf(stderr, "logger_libsir: Logger destroyed\n");
}

static bool st_logger_enable_events(st_loggerctx_t *logger_ctx,
 struct st_eventsctx_s *events_ctx) {
    /* Not implemented */

    return false;
}

static bool st_logger_set_stdout_levels(st_loggerctx_t *logger_ctx,
 st_loglvl_t levels) {
    return sir_stdoutlevels((sir_levels)levels);
}

static bool st_logger_set_stderr_levels(st_loggerctx_t *logger_ctx,
 st_loglvl_t levels) {
    return sir_stderrlevels((sir_levels)levels);
}

static bool st_logger_set_log_file(st_loggerctx_t *logger_ctx,
 const char *filename, st_loglvl_t levels) {
    st_dlnode_t *node;

    if (!logger_ctx || !filename)
        return false;

    if (!logger_ctx->log_files)
        return false;

    node = st_dlist_get_head(logger_ctx->log_files);

    while (node) {
        st_logger_libsir_log_file_t *log_file = st_dlist_get_data(node);

        if (strncmp(log_file->filename, filename, PATH_MAX) == 0) {
            if (levels == ST_LL_NONE) {
                st_dlist_remove(node);
            } else if (!sir_filelevels(log_file->file, (sir_levels)levels)) {
                char errmsg[SIR_MAXERROR];

                if (sir_geterror(errmsg) == SIR_E_NOERROR) {
                    st_logger_error(logger_ctx,
                     "logger_libsir: Unable to update \"%s\" log file levels: "
                     "%s", filename, errmsg);
                } else {
                    st_logger_error(logger_ctx,
                     "logger_libsir: Unable to update \"%s\" log file levels",
                     filename);
                }

                return false;
            }

            return true;
        }

        node = st_dlist_get_next(node);
    }

    if (levels != ST_LL_NONE) {
        int                         ret;
        st_logger_libsir_log_file_t log_file;

        ret = snprintf(log_file.filename, PATH_MAX, "%s", filename);
        if (ret < 0 || ret == PATH_MAX) {
            st_logger_error(logger_ctx,
             "logger_libsir: Unable to set log filename");

            return false;
        }

        log_file.file = sir_addfile(filename, SIRL_ALL,
         (sir_options)SIRO_NONAME | (sir_options)SIRO_NOPID |
         (sir_options)SIRO_NOTID);
        if (!log_file.file) {
            char errmsg[SIR_MAXERROR];

            if (sir_geterror(errmsg) == SIR_E_NOERROR) {
                st_logger_error(logger_ctx,
                 "logger_libsir: Unable to add \"%s\" log file: %s", filename,
                 errmsg);
            } else {
                st_logger_error(logger_ctx,
                 "logger_libsir: Unable to add \"%s\" log file", filename);
            }

            return false;
        }

        if (!sir_filelevels(log_file.file, (sir_levels)levels)) {
            char errmsg[SIR_MAXERROR];

            if (sir_geterror(errmsg) == SIR_E_NOERROR) {
                st_logger_error(logger_ctx,
                 "logger_libsir: Unable to set \"%s\" log file levels: %s",
                 filename, errmsg);
            } else {
                st_logger_error(logger_ctx,
                 "logger_libsir: Unable to set \"%s\" log file levels",
                 filename);
            }

            sir_remfile(log_file.file);

            return false;
        }

        if (!!st_dlist_push_back(logger_ctx->log_files, &log_file)) {
            sir_remfile(log_file.file);

            st_logger_error(logger_ctx,
             "logger_libsir: Unable to add \"%s\" log file entry to the list",
             filename);

            return false;
        }
    }

    return true;
}

static bool st_logger_set_callback(st_loggerctx_t *logger_ctx, st_logcbk_t func,
 void *userdata, st_loglvl_t levels) {
    st_dlnode_t *node;

    if (!logger_ctx || !func)
        return false;

    if (!logger_ctx->callbacks)
        return false;

    node = st_dlist_get_head(logger_ctx->callbacks);

    while (node) {
        st_logger_libsir_callback_t *callback = st_dlist_get_data(node);

        if (callback->func == func && callback->userdata == userdata) {
            if (levels == ST_LL_NONE)
                st_dlist_remove(node);
            else
                callback->log_levels = levels;

            return true;
        }

        node = st_dlist_get_next(node);
    }

    if (levels != ST_LL_NONE) {
        st_logger_libsir_callback_t callback = {
            .func       = func,
            .userdata   = userdata,
            .log_levels = levels,
        };

        if (!!st_dlist_push_back(logger_ctx->callbacks, &callback)) {
            st_logger_error(logger_ctx,
             "libsir: Unable to add callback entry to the list");

            return false;
        }
    }

    return true;
}

#define ST_LOGGER_MESSAGE_LEN_MAX 4096
#define ST_LOGGER_OUTPUT_FUNC(st_func, sir_func, log_level, evtype_id_field) \
    static __attribute__ ((format (printf, 2, 0))) void st_func(             \
     const st_loggerctx_t *logger_ctx, const char* format, va_list args) {      \
        char                message[ST_LOGGER_MESSAGE_LEN_MAX];              \
        vsnprintf(message, ST_LOGGER_MESSAGE_LEN_MAX, format, args);         \
        sir_func("%s", message);                                             \
        if (logger_ctx->callbacks) {                                             \
            st_dlnode_t *node = st_dlist_get_head(logger_ctx->callbacks);        \
            while (node) {                                                   \
                st_logger_libsir_callback_t *callback = st_dlist_get_data(   \
                 node);                                                      \
                if ((callback->log_levels & log_level) == log_level)         \
                    callback->func(message, callback->userdata);             \
                node = st_dlist_get_next(node);                              \
            }                                                                \
        }                                                                    \
        if (logger_ctx->events_ctx) {                                            \
            size_t min_size = ST_EV_LOG_MSG_SIZE < ST_LOGGER_MESSAGE_LEN_MAX \
                ? ST_EV_LOG_MSG_SIZE                                         \
                : ST_LOGGER_MESSAGE_LEN_MAX;                                 \
            message[min_size - 1] = '\0';                                    \
        }                                                                    \
    }

    //     if (logger_ctx->events_ctx) {                                            \
    //         size_t min_size = ST_EV_LOG_MSG_SIZE < ST_LOGGER_MESSAGE_LEN_MAX \
    //             ? ST_EV_LOG_MSG_SIZE                                         \
    //             : ST_LOGGER_MESSAGE_LEN_MAX;                                 \
    //         message[min_size - 1] = '\0';                                    \
    // >>>     logger_ctx->events.push(logger_ctx->events_ctx, logger_ctx->evtype_id_field, \
    // >>>      message);                                                       \
    //     }                                                                    \
    // }

ST_LOGGER_OUTPUT_FUNC(st_logger_debug_output, sir_debug, ST_LL_DEBUG, // NOLINT(cert-err33-c)
 ev_log_output_debug);
ST_LOGGER_OUTPUT_FUNC(st_logger_info_output, sir_info, ST_LL_INFO, // NOLINT(cert-err33-c)
 ev_log_output_info);
ST_LOGGER_OUTPUT_FUNC(st_logger_warning_output, sir_warn, ST_LL_WARNING, // NOLINT(cert-err33-c)
 ev_log_output_warning);
ST_LOGGER_OUTPUT_FUNC(st_logger_error_output, sir_error, ST_LL_ERROR, // NOLINT(cert-err33-c)
 ev_log_output_warning);

#define ST_LOGGER_LOG_FUNC(st_func, st_output_func) \
    static __attribute__((format (printf, 2, 3))) void st_func(   \
     const st_loggerctx_t *logger_ctx, const char* format, ...) {    \
        va_list             args;                                 \
        va_start(args, format);                                   \
        st_output_func(logger_ctx, format, args);                 \
        va_end(args);                                             \
    }

ST_LOGGER_LOG_FUNC(st_logger_debug,   st_logger_debug_output);
ST_LOGGER_LOG_FUNC(st_logger_info,    st_logger_info_output);
ST_LOGGER_LOG_FUNC(st_logger_warning, st_logger_warning_output);
ST_LOGGER_LOG_FUNC(st_logger_error,   st_logger_error_output);

static void st_logger_set_postmortem_msg(st_loggerctx_t *logger_ctx,
 const char *msg) {
    snprintf(logger_ctx->postmortem_msg, ST_POSTMORTEM_MSG_SIZE_MAX, "%s", msg);
}
