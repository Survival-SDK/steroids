#include "sdl3.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "steroids/moddata.h"
#include "steroids/modsmgr.h"
#include "steroids/modules/keyboard.h"

#define DISPLAY_BOUNDS_UNKNOWN (SDL_Rect){0}

static st_dpsrvconnctx_t *st_dpsrvconn_init(const st_param_t params[]);
static void st_dpsrvconn_quit(st_dpsrvconnctx_t *dpsrvconn_ctx);
static void st_dpsrvconn_window_destroy(st_window_t *window);

static int st_dpsrvconn_get_monitors_count(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
static int st_dpsrvconn_get_primary_monitor_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
static const st_monitor_t *st_dpsrvconn_get_monitor_by_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx, unsigned index);
static st_monitor_t *st_dpsrvconn_get_monitor_by_id(
 st_dpsrvconnctx_t *dpsrvconn_ctx, uintptr_t id);
static const st_monitor_t *st_dpsrvconn_get_primary_monitor(
 const st_dpsrvconnctx_t *dpsrvconn_ctx);
static st_window_t *st_dpsrvconn_open_window(st_dpsrvconnctx_t *dpsrvconn_ctx,
 st_monitor_t *monitor, int x, int y, unsigned width, unsigned height,
 bool fullscreen, const char *title);
static void st_dpsrvconn_process(st_dpsrvconnctx_t *dpsrvconn_ctx);

static unsigned st_dpsrvconn_get_monitor_width(const st_monitor_t *monitor);
static unsigned st_dpsrvconn_get_monitor_height(const st_monitor_t *monitor);
static int st_dpsrvconn_get_monitor_index(const st_monitor_t *monitor);
static const char *st_dpsrvconn_get_monitor_name(const st_monitor_t *monitor);
static bool st_dpsrvconn_is_monitor_primary(const st_monitor_t *monitor);
static void *st_dpsrvconn_get_monitor_device_handle(
 const st_monitor_t *monitor);
static void *st_dpsrvconn_get_monitor_native_device_handle(
 const st_monitor_t *monitor);

static bool st_dpsrvconn_is_window_xed(const st_window_t *window);
static const st_monitor_t *st_dpsrvconn_get_window_monitor(
 const st_window_t *window);
static void *st_dpsrvconn_get_window_handle(const st_window_t *window);
static void *st_dpsrvconn_get_window_native_handle(const st_window_t *window);
static unsigned st_dpsrvconn_get_window_width(const st_window_t *window);
static unsigned st_dpsrvconn_get_window_height(const st_window_t *window);

static st_dpsrvconnctx_funcs_t dpsrvconnctx_funcs = {
    st_modctx_funcs,
    .get_monitors_count        = st_dpsrvconn_get_monitors_count,
    .get_primary_monitor_index = st_dpsrvconn_get_primary_monitor_index,
    .get_monitor_by_index      = st_dpsrvconn_get_monitor_by_index,
    .get_monitor_by_id         = st_dpsrvconn_get_monitor_by_id,
    .get_primary_monitor       = st_dpsrvconn_get_primary_monitor,
    .open_window               = st_dpsrvconn_open_window,
    .process                   = st_dpsrvconn_process,
};

static st_monitor_funcs_t monitor_funcs = {
    st_object_funcs,
    .get_width                = st_dpsrvconn_get_monitor_width,
    .get_height               = st_dpsrvconn_get_monitor_height,
    .get_index                = st_dpsrvconn_get_monitor_index,
    .get_name                 = st_dpsrvconn_get_monitor_name,
    .is_primary               = st_dpsrvconn_is_monitor_primary,
    .get_device_handle        = st_dpsrvconn_get_monitor_device_handle,
    .get_native_device_handle = st_dpsrvconn_get_monitor_native_device_handle,
};

static st_window_funcs_t window_funcs = {
    st_object_funcs,
    .xed               = st_dpsrvconn_is_window_xed,
    .get_monitor       = st_dpsrvconn_get_window_monitor,
    .get_handle        = st_dpsrvconn_get_window_handle,
    .get_native_handle = st_dpsrvconn_get_window_native_handle,
    .get_width         = st_dpsrvconn_get_window_width,
    .get_height        = st_dpsrvconn_get_window_height,
};

static const st_modprerq_t mod_prereqs[] = {
    { "events", NULL, },
    { "logger", NULL, },
    { "sdl3loader", "sdl3", },
    {0},
};

st_moddata_t *st_module_dpsrvconn_sdl3_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new("dpsrvconn", "sdl3", ST_MODULE_TYPE, mod_prereqs,
     st_dpsrvconn_init, modsmgr);
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_dpsrvconn_sdl3_init(modsmgr);
}
#endif

static const char *st_module_subsystem = "dpsrvconn";
static const char *st_module_name = "sdl3";

void *load_sdl3_func(st_dpsrvconnctx_t *dpsrvconn_ctx, bool *all_funcs_loaded, 
 const char *func_name) {
    void *func = ST_SDL3LOADERCTX_CALL(dpsrvconn_ctx->sdl3loader_ctx, 
     get_proc_address, func_name);
    
    if (!func)
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to load function \"%s\"", st_module_subsystem,
         st_module_name, func_name);

    *all_funcs_loaded &= !!func;

    return func;
}

bool load_sdl3_funcs(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    bool all_funcs_loaded = true;
    
    /* subsystem */
    dpsrvconn_ctx->sdl3.init_subsystem = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_InitSubSystem");
    dpsrvconn_ctx->sdl3.quit_subsystem = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_QuitSubSystem");
    dpsrvconn_ctx->sdl3.was_init = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_WasInit");

    /* events */
    dpsrvconn_ctx->sdl3.poll_event = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_PollEvent");

    /* video */
    dpsrvconn_ctx->sdl3.get_current_video_driver = load_sdl3_func(
     dpsrvconn_ctx, &all_funcs_loaded, "SDL_GetCurrentVideoDriver");

    /* display */
    dpsrvconn_ctx->sdl3.get_primary_display = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetPrimaryDisplay");
    dpsrvconn_ctx->sdl3.get_displays = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetDisplays");
    dpsrvconn_ctx->sdl3.get_display_name = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetDisplayName");
    dpsrvconn_ctx->sdl3.get_display_bounds = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetDisplayBounds");
    dpsrvconn_ctx->sdl3.get_display_properties = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetDisplayProperties");

    /* window */
    dpsrvconn_ctx->sdl3.create_window = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_CreateWindow");
    dpsrvconn_ctx->sdl3.create_window_with_properties = load_sdl3_func(
     dpsrvconn_ctx, &all_funcs_loaded, "SDL_CreateWindowWithProperties");
    dpsrvconn_ctx->sdl3.destroy_window = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_DestroyWindow");
    dpsrvconn_ctx->sdl3.get_windows = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetWindows");
    dpsrvconn_ctx->sdl3.get_window_properties = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetWindowProperties");
    dpsrvconn_ctx->sdl3.get_window_id = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetWindowID");
    dpsrvconn_ctx->sdl3.get_window_from_id = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetWindowFromID");
    dpsrvconn_ctx->sdl3.get_display_for_window = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetDisplayForWindow");

    /* properties */
    dpsrvconn_ctx->sdl3.create_properties = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_CreateProperties");
    dpsrvconn_ctx->sdl3.destroy_properties = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_DestroyProperties");
    dpsrvconn_ctx->sdl3.set_string_property = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_SetStringProperty");
    dpsrvconn_ctx->sdl3.set_number_property = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_SetNumberProperty");
    dpsrvconn_ctx->sdl3.set_boolean_property = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_SetBooleanProperty");
    dpsrvconn_ctx->sdl3.set_pointer_property = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_SetPointerProperty");
    dpsrvconn_ctx->sdl3.get_number_property = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetNumberProperty");
    dpsrvconn_ctx->sdl3.get_pointer_property = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetPointerProperty");

    /* misc */
    dpsrvconn_ctx->sdl3.set_hint = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_SetHint");
    dpsrvconn_ctx->sdl3.free = load_sdl3_func(dpsrvconn_ctx, &all_funcs_loaded, 
     "SDL_free");
    dpsrvconn_ctx->sdl3.get_error = load_sdl3_func(dpsrvconn_ctx, 
     &all_funcs_loaded, "SDL_GetError");

    return all_funcs_loaded;
}

static bool init_sdl3(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    dpsrvconn_ctx->sdl3.set_hint(SDL_HINT_VIDEO_DRIVER, "wayland,x11");

    if (dpsrvconn_ctx->sdl3.was_init(SDL_INIT_EVENTS)) {
        dpsrvconn_ctx->owns_sdl3_events_subsystem = false;
    } else {
        if (!dpsrvconn_ctx->sdl3.init_subsystem(SDL_INIT_EVENTS)) {
            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
            "%s_%s: Unable to initialize SDL3 events subsystems", 
            st_module_subsystem, st_module_name);

            return false;
        }
        dpsrvconn_ctx->owns_sdl3_events_subsystem = true;
    }

    if (dpsrvconn_ctx->sdl3.was_init(SDL_INIT_VIDEO)) {
        dpsrvconn_ctx->owns_sdl3_video_subsystem = false;
    } else {
        if (!dpsrvconn_ctx->sdl3.init_subsystem(SDL_INIT_VIDEO)) {
            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
            "%s_%s: Unable to initialize SDL3 video subsystems", 
            st_module_subsystem, st_module_name);

            if (dpsrvconn_ctx->owns_sdl3_events_subsystem)
                dpsrvconn_ctx->sdl3.quit_subsystem(SDL_INIT_EVENTS);

            return false;
        }
        dpsrvconn_ctx->owns_sdl3_video_subsystem = true;
    }

    return true;
}

static void update_current_video_driver(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    const char *video_driver = dpsrvconn_ctx->sdl3.get_current_video_driver();
    
    if (!video_driver) {        
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to detect current video backend. Using native device "
         "handle is not supported on this run", st_module_subsystem,
         st_module_name);

        dpsrvconn_ctx->video_driver = VD_UNKNOWN;

        return;
    }

    if (strcmp(video_driver, "x11") == 0)
        dpsrvconn_ctx->video_driver = VD_X11;
    else if (strcmp(video_driver, "wayland") == 0)
        dpsrvconn_ctx->video_driver = VD_WAYLAND;
    else if (strcmp(video_driver, "windows") == 0)
        dpsrvconn_ctx->video_driver = VD_WINDOWS;
    else {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
         "%s_%s: Unknown video backend \"%s\"", st_module_subsystem,
         st_module_name, video_driver);

        dpsrvconn_ctx->video_driver = VD_UNKNOWN;
    }
}

static void fill_current_monitor_handles(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 SDL_DisplayID *current_monitor_handles, int *current_monitors_count) {
    for (*current_monitors_count = 0; 
     *current_monitors_count < dpsrvconn_ctx->monitors_count; 
     (*current_monitors_count)++)
        current_monitor_handles[*current_monitors_count] = 
         dpsrvconn_ctx->monitors[*current_monitors_count].handle;
}

static bool is_monitor_exists(const SDL_DisplayID *current_monitor_handles, 
 int current_monitors_count, SDL_DisplayID monitor_handle) {
    for (int i = 0; i < current_monitors_count; i++) {
        if (current_monitor_handles[i] == monitor_handle)
            return true;
    }
    return false;
}

static int get_removed_monitor_internal_index(
 const SDL_DisplayID *current_monitor_handles, int current_monitors_count, 
 const SDL_DisplayID *new_monitor_handles, int new_monitors_count) {
    bool not_removed_array[MONITORS_MAX];

    for (int i = 0; i < MONITORS_MAX; i++)
        not_removed_array[i] = false;
    
    for (int cur_i = 0; cur_i < current_monitors_count; cur_i++) {
        for (int new_i = 0; new_i < new_monitors_count; new_i++) {
            if (current_monitor_handles[cur_i] == new_monitor_handles[new_i])
                not_removed_array[cur_i] = true;
        }
    }
    for (int i = 0; i < current_monitors_count; i++)
        if (!not_removed_array[i])
            return i;

    return -1;
}

static void remove_monitor(st_dpsrvconnctx_t *dpsrvconn_ctx, unsigned index) {
    st_evmondisc_t disconnect_event = {
        .id = dpsrvconn_ctx->monitors[index].handle,
        .index = index,
    };
    st_evmonreidx_t reindex_event = {
        .monitor = &dpsrvconn_ctx->monitors[index],
        .new_index = index,
    };
    /* noop(close monitor)  */
    dpsrvconn_ctx->monitors[index] = dpsrvconn_ctx->monitors[
     dpsrvconn_ctx->monitors_count - 1];
    dpsrvconn_ctx->monitors_count--;

    ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
     dpsrvconn_ctx->evtypes[EV_MONITOR_DISCONNECTED], &disconnect_event);

    ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
     dpsrvconn_ctx->evtypes[EV_MONITOR_REINDEX], &reindex_event);
}

/* Monitor object destructor.
 * We need not to free memory because monitor object created with placement new 
 */
static void st_dpsrvconn_close_monitor(st_monitor_t *monitor) {
    if (monitor->name)
        free(monitor->name);
}

static void add_monitor(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 SDL_DisplayID monitor_handle, bool first_call) {
    SDL_DisplayID  primary_display = dpsrvconn_ctx->sdl3.get_primary_display();
    st_evmonconn_t connect_event = {0};
    st_monitor_t  *monitor = (st_monitor_t *)st_object_placement_new(
     &dpsrvconn_ctx->monitors[dpsrvconn_ctx->monitors_count], &monitor_funcs,
     (st_object_dtor_t)st_dpsrvconn_close_monitor, 
     (st_object_t *)dpsrvconn_ctx);

    monitor->native_handle = NULL;

    if (dpsrvconn_ctx->video_driver == VD_WINDOWS) {
        SDL_PropertiesID props_id = dpsrvconn_ctx->sdl3.get_display_properties(
         monitor_handle);

        if (props_id)
            monitor->native_handle = dpsrvconn_ctx->sdl3.get_pointer_property(
             props_id, SDL_PROP_DISPLAY_WINDOWS_HMONITOR_POINTER, NULL);
        else
            ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
             "%s_%s: Unable to get monitor properties. This monitor will not "
             "be used as native device handle on this run", 
             st_module_subsystem, st_module_name);
    }

    monitor->name = strdup(dpsrvconn_ctx->sdl3.get_display_name(
     monitor_handle));

    monitor->handle = monitor_handle;
    monitor->bounds = DISPLAY_BOUNDS_UNKNOWN;
    monitor->is_primary = (monitor_handle == primary_display);

    if (!dpsrvconn_ctx->sdl3.get_display_bounds(monitor_handle, 
     &monitor->bounds)) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to get monitor bounds: %s", st_module_subsystem,
         st_module_name, dpsrvconn_ctx->sdl3.get_error());

        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
         "%s_%s: This monitor can not be used for manually placing windows on "
         "it", st_module_subsystem, st_module_name);
    }

    connect_event.monitor = monitor;
    connect_event.id = monitor_handle;
    connect_event.index = dpsrvconn_ctx->monitors_count;
    dpsrvconn_ctx->monitors_count++;
    if (first_call)
        ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
         dpsrvconn_ctx->evtypes[EV_MONITOR_CONNECTED], &connect_event);
}

static st_monitor_t *get_monitor_by_sdl_display_id(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 SDL_DisplayID display_id) {
    for (int i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (dpsrvconn_ctx->monitors[i].handle == display_id)
            return &dpsrvconn_ctx->monitors[i];
    }

    return NULL;
}

static st_window_t *get_window_by_sdl_window_id(
 st_dpsrvconnctx_t *dpsrvconn_ctx, SDL_WindowID window_id) {
    SDL_Window      *sdl_window = dpsrvconn_ctx->sdl3.get_window_from_id(
     window_id);
    SDL_PropertiesID props = dpsrvconn_ctx->sdl3.get_window_properties(
     sdl_window);

   return (st_window_t *)dpsrvconn_ctx->sdl3.get_pointer_property(props, 
    "st_window", NULL);
}

static void reset_window_monitors(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    int          count;
    SDL_Window **sdl3_windows = dpsrvconn_ctx->sdl3.get_windows(&count);

    for (int i = 0; i < count; i++) {
        SDL_WindowID  id = dpsrvconn_ctx->sdl3.get_window_id(sdl3_windows[i]);
        SDL_DisplayID actual_display_id = dpsrvconn_ctx->sdl3.
         get_display_for_window(sdl3_windows[i]);
        st_window_t  *window = get_window_by_sdl_window_id(dpsrvconn_ctx, id);
        st_monitor_t *monitor = get_monitor_by_sdl_display_id(dpsrvconn_ctx, 
         actual_display_id);

        window->monitor = monitor;
    }

    dpsrvconn_ctx->sdl3.free(sdl3_windows);
}

static bool update_monitors(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    static bool    first_call = true;

    SDL_DisplayID  current_monitor_handles[MONITORS_MAX];
    int            current_monitors_count;
    int            new_monitors_count;
    SDL_DisplayID *new_monitor_handles = dpsrvconn_ctx->sdl3.get_displays(
     &new_monitors_count);
    int            removed_monitor_internal_index = -1;

    if (!new_monitor_handles) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to get monitors data", st_module_subsystem,
         st_module_name);

        return false;
    }

    if (new_monitors_count > MONITORS_MAX) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
         "%s_%s: Found more than maximum allowed %u monitors. That monitors "
         "will not be available on this run", st_module_subsystem, 
         st_module_name, MONITORS_MAX);

        new_monitors_count = MONITORS_MAX;
    }

    fill_current_monitor_handles(dpsrvconn_ctx, current_monitor_handles, 
     &current_monitors_count);

    removed_monitor_internal_index = get_removed_monitor_internal_index(
     current_monitor_handles, current_monitors_count, new_monitor_handles, 
     new_monitors_count);

    if (removed_monitor_internal_index != -1) {
        remove_monitor(dpsrvconn_ctx, removed_monitor_internal_index);

        dpsrvconn_ctx->sdl3.free(new_monitor_handles);

        return update_monitors(dpsrvconn_ctx);
    }

    for (int i = 0; i < new_monitors_count; i++) {
        bool monitor_exists = is_monitor_exists(
         current_monitor_handles, current_monitors_count, 
         new_monitor_handles[i]);

        if (!monitor_exists) {
            add_monitor(dpsrvconn_ctx, new_monitor_handles[i], first_call);

            break;
        }
    }

    reset_window_monitors(dpsrvconn_ctx);

    dpsrvconn_ctx->sdl3.free(new_monitor_handles);

    first_call = false;

    return true;
}

static st_monitor_t *get_monitor_by_window_handle(
 st_dpsrvconnctx_t *dpsrvconn_ctx, SDL_Window *handle) {
    SDL_DisplayID display_id = dpsrvconn_ctx->sdl3.get_display_for_window(
     handle);

    for (int i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (dpsrvconn_ctx->monitors[i].handle == display_id)
            return &dpsrvconn_ctx->monitors[i];
    }

    return NULL;
}

static st_dpsrvconnctx_t *st_dpsrvconn_init(const st_param_t params[]) {
    st_modsmgr_t       *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    st_loggerctx_t     *logger_ctx;
    st_eventsctx_t     *events_ctx;
    st_sdl3loaderctx_t *sdl3loader_ctx;
    st_dpsrvconnctx_t  *dpsrvconn_ctx;

    if (!modsmgr)
        return NULL;
    
    logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "logger", NULL);
    if (!logger_ctx)
        return NULL;

    events_ctx = (st_eventsctx_t *)ST_MODSMGR_CALL(modsmgr, get_singleton, 
     "events", NULL);
    if (!events_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to get events context", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    if (ST_MODSMGR_CALL(modsmgr, have_singleton, "sdl3loader", "sdl3")) {
        sdl3loader_ctx = (st_sdl3loaderctx_t *)ST_MODSMGR_CALL(modsmgr, 
         get_singleton, "sdl3loader", NULL);
        if (!sdl3loader_ctx) {
            ST_LOGGERCTX_CALL(logger_ctx, error,
            "%s_%s: Unable to get sdl3loader context", st_module_subsystem,
            st_module_name);

            return NULL;
        }
    } else {
        sdl3loader_ctx = (st_sdl3loaderctx_t *)ST_MODSMGR_CALL(modsmgr, 
         create_singleton, "sdl3loader", NULL, (st_params_t){{0}});
        if (!sdl3loader_ctx) {
            ST_LOGGERCTX_CALL(logger_ctx, error,
            "%s_%s: Unable to create sdl3loader context", st_module_subsystem,
            st_module_name);

            return NULL;
        }
    }

    dpsrvconn_ctx = (st_dpsrvconnctx_t *)st_modctx_new(st_module_subsystem,
     st_module_name, sizeof(st_dpsrvconnctx_t), NULL, &dpsrvconnctx_funcs,
     (st_object_dtor_t)st_dpsrvconn_quit);
    if (!dpsrvconn_ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
         "%s_%s: Unable to create display server connection context", 
         st_module_subsystem, st_module_name);

        return NULL;
    }

    dpsrvconn_ctx->logger_ctx = logger_ctx;
    dpsrvconn_ctx->events_ctx = events_ctx;
    dpsrvconn_ctx->sdl3loader_ctx = sdl3loader_ctx;
    if (!load_sdl3_funcs(dpsrvconn_ctx))
        goto load_funcs_fail;

    if (!init_sdl3(dpsrvconn_ctx))
        goto init_sdl3_fail;

    update_current_video_driver(dpsrvconn_ctx);

    /* We can't get display pointer on X11 and Wayland before first window is 
     * created */
    dpsrvconn_ctx->native_display = NULL;
    memset(dpsrvconn_ctx->monitors, 0, sizeof(st_monitor_t) * MONITORS_MAX);
    dpsrvconn_ctx->monitors_count = 0;

    /* Register event types */
    dpsrvconn_ctx->evtypes[EV_MONITOR_CONNECTED] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_connected",
     sizeof(st_evmonconn_t));
    dpsrvconn_ctx->evtypes[EV_MONITOR_DISCONNECTED] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_disconnected",
     sizeof(st_evmondisc_t));
    dpsrvconn_ctx->evtypes[EV_MONITOR_REINDEX] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_reindex",
     sizeof(st_evmonreidx_t));
    dpsrvconn_ctx->evtypes[EV_MONITOR_RESIZE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "monitor_resize",
     sizeof(st_evmonresize_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_PRESS] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_press",
     sizeof(st_evwinunsigned_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_RELEASE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_release",
     sizeof(st_evwinunsigned_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_WHEEL] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_wheel",
     sizeof(st_evwininteger_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_MOVE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_move",
     sizeof(st_evwinuvec2_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_ENTER] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_enter",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_MOUSE_LEAVE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "mouse_leave",
     sizeof(st_evwinnoargs_t));

    dpsrvconn_ctx->evtypes[EV_KEY_PRESS] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "key_press",
     sizeof(st_evwinu64_t));
    dpsrvconn_ctx->evtypes[EV_KEY_RELEASE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "key_release",
     sizeof(st_evwinu64_t));
    dpsrvconn_ctx->evtypes[EV_KEY_INPUT] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "key_input",
     sizeof(st_evwinsymbol_t));

    dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_IN] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_focus_in",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_OUT] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_focus_out",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_RESIZE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_resize",
     sizeof(st_evwinuvec2_t));
    dpsrvconn_ctx->evtypes[EV_WIN_PLACE_ON_TOP] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_place_on_top",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_PLACE_ON_BOTTOM] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_place_on_bottom",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_CREATE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_create",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_DESTROY] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_destroy",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_SHOW] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_show",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_HIDE] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_hide",
     sizeof(st_evwinnoargs_t));
    dpsrvconn_ctx->evtypes[EV_WIN_MONITOR_CHANGED] = ST_EVENTSCTX_CALL(
     dpsrvconn_ctx->events_ctx, register_type, "window_monitor_changed",
     sizeof(st_evwinptr_t));
    
    if (!update_monitors(dpsrvconn_ctx))
        goto update_monitors_fail;

    ST_LOGGERCTX_CALL(logger_ctx, info,
     "%s_%s: Display server connection context initialized", 
     st_module_subsystem, st_module_name);

    return dpsrvconn_ctx;

update_monitors_fail:
    if (dpsrvconn_ctx->owns_sdl3_video_subsystem)
        dpsrvconn_ctx->sdl3.quit_subsystem(SDL_INIT_VIDEO);
    if (dpsrvconn_ctx->owns_sdl3_events_subsystem)
        dpsrvconn_ctx->sdl3.quit_subsystem(SDL_INIT_EVENTS);
init_sdl3_fail:
load_funcs_fail:
    free(dpsrvconn_ctx);
    
    return NULL;
}

static void st_dpsrvconn_quit(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, info,
     "%s_%s: Display server connection context destroyed", st_module_subsystem, 
     st_module_name);
    if (dpsrvconn_ctx->owns_sdl3_video_subsystem)
        dpsrvconn_ctx->sdl3.quit_subsystem(SDL_INIT_VIDEO);
    if (dpsrvconn_ctx->owns_sdl3_events_subsystem)
        dpsrvconn_ctx->sdl3.quit_subsystem(SDL_INIT_EVENTS);
    free(dpsrvconn_ctx);
}

static int st_dpsrvconn_get_monitors_count(
 const st_dpsrvconnctx_t *dpsrvconn_ctx) {
    return dpsrvconn_ctx->monitors_count;
}

static int st_dpsrvconn_get_primary_monitor_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx) {
    for (unsigned i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (dpsrvconn_ctx->monitors[i].is_primary)
            return i;
    }

    return -1;
}

static const st_monitor_t *st_dpsrvconn_get_monitor_by_index(
 const st_dpsrvconnctx_t *dpsrvconn_ctx, unsigned index) {
    if (index >= dpsrvconn_ctx->monitors_count) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Incorrect monitor index %u", st_module_subsystem,
         st_module_name, index);

        return NULL;
    }

    return &dpsrvconn_ctx->monitors[index];
}

static st_monitor_t *st_dpsrvconn_get_monitor_by_id(
 st_dpsrvconnctx_t *dpsrvconn_ctx, uintptr_t id) {
    for (unsigned i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (dpsrvconn_ctx->monitors[i].handle == id)
            return &dpsrvconn_ctx->monitors[i];
    }

    return NULL;
}

static const st_monitor_t *st_dpsrvconn_get_primary_monitor(
 const st_dpsrvconnctx_t *dpsrvconn_ctx) {
    int primary_index = st_dpsrvconn_get_primary_monitor_index(dpsrvconn_ctx);

    if (primary_index == -1) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: No primary monitor found", st_module_subsystem,
         st_module_name);

        return NULL;
    }

    return st_dpsrvconn_get_monitor_by_index(dpsrvconn_ctx, primary_index);
}

static st_window_t *st_dpsrvconn_open_window(st_dpsrvconnctx_t *dpsrvconn_ctx,
 st_monitor_t *monitor, int x, int y, unsigned width, unsigned height,
 bool fullscreen, const char *title) {
    st_window_t     *window;
    SDL_Rect        *monitor_bounds;
    SDL_Window      *handle;
    SDL_PropertiesID props = dpsrvconn_ctx->sdl3.create_properties();
    SDL_PropertiesID actual_props;
    st_evwinnoargs_t create_event;

    assert(monitor);
    monitor_bounds = &monitor->bounds;

    if (props) {
        if (fullscreen) {
            if (monitor_bounds->w != width || monitor_bounds->h != height) {
                ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
                 "%s_%s: Window sizes are ignored for fullscreen window on "
                 "this implementation", 
                 st_module_subsystem, st_module_name);

                width = monitor_bounds->w;
                height = monitor_bounds->h;
            }

            if (x != monitor_bounds->x || y != monitor_bounds->y)
                ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, warning,
                 "%s_%s: Window positions are ignored for fullscreen window", 
                 st_module_subsystem, st_module_name);
        }

        x = monitor_bounds->x + x * !fullscreen;
        y = monitor_bounds->y + y * !fullscreen;

        dpsrvconn_ctx->sdl3.set_string_property(props, 
         SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
        dpsrvconn_ctx->sdl3.set_number_property(props, 
         SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
        dpsrvconn_ctx->sdl3.set_number_property(props, 
         SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
        dpsrvconn_ctx->sdl3.set_number_property(props, 
         SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
        dpsrvconn_ctx->sdl3.set_number_property(props, 
         SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);

        dpsrvconn_ctx->sdl3.set_boolean_property(props, 
         SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, fullscreen);

        if (dpsrvconn_ctx->video_driver == VD_WAYLAND)
            dpsrvconn_ctx->sdl3.set_boolean_property(props, 
             SDL_PROP_WINDOW_CREATE_WAYLAND_CREATE_EGL_WINDOW_BOOLEAN, true);

        handle = dpsrvconn_ctx->sdl3.create_window_with_properties(props);

        dpsrvconn_ctx->sdl3.destroy_properties(props);
    } else {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to create SDL_PropertiesID for new window. Window will "
         "be created on primary or first available monitor", 
         st_module_subsystem, st_module_name);

        handle = dpsrvconn_ctx->sdl3.create_window(title, width, height, 
         fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    }

    if (!handle) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to create window: %s", st_module_subsystem, 
         st_module_name, dpsrvconn_ctx->sdl3.get_error());

        return NULL;
    }

    window = (st_window_t *)st_object_new(sizeof(st_window_t), &window_funcs,
     (st_object_dtor_t)st_dpsrvconn_window_destroy, 
     (st_object_t *)dpsrvconn_ctx);

    if (!window) {
        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to allocate memory for window object", 
         st_module_subsystem, st_module_name);

        dpsrvconn_ctx->sdl3.destroy_window(handle);

        return NULL;
    }

    window->handle = handle;
    window->width = width;
    window->height = height;
    window->xed = false;

    window->monitor = get_monitor_by_window_handle(dpsrvconn_ctx, handle);

    actual_props = dpsrvconn_ctx->sdl3.get_window_properties(handle);
    if (actual_props) {
        dpsrvconn_ctx->sdl3.set_pointer_property(actual_props, "st_window", 
         window);
        
        switch (dpsrvconn_ctx->video_driver) {
            case VD_X11: {
                dpsrvconn_ctx->native_display = dpsrvconn_ctx->sdl3.
                 get_pointer_property(actual_props, 
                  SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
                window->native_handle = (void *)(uintptr_t)dpsrvconn_ctx->sdl3.
                 get_number_property(actual_props, 
                  SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);

                break;
            }
            case VD_WAYLAND: {
                dpsrvconn_ctx->native_display = dpsrvconn_ctx->sdl3.
                 get_pointer_property(actual_props, 
                  SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
                window->native_handle = dpsrvconn_ctx->sdl3.
                 get_pointer_property(actual_props,
                  SDL_PROP_WINDOW_WAYLAND_EGL_WINDOW_POINTER, NULL);

                break;
            }
            case VD_WINDOWS: {
                window->native_handle = dpsrvconn_ctx->sdl3.
                 get_pointer_property(actual_props,
                  SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

                break;
            }
            default:
                ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
                 "%s_%s: Unsupported video backend %u", st_module_subsystem,
                 st_module_name, dpsrvconn_ctx->video_driver);

                return NULL;
        }
    } else {
        const char *native_handle_owners = dpsrvconn_ctx->video_driver == VD_X11
            ? "window and display"
            : "window";

        ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, error,
         "%s_%s: Unable to get window properties. This %s will not have a "
         "native handles on this run", st_module_subsystem,
         st_module_name, native_handle_owners);

        return NULL;
    }

    ST_LOGGERCTX_CALL(dpsrvconn_ctx->logger_ctx, debug,
     "%s_%s: Window created on monitor %s", st_module_subsystem, st_module_name, 
     window->monitor 
        ? (window->monitor->name ? window->monitor->name : "(unnamed)")
        : "(out of bounds of any monitor)"
    );

    create_event.window = window;
    ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
     dpsrvconn_ctx->evtypes[EV_WIN_CREATE], &create_event);

    return window;
}

static void st_dpsrvconn_window_destroy(st_window_t *window) {
    st_dpsrvconnctx_t *dpsrvconn_ctx = (st_dpsrvconnctx_t *)ST_WINDOW_CALL(
     window, get_owner);

    st_evwinnoargs_t destroy_event = {
        .window = window,
    };

    ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
     dpsrvconn_ctx->evtypes[EV_WIN_DESTROY], &destroy_event);
    
    dpsrvconn_ctx->sdl3.destroy_window(window->handle);
    free(window);
}

/* Convert SDL3 keycode to engine's internal keycode (X11 KeySym compatible) */
static st_key_t sdl_keycode_to_st_key(SDL_Keycode sdl_key) {
    /* For printable ASCII characters (0x20-0x7E), SDL uses direct Unicode */
    if (sdl_key >= 0x20 && sdl_key <= 0x7E) {
        /* Convert lowercase letters to uppercase for ST_KEY_* format */
        return sdl_key >= 'a' && sdl_key <= 'z'
            ? sdl_key - 'a' + 'A'
            : sdl_key;
    }

    /* Special keys mapping from SDL to X11 KeySym */
    switch (sdl_key) {
        /* Control keys */
        case SDLK_BACKSPACE:    return ST_KEY_BACKSPACE;   /* 0xFF08 */
        case SDLK_TAB:          return ST_KEY_TAB;         /* 0xFF09 */
        case SDLK_RETURN:       return ST_KEY_RETURN;      /* 0xFF0D */
        case SDLK_PAUSE:        return ST_KEY_PAUSE;       /* 0xFF13 */
        case SDLK_SCROLLLOCK:   return ST_KEY_SCROLL_LOCK; /* 0xFF14 */
        case SDLK_ESCAPE:       return ST_KEY_ESCAPE;      /* 0xFF1B */
        case SDLK_DELETE:       return ST_KEY_DELETE;      /* 0xFFFF */

        /* Navigation keys */
        case SDLK_HOME:         return ST_KEY_HOME;        /* 0xFF50 */
        case SDLK_LEFT:         return ST_KEY_LEFT;        /* 0xFF51 */
        case SDLK_UP:           return ST_KEY_UP;          /* 0xFF52 */
        case SDLK_RIGHT:        return ST_KEY_RIGHT;       /* 0xFF53 */
        case SDLK_DOWN:         return ST_KEY_DOWN;        /* 0xFF54 */
        case SDLK_PAGEUP:       return ST_KEY_PAGE_UP;     /* 0xFF55 */
        case SDLK_PAGEDOWN:     return ST_KEY_PAGE_DOWN;   /* 0xFF56 */
        case SDLK_END:          return ST_KEY_END;         /* 0xFF57 */

        /* Editing keys */
        case SDLK_PRINTSCREEN:  return ST_KEY_PRINT;       /* 0xFF61 */
        case SDLK_INSERT:       return ST_KEY_INSERT;      /* 0xFF63 */
        case SDLK_MENU:         return ST_KEY_MENU;        /* 0xFF67 */
        case SDLK_HELP:         return ST_KEY_HELP;        /* 0xFF6A */
        case SDLK_NUMLOCKCLEAR: return ST_KEY_NUM_LOCK;    /* 0xFF7F */

        /* Keypad keys */
        case SDLK_KP_SPACE:     return ST_KEY_KP_SPACE;    /* 0xFF80 */
        case SDLK_KP_TAB:       return ST_KEY_KP_TAB;      /* 0xFF89 */
        case SDLK_KP_ENTER:     return ST_KEY_KP_ENTER;    /* 0xFF8D */
        case SDLK_KP_0:         return ST_KEY_KP_0;        /* 0xFFB0 */
        case SDLK_KP_1:         return ST_KEY_KP_1;        /* 0xFFB1 */
        case SDLK_KP_2:         return ST_KEY_KP_2;        /* 0xFFB2 */
        case SDLK_KP_3:         return ST_KEY_KP_3;        /* 0xFFB3 */
        case SDLK_KP_4:         return ST_KEY_KP_4;        /* 0xFFB4 */
        case SDLK_KP_5:         return ST_KEY_KP_5;        /* 0xFFB5 */
        case SDLK_KP_6:         return ST_KEY_KP_6;        /* 0xFFB6 */
        case SDLK_KP_7:         return ST_KEY_KP_7;        /* 0xFFB7 */
        case SDLK_KP_8:         return ST_KEY_KP_8;        /* 0xFFB8 */
        case SDLK_KP_9:         return ST_KEY_KP_9;        /* 0xFFB9 */
        case SDLK_KP_PERIOD:    return ST_KEY_KP_DECIMAL;  /* 0xFFAE */
        case SDLK_KP_DIVIDE:    return ST_KEY_KP_DIVIDE;   /* 0xFFAF */
        case SDLK_KP_MULTIPLY:  return ST_KEY_KP_MULTIPLY; /* 0xFFAA */
        case SDLK_KP_MINUS:     return ST_KEY_KP_SUBTRACT; /* 0xFFAD */
        case SDLK_KP_PLUS:      return ST_KEY_KP_ADD;      /* 0xFFAB */
        case SDLK_KP_EQUALS:    return ST_KEY_KP_EQUAL;    /* 0xFFBD */

        /* Function keys */
        case SDLK_F1:           return ST_KEY_F1;          /* 0xFFBE */
        case SDLK_F2:           return ST_KEY_F2;          /* 0xFFBF */
        case SDLK_F3:           return ST_KEY_F3;          /* 0xFFC0 */
        case SDLK_F4:           return ST_KEY_F4;          /* 0xFFC1 */
        case SDLK_F5:           return ST_KEY_F5;          /* 0xFFC2 */
        case SDLK_F6:           return ST_KEY_F6;          /* 0xFFC3 */
        case SDLK_F7:           return ST_KEY_F7;          /* 0xFFC4 */
        case SDLK_F8:           return ST_KEY_F8;          /* 0xFFC5 */
        case SDLK_F9:           return ST_KEY_F9;          /* 0xFFC6 */
        case SDLK_F10:          return ST_KEY_F10;         /* 0xFFC7 */
        case SDLK_F11:          return ST_KEY_F11;         /* 0xFFC8 */
        case SDLK_F12:          return ST_KEY_F12;         /* 0xFFC9 */
        case SDLK_F13:          return ST_KEY_F13;         /* 0xFFCA */
        case SDLK_F14:          return ST_KEY_F14;         /* 0xFFCB */
        case SDLK_F15:          return ST_KEY_F15;         /* 0xFFCC */
        case SDLK_F16:          return ST_KEY_F16;         /* 0xFFCD */
        case SDLK_F17:          return ST_KEY_F17;         /* 0xFFCE */
        case SDLK_F18:          return ST_KEY_F18;         /* 0xFFCF */
        case SDLK_F19:          return ST_KEY_F19;         /* 0xFFD0 */
        case SDLK_F20:          return ST_KEY_F20;         /* 0xFFD1 */
        case SDLK_F21:          return ST_KEY_F21;         /* 0xFFD2 */
        case SDLK_F22:          return ST_KEY_F22;         /* 0xFFD3 */
        case SDLK_F23:          return ST_KEY_F23;         /* 0xFFD4 */
        case SDLK_F24:          return ST_KEY_F24;         /* 0xFFD5 */

        /* Modifier keys */
        case SDLK_LSHIFT:       return ST_KEY_SHIFT_L;     /* 0xFFE1 */
        case SDLK_RSHIFT:       return ST_KEY_SHIFT_R;     /* 0xFFE2 */
        case SDLK_LCTRL:        return ST_KEY_CONTROL_L;   /* 0xFFE3 */
        case SDLK_RCTRL:        return ST_KEY_CONTROL_R;   /* 0xFFE4 */
        case SDLK_CAPSLOCK:     return ST_KEY_CAPS_LOCK;   /* 0xFFE5 */
        case SDLK_LALT:         return ST_KEY_ALT_L;       /* 0xFFE9 */
        case SDLK_RALT:         return ST_KEY_ALT_R;       /* 0xFFEA */
        case SDLK_LGUI:         return ST_KEY_SUPER_L;     /* 0xFFEB */
        case SDLK_RGUI:         return ST_KEY_SUPER_R;     /* 0xFFEC */

        default:                return ST_KEY_UNKNOWN;
    }
}

static void handle_display_event(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 SDL_DisplayEvent *event) {
    switch (event->type) {
        case SDL_EVENT_DISPLAY_ADDED:
        case SDL_EVENT_DISPLAY_REMOVED:
            update_monitors(dpsrvconn_ctx);

            break;
        // case SDL_EVENT_DISPLAY_ORIENTATION:
        // case SDL_EVENT_DISPLAY_MOVED:
        case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
        case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED: {
            st_evmonresize_t resize_event = {
                .monitor = get_monitor_by_sdl_display_id(dpsrvconn_ctx, 
                 event->displayID),
                .new_width = event->data1,
                .new_height = event->data2,
            };

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MONITOR_RESIZE], &resize_event);

            break;
        }
        // case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
        // case SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED:
        default:
            break;
    }
}

static void handle_window_event(st_dpsrvconnctx_t *dpsrvconn_ctx, 
 SDL_WindowEvent *event) {
    st_window_t *window = get_window_by_sdl_window_id(dpsrvconn_ctx, 
     event->windowID);

    if (!window)
        return;

    switch (event->type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            window->xed = true;

            break;
        case SDL_EVENT_WINDOW_RESIZED: {
            st_evwinuvec2_t resize_event = {
                .window = window,
                .hvalue = event->data1,
                .vvalue = event->data2,
            };

            window->width = event->data1;
            window->height = event->data2;

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_RESIZE], &resize_event);

            break;
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED: {
            st_evwinnoargs_t focus_event = {
                .window = window,
            };

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_IN], &focus_event);

            break;
        }
        case SDL_EVENT_WINDOW_FOCUS_LOST: {
            st_evwinnoargs_t focus_event = {
                .window = window,
            };

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_FOCUS_OUT], &focus_event);

            break;
        }
        case SDL_EVENT_WINDOW_SHOWN: {
            st_evwinnoargs_t show_event = {
                .window = window,
            };

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_SHOW], &show_event);

            break;
        }
        case SDL_EVENT_WINDOW_HIDDEN: {
            st_evwinnoargs_t hide_event = {
                .window = window,
            };

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_WIN_HIDE], &hide_event);

            break;
        }
        case SDL_EVENT_WINDOW_MOUSE_ENTER: {
            st_evwinnoargs_t enter_event = {
                .window = window,
            };

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MOUSE_ENTER], &enter_event);

            break;
        }
        case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
            st_evwinnoargs_t leave_event = {
                .window = window,
            };

            ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
             dpsrvconn_ctx->evtypes[EV_MOUSE_LEAVE], &leave_event);

            break;
        }
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED: {
            st_monitor_t    *old_monitor = window->monitor;
            st_monitor_t    *new_monitor = get_monitor_by_sdl_display_id(
             dpsrvconn_ctx, event->data1);
            st_evwinptr_t    monitor_changed_event = {
                .window = window,
                .ptr = new_monitor,
            };

            if (old_monitor != new_monitor) {
                window->monitor = new_monitor;

                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_WIN_MONITOR_CHANGED], 
                 &monitor_changed_event);
            }

            break;
        }
        // case SDL_EVENT_WINDOW_EXPOSED:
        // case SDL_EVENT_WINDOW_MOVED:
        //     break;
        default:
            break;
    }
}

static void st_dpsrvconn_process(st_dpsrvconnctx_t *dpsrvconn_ctx) {
    SDL_Event event;

    while (dpsrvconn_ctx->sdl3.poll_event(&event)) {
        if (event.type >= SDL_EVENT_DISPLAY_FIRST 
         && event.type <= SDL_EVENT_DISPLAY_LAST)
            handle_display_event(dpsrvconn_ctx, &event.display);

        if (event.type >= SDL_EVENT_WINDOW_FIRST 
         && event.type <= SDL_EVENT_WINDOW_LAST)
            handle_window_event(dpsrvconn_ctx, &event.window);

        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                st_evwinu64_t key_event = {
                    .window = get_window_by_sdl_window_id(dpsrvconn_ctx, 
                     event.key.windowID),
                    .value = sdl_keycode_to_st_key(event.key.key),
                };
                st_evtypeid_t key_event_type = event.type == SDL_EVENT_KEY_DOWN
                    ? EV_KEY_PRESS
                    : EV_KEY_RELEASE;

                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[key_event_type], &key_event);

                break;
            }
            case SDL_EVENT_TEXT_INPUT: {
                st_evwinsymbol_t text_event = {
                    .window = get_window_by_sdl_window_id(dpsrvconn_ctx, 
                     event.text.windowID),
                    .value = "\0\0\0\0",
                };

                strncpy(text_event.value, event.text.text, 4);
                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_KEY_INPUT], &text_event);

                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                st_evwinuvec2_t mouse_event = {
                    .window = get_window_by_sdl_window_id(dpsrvconn_ctx, 
                     event.motion.windowID),
                    .hvalue = event.motion.x,
                    .vvalue = event.motion.y,
                };

                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_MOUSE_MOVE], &mouse_event);

                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                st_evwinunsigned_t mouse_event = {
                    .window = get_window_by_sdl_window_id(dpsrvconn_ctx, 
                     event.button.windowID),
                    .value = event.button.button,
                };
                st_evtypeid_t      mouse_event_type = (
                 event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                    ? EV_MOUSE_PRESS
                    : EV_MOUSE_RELEASE;
                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[mouse_event_type], &mouse_event);

                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                st_evwininteger_t wheel_event = {
                    .window = get_window_by_sdl_window_id(dpsrvconn_ctx, 
                     event.wheel.windowID),
                    .value = event.wheel.y,
                };

                ST_EVENTSCTX_CALL(dpsrvconn_ctx->events_ctx, push,
                 dpsrvconn_ctx->evtypes[EV_MOUSE_WHEEL], &wheel_event);

                break;
            }
            default:
                break;
        }
    }
}

static unsigned st_dpsrvconn_get_monitor_width(const st_monitor_t *monitor) {
    return monitor->bounds.w;
}

static unsigned st_dpsrvconn_get_monitor_height(const st_monitor_t *monitor) {
    return monitor->bounds.h;
}

static int st_dpsrvconn_get_monitor_index(const st_monitor_t *monitor) {
    st_dpsrvconnctx_t *dpsrvconn_ctx = (st_dpsrvconnctx_t *)ST_MONITOR_CALL(
     monitor, get_owner);
    
    for (int i = 0; i < dpsrvconn_ctx->monitors_count; i++) {
        if (dpsrvconn_ctx->monitors[i].handle == monitor->handle)
            return i;
    }

    return -1;
}

static const char *st_dpsrvconn_get_monitor_name(const st_monitor_t *monitor) {
    return monitor->name;
}

static bool st_dpsrvconn_is_monitor_primary(const st_monitor_t *monitor) {
    return monitor->is_primary;
}

static void *st_dpsrvconn_get_monitor_device_handle(
 const st_monitor_t *monitor) {
    return (void *)((uintptr_t)monitor->handle);
}

static void *st_dpsrvconn_get_monitor_native_device_handle(
 const st_monitor_t *monitor) {
    st_dpsrvconnctx_t *dpsrvconn_ctx = (st_dpsrvconnctx_t *)ST_MONITOR_CALL(
     monitor, get_owner);

    assert(dpsrvconn_ctx->video_driver != VD_UNKNOWN);

    return dpsrvconn_ctx->video_driver == VD_WINDOWS
        ? monitor->native_handle
        : dpsrvconn_ctx->native_display;
}

static bool st_dpsrvconn_is_window_xed(const st_window_t *window) {
    return window->xed;
}

static const st_monitor_t *st_dpsrvconn_get_window_monitor(
 const st_window_t *window) {
    return window->monitor;
}

static void *st_dpsrvconn_get_window_handle(const st_window_t *window) {
    return window->handle;
}

static void *st_dpsrvconn_get_window_native_handle(const st_window_t *window) {
    return window->native_handle;
}

static unsigned st_dpsrvconn_get_window_width(const st_window_t *window) {
    return window->width;
}

static unsigned st_dpsrvconn_get_window_height(const st_window_t *window) {
    return window->height;
}
