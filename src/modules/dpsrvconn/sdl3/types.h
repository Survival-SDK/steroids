#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <SDL3/SDL.h>

#include "steroids/modctx.h"
#include "steroids/modules/events.h"
#include "steroids/modules/logger.h"
#include "steroids/modules/sdl3loader.h"
#include "steroids/object.h"

#define MONITORS_MAX 8

typedef enum {
    EV_MONITOR_CONNECTED = 0,
    EV_MONITOR_DISCONNECTED,
    EV_MONITOR_REINDEX,
    EV_MONITOR_RESIZE,

    EV_MOUSE_PRESS,
    EV_MOUSE_RELEASE,
    EV_MOUSE_WHEEL,
    EV_MOUSE_MOVE,
    EV_MOUSE_ENTER,
    EV_MOUSE_LEAVE,

    EV_KEY_PRESS,
    EV_KEY_RELEASE,
    EV_KEY_INPUT,

    EV_WIN_FOCUS_IN,
    EV_WIN_FOCUS_OUT,
    EV_WIN_RESIZE,
    EV_WIN_PLACE_ON_TOP,
    EV_WIN_PLACE_ON_BOTTOM,
    EV_WIN_CREATE,
    EV_WIN_DESTROY,
    EV_WIN_SHOW,
    EV_WIN_HIDE,
    EV_WIN_MONITOR_CHANGED,

    EV_MAX,
} evtype_index_t;

typedef enum {
    VD_UNKNOWN = 0,
    VD_X11,
    VD_WAYLAND,
    VD_WINDOWS,
    VD_MAX,
} videodriver_t;

typedef struct {
    st_object_t;
    SDL_Rect      bounds;
    bool          is_primary;
    char         *name;
    SDL_DisplayID handle;
    void         *native_handle; /* Wayland output pointer or Windows HDC */
} st_monitor_t;

typedef struct {
    /* subsystem */
    bool (*init_subsystem)(SDL_InitFlags flags);
    void (*quit_subsystem)(SDL_InitFlags flags);
    SDL_InitFlags (*was_init)(SDL_InitFlags flags);

    /* events */
    bool (*poll_event)(SDL_Event *event);

    /* video */
    const char *(*get_current_video_driver)(void);

    /* display */
    SDL_DisplayID (*get_primary_display)(void);
    SDL_DisplayID *(*get_displays)(int *count);
    const char *(*get_display_name)(SDL_DisplayID displayID);
    bool (*get_display_bounds)(SDL_DisplayID displayID, SDL_Rect *rect);
    SDL_PropertiesID (*get_display_properties)(SDL_DisplayID displayID);
    
    /* window */
    SDL_Window *(*create_window)(const char *title, int w, int h, 
     unsigned flags);
    SDL_Window *(*create_window_with_properties)(SDL_PropertiesID props);
    void (*destroy_window)(SDL_Window *window);
    SDL_Window **(*get_windows)(int *count);
    SDL_PropertiesID (*get_window_properties)(SDL_Window *window);
    SDL_WindowID (*get_window_id)(SDL_Window *window);
    SDL_Window *(*get_window_from_id)(SDL_WindowID id);
    SDL_DisplayID (*get_display_for_window)(SDL_Window *window);

    /* properties */
    SDL_PropertiesID (*create_properties)(void);
    void (*destroy_properties)(SDL_PropertiesID props); 
    bool (*set_string_property)(SDL_PropertiesID props, const char *name, 
     const char *value);
    bool (*set_number_property)(SDL_PropertiesID props, const char *name, 
     int value);
    bool (*set_boolean_property)(SDL_PropertiesID props, const char *name, 
     bool value);
    bool (*set_pointer_property)(SDL_PropertiesID props, const char *name,
     void *value); 
    Sint64 (*get_number_property)(SDL_PropertiesID props, const char *name, 
     Sint64 default_value);
    void *(*get_pointer_property)(SDL_PropertiesID props, const char *name, 
     void *default_value);

    /* misc */
    bool (*set_hint)(const char *name, const char *value);
    void (*free)(void *ptr);
    const char *(*get_error)(void);
} sdl3_funcs_t;

typedef struct {
    st_modctx_t;
    st_loggerctx_t     *logger_ctx;
    st_eventsctx_t     *events_ctx;
    st_sdl3loaderctx_t *sdl3loader_ctx;
    sdl3_funcs_t        sdl3;
    bool                owns_sdl3_events_subsystem;
    bool                owns_sdl3_video_subsystem;
    videodriver_t       video_driver;
    void               *native_display; /* X11 or Wayland display pointer */
    st_monitor_t        monitors[MONITORS_MAX];
    size_t              monitors_count;
    st_evtypeid_t       evtypes[EV_MAX];
} st_dpsrvconnctx_t;

typedef struct {
    st_object_t;
    SDL_Window   *handle;
    void         *native_handle;
    unsigned      width;
    unsigned      height;
    st_monitor_t *monitor;
    bool          xed;
} st_window_t;

#define ST_DPSRVCONNCTX_T_DEFINED
#define ST_MONITOR_T_DEFINED
#define ST_WINDOW_T_DEFINED
