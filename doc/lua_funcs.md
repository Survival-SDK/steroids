## Object
```luau
Object = require "Object"

object:destroy()
object:get_owner(): object
object:get_owner_unsafe(): object
```

## ModCtx
```luau
ModCtx = require "ModCtx"

modctx:get_subsystem(): string
modctx:get_name(): string
modctx:as(target: string | table): any
-- inherited from object
modctx:destroy()
modctx:get_owner(): object
modctx:get_owner_unsafe(): object
```

## ModsMgr
```luau
ModsMgr = require "ModsMgr"

ModsMgr.get_instance(): modsmgr

<!-- modsmgr:get_module_names(dst: table, mods_count: integer, 
 modname_size: integer, subsystem: string) -->
modsmgr:get_ctor(subsystem: string, module_name: string): cfunction
modsmgr:create_singleton(subsystem: string, module_name: string, 
 params: params): modctx
modsmgr:have_singleton(subsystem: string, module_name: string): bool
modsmgr:get_singleton(subsystem: string, module_name: string): modctx
-- inherited from object
modsmgr:destroy()
modsmgr:get_owner(): object
modsmgr:get_owner_unsafe(): object
```

## angle
```luau
AngleCtx = require "AngleCtx"

AngleCtx.new(...): angle_ctx
AngleCtx.new_by_name(module_name: string, ...): angle_ctx
AngleCtx.get_instance(module_name: ?string): angle_ctx

angle_ctx:rtod(radians: double): double
angle_ctx:dtor(degrees: double): double
angle_ctx:rnormalized360(radians: double): double
angle_ctx:dnormalized360(degrees: double): double
angle_ctx:rdsin(radians: double): double
angle_ctx:dgsin(degrees: double): double
angle_ctx:rdcos(radians: double): double
angle_ctx:dgcos(degrees: double): double
angle_ctx:rdtan(radians: double): double
angle_ctx:dgtan(degrees: double): double
angle_ctx:rdacos(cos: double): double
angle_ctx:dgacos(cos: double): double
-- inherited from modctx
angle_ctx:get_subsystem(): string
angle_ctx:get_name(): string
angle_ctx:as(target: string | table): any
-- inherited from object
angle_ctx:destroy()
angle_ctx:get_owner(): object
angle_ctx:get_owner_unsafe(): object
```

## dpsrvconn
```luau
DpsrvConnCtx = require "DpsrvConnCtx"

DpsrvConnCtx.new(...): dpsrvconn_ctx
DpsrvConnCtx.new_by_name(module_name: string, ...): dpsrvconn_ctx
DpsrvConnCtx.get_instance(module_name: ?string): dpsrvconn_ctx

dpsrvconn_ctx:get_monitors_count(): integer
dpsrvconn_ctx:get_primary_monitor_index(): integer
dpsrvconn_ctx:get_monitor_by_index(index: integer): monitor
dpsrvconn_ctx:get_monitor_by_id(id: integer): monitor
dpsrvconn_ctx:get_primary_monitor(): monitor | nil
dpsrvconn_ctx:open_window(monitor: monitor, x: integer, y: integer, 
 width: integer, height: integer, fullscreen: bool, title: string): window
dpsrvconn_ctx:process()
-- inherited from modctx
dpsrvconn_ctx:get_subsystem(): string
dpsrvconn_ctx:get_name(): string
dpsrvconn_ctx:as(target: string | table): any
-- inherited from object
dpsrvconn_ctx:destroy()
dpsrvconn_ctx:get_owner(): object
dpsrvconn_ctx:get_owner_unsafe(): object

monitor:get_width(): integer
monitor:get_height(): integer
monitor:get_index(): integer
monitor:get_name(): string | nil
monitor:is_primary(): bool
monitor:get_device_handle(): userdata
-- inherited from object
monitor:destroy()
monitor:get_owner(): object
monitor:get_owner_unsafe(): object

window:xed(): bool
window:get_monitor(): monitor | nil
window:get_width(): integer
window:get_height(): integer
-- inherited from object
window:destroy()
window:get_owner(): object
window:get_owner_unsafe(): object
```

## fs
```luau
Fs = require "Fs"

Fs.new_ctx(logger_ctx: logger_ctx, pathtools_ctx: pathtools_ctx): fs_ctx
Fs.ft_unknown: integer
Fs.ft_reg: integer
Fs.ft_dir: integer
Fs.ft_chr: integer
Fs.ft_blk: integer
Fs.ft_fifo: integer
Fs.ft_link: integer
Fs.ft_sock: integer

fs_ctx:destroy()
fs_ctx:get_file_type(filename: string): integer
fs_ctx:mkdir(dirname: string): bool
```

## gfxctx
```luau
GfxCtxCtx = require "GfxCtxCtx"

GfxCtxCtx.gapi_gl1: integer
GfxCtxCtx.gapi_gl11: integer
GfxCtxCtx.gapi_gl12: integer
GfxCtxCtx.gapi_gl13: integer
GfxCtxCtx.gapi_gl14: integer
GfxCtxCtx.gapi_gl15: integer
GfxCtxCtx.gapi_gl2: integer
GfxCtxCtx.gapi_gl21: integer
GfxCtxCtx.gapi_gl3: integer
GfxCtxCtx.gapi_gl31: integer
GfxCtxCtx.gapi_gl32: integer
GfxCtxCtx.gapi_gl33: integer
GfxCtxCtx.gapi_gl4: integer
GfxCtxCtx.gapi_gl41: integer
GfxCtxCtx.gapi_gl42: integer
GfxCtxCtx.gapi_gl43: integer
GfxCtxCtx.gapi_gl44: integer
GfxCtxCtx.gapi_gl45: integer
GfxCtxCtx.gapi_gl46: integer
GfxCtxCtx.gapi_es1: integer
GfxCtxCtx.gapi_es11: integer
GfxCtxCtx.gapi_es2: integer
GfxCtxCtx.gapi_es3: integer
GfxCtxCtx.gapi_es31: integer
GfxCtxCtx.gapi_es32: integer

GfxCtxCtx.new(...): gfxctx_ctx
GfxCtxCtx.new_by_name(module_name: string, ...): gfxctx_ctx
GfxCtxCtx.get_instance(module_name: ?string): gfxctx_ctx

gfxctx_ctx:create(monitor: monitor, window: window, api: integer): gfxctx
gfxctx_ctx:create_shared(monitor: monitor, window: window, other: gfxctx): 
 gfxctx
-- inherited from modctx
gfxctx_ctx:get_subsystem(): string
gfxctx_ctx:get_name(): string
gfxctx_ctx:as(target: string | table): any
-- inherited from object
gfxctx_ctx:destroy()
gfxctx_ctx:get_owner(): object
gfxctx_ctx:get_owner_unsafe(): object

gfxctx:make_current(): bool
gfxctx:swap_buffers(): bool
gfxctx:get_window(): window
gfxctx:get_api(): integer
-- inherited from object
gfxctx:destroy()
gfxctx:get_owner(): object
gfxctx:get_owner_unsafe(): object
```

## keyboard
```luau
KeyboardCtx = require "KeyboardCtx"

KeyboardCtx.new(...): keyboard_ctx
KeyboardCtx.new_by_name(module_name: string, ...): keyboard_ctx
KeyboardCtx.get_instance(module_name: ?string): keyboard_ctx

KeyboardCtx.key_unknown: integer
KeyboardCtx.key_space: integer
KeyboardCtx.key_0: integer
KeyboardCtx.key_1: integer
KeyboardCtx.key_2: integer
KeyboardCtx.key_3: integer
KeyboardCtx.key_4: integer
KeyboardCtx.key_5: integer
KeyboardCtx.key_6: integer
KeyboardCtx.key_7: integer
KeyboardCtx.key_8: integer
KeyboardCtx.key_9: integer
KeyboardCtx.key_a: integer
KeyboardCtx.key_b: integer
KeyboardCtx.key_c: integer
KeyboardCtx.key_d: integer
KeyboardCtx.key_e: integer
KeyboardCtx.key_f: integer
KeyboardCtx.key_g: integer
KeyboardCtx.key_h: integer
KeyboardCtx.key_i: integer
KeyboardCtx.key_j: integer
KeyboardCtx.key_k: integer
KeyboardCtx.key_l: integer
KeyboardCtx.key_m: integer
KeyboardCtx.key_n: integer
KeyboardCtx.key_o: integer
KeyboardCtx.key_p: integer
KeyboardCtx.key_q: integer
KeyboardCtx.key_r: integer
KeyboardCtx.key_s: integer
KeyboardCtx.key_t: integer
KeyboardCtx.key_u: integer
KeyboardCtx.key_v: integer
KeyboardCtx.key_w: integer
KeyboardCtx.key_x: integer
KeyboardCtx.key_y: integer
KeyboardCtx.key_z: integer
KeyboardCtx.key_backspace: integer
KeyboardCtx.key_tab: integer
KeyboardCtx.key_return: integer
KeyboardCtx.key_pause: integer
KeyboardCtx.key_escape: integer
KeyboardCtx.key_delete: integer
KeyboardCtx.key_home: integer
KeyboardCtx.key_left: integer
KeyboardCtx.key_up: integer
KeyboardCtx.key_right: integer
KeyboardCtx.key_down: integer
KeyboardCtx.key_page_up: integer
KeyboardCtx.key_page_down: integer
KeyboardCtx.key_end: integer
KeyboardCtx.key_insert: integer
KeyboardCtx.key_f1: integer
KeyboardCtx.key_f2: integer
KeyboardCtx.key_f3: integer
KeyboardCtx.key_f4: integer
KeyboardCtx.key_f5: integer
KeyboardCtx.key_f6: integer
KeyboardCtx.key_f7: integer
KeyboardCtx.key_f8: integer
KeyboardCtx.key_f9: integer
KeyboardCtx.key_f10: integer
KeyboardCtx.key_f11: integer
KeyboardCtx.key_f12: integer
KeyboardCtx.key_shift_l: integer
KeyboardCtx.key_shift_r: integer
KeyboardCtx.key_control_l: integer
KeyboardCtx.key_control_r: integer
KeyboardCtx.key_alt_l: integer
KeyboardCtx.key_alt_r: integer
KeyboardCtx.key_super_l: integer
KeyboardCtx.key_super_r: integer

keyboard_ctx:process()
keyboard_ctx:press(key: integer): bool
keyboard_ctx:release(key: integer): bool
keyboard_ctx:pressed(key: integer): bool
keyboard_ctx:get_input(): ?string
-- inherited from modctx
keyboard_ctx:get_subsystem(): string
keyboard_ctx:get_name(): string
keyboard_ctx:as(target: string | table): any
-- inherited from object
keyboard_ctx:destroy()
keyboard_ctx:get_owner(): object
keyboard_ctx:get_owner_unsafe(): object
```

## logger
```luau
LoggerCtx = require "LoggerCtx"

LoggerCtx.ll_none: integer
LoggerCtx.ll_error: integer
LoggerCtx.ll_warning: integer
LoggerCtx.ll_info: integer
LoggerCtx.ll_debug: integer
LoggerCtx.ll_all: integer

LoggerCtx.new(...): logger_ctx
LoggerCtx.new_by_name(module_name: string, ...): logger_ctx
LoggerCtx.get_instance(module_name: ?string): logger_ctx

logger_ctx:enable_events(): bool
logger_ctx:set_stdout_levels(levels: integer): bool
logger_ctx:set_stderr_levels(levels: integer): bool
logger_ctx:set_log_file(filename: string, levels: integer): bool
<!-- logger_ctx:set_callback(callback: function, userdata: ?any, 
 levels: integer): bool -->
logger_ctx:debug(format: string, ...)
logger_ctx:info(format: string, ...)
logger_ctx:warning(format: string, ...)
logger_ctx:error(format: string, ...)
logger_ctx:set_postmortem_msg(msg: string)
-- inherited from modctx
logger_ctx:get_subsystem(): string
logger_ctx:get_name(): string
logger_ctx:as(target: string | table): any
-- inherited from object
logger_ctx:destroy()
logger_ctx:get_owner(): object
logger_ctx:get_owner_unsafe(): object
```

## monitor
```luau
MonitorCtx = require "MonitorCtx"

MonitorCtx.new(...): monitor_ctx
MonitorCtx.new_by_name(module_name: string, ...): monitor_ctx
MonitorCtx.get_instance(module_name: ?string): monitor_ctx

monitor_ctx:get_monitors_count(): integer
monitor_ctx:get_primary_index(): integer
monitor_ctx:open(index: integer): monitor
monitor_ctx:open_primary(): monitor
-- inherited from modctx
monitor_ctx:get_subsystem(): string
monitor_ctx:get_name(): string
monitor_ctx:as(target: string | table): any
-- inherited from object
monitor_ctx:destroy()
monitor_ctx:get_owner(): object
monitor_ctx:get_owner_unsafe(): object

monitor:get_width(): integer
monitor:get_height(): integer
monitor:get_index(): integer
monitor:get_name(): ?string
monitor:is_primary(): bool
-- inherited from object
monitor:destroy()
monitor:get_owner(): object
monitor:get_owner_unsafe(): object
```

## mouse
```luau
MouseCtx = require "MouseCtx"

MouseCtx.new(...): mouse_ctx
MouseCtx.new_by_name(module_name: string, ...): mouse_ctx
MouseCtx.get_instance(module_name: ?string): mouse_ctx

MouseCtx.mb_left: integer
MouseCtx.mb_middle: integer
MouseCtx.mb_right: integer

mouse_ctx:process()
mouse_ctx:press(button: integer): bool
mouse_ctx:release(button: integer): bool
mouse_ctx:pressed(button: integer): bool
mouse_ctx:get_wheel_relative(): integer
mouse_ctx:moved(): bool
mouse_ctx:entered(): bool
mouse_ctx:leaved(): bool
mouse_ctx:get_x(): integer
mouse_ctx:get_y(): integer
mouse_ctx:get_window(): ?window
-- inherited from modctx
mouse_ctx:get_subsystem(): string
mouse_ctx:get_name(): string
mouse_ctx:as(target: string | table): any
-- inherited from object
mouse_ctx:destroy()
mouse_ctx:get_owner(): object
mouse_ctx:get_owner_unsafe(): object
```

## opts
```luau
OptsCtx = require "OptsCtx"

OptsCtx.oa_no: integer
OptsCtx.oa_required: integer
OptsCtx.oa_optional: integer

OptsCtx.new(...): opts_ctx
OptsCtx.new_by_name(module_name: string, ...): opts_ctx
OptsCtx.get_instance(module_name: ?string): opts_ctx

opts_ctx:add_option(short_option: ?string, long_option: ?string, arg: integer, 
 arg_fmt: ?string, option_descr: ?string): bool
opts_ctx:get_str(opt: string): ?string
opts_ctx:get_help(columns: integer): ?string
-- inherited from modctx
opts_ctx:get_subsystem(): string
opts_ctx:get_name(): string
opts_ctx:as(target: string | table): any
-- inherited from object
opts_ctx:destroy()
opts_ctx:get_owner(): object
opts_ctx:get_owner_unsafe(): object
```

## pathtools
```luau
PathTools = require "PathTools"

PathTools.new_ctx(logger_ctx: logger_ctx): pathtools_ctx

pathtools_ctx:destroy()
pathtools_ctx:get_parent_dir(path: string): ?string
pathtools_ctx:concat(path: string, append: string): ?string
```

## render
```luau
Render = require "Render"

Render.new_ctx(angle_ctx: angle_ctx, drawq_ctx: drawq_ctx, dynarr_ctx: dynarr_ctx, logger_ctx: logger_ctx, matrix3x3_ctx: matrix3x3_ctx, sprite_ctx: sprite_ctx, texture_ctx: texture_ctx, vec2_ctx: vec2_ctx, gfxctx: gfxctx): render_ctx

render_ctx:destroy()
render_ctx:put_sprite(sprite: sprite, x: double, y: double, z: double, hscale: double, vscale: double, pivot_x: double, pivot_y: double)
render_ctx:put_sprite_rdangled(sprite: sprite, x: double, y: double, z: double, hscale: double, vscale: double, radians: double, pivot_x: double, pivot_y: double)
render_ctx:put_sprite_dgangled(sprite: sprite, x: double, y: double, z: double, hscale: double, vscale: double, degrees: double, pivot_x: double, pivot_y: double)
render_ctx:put_sprite_rhsheared(sprite: sprite, x: double, y: double, z: double, hscale: double, vscale: double, radians: double, pivot_x: double, pivot_y: double)
render_ctx:put_sprite_dhsheared(sprite: sprite, x: double, y: double, z: double, hscale: double, vscale: double, degrees: double, pivot_x: double, pivot_y: double)
render_ctx:put_sprite_rvsheared(sprite: sprite, x: double, y: double, z: double, hscale: double, vscale: double, radians: double, pivot_x: double, pivot_y: double)
render_ctx:put_sprite_dvsheared(sprite: sprite, x: double, y: double, z: double, hscale: double, vscale: double, degrees: double, pivot_x: double, pivot_y: double)
render_ctx:process()
```

## sprite
```luau
SpriteCtx = require "SpriteCtx"

SpriteCtx.new(...): sprite_ctx
SpriteCtx.new_by_name(module_name: string, ...): sprite_ctx
SpriteCtx.get_instance(module_name: ?string): sprite_ctx

sprite_ctx:from_texture(texture: texture): sprite
-- inherited from modctx
sprite_ctx:get_subsystem(): string
sprite_ctx:get_name(): string
sprite_ctx:as(target: string | table): any
-- inherited from object
sprite_ctx:destroy()
sprite_ctx:get_owner(): object
sprite_ctx:get_owner_unsafe(): object

sprite:get_texture(): texture
sprite:get_width(): integer
sprite:get_height(): integer
-- inherited from object
sprite:destroy()
sprite:get_owner(): object
sprite:get_owner_unsafe(): object
```

## timer
```luau
TimerCtx = require "TimerCtx"

TimerCtx.new(...): timer_ctx
TimerCtx.new_by_name(module_name: string, ...): timer_ctx
TimerCtx.get_instance(module_name: ?string): timer_ctx

timer_ctx:start(): integer
timer_ctx:get_elapsed(start: integer): integer
timer_ctx:sleep(ms: integer)
timer_ctx:sleep_for_fps(fps: integer)
-- inherited from modctx
timer_ctx:get_subsystem(): string
timer_ctx:get_name(): string
timer_ctx:as(target: string | table): any
-- inherited from object
timer_ctx:destroy()
timer_ctx:get_owner(): object
timer_ctx:get_owner_unsafe(): object
```

## terminal
```luau
TerminalCtx = require "TerminalCtx"

TerminalCtx.new(...): terminal_ctx
TerminalCtx.new_by_name(module_name: string, ...): terminal_ctx
TerminalCtx.get_instance(module_name: ?string): terminal_ctx

terminal_ctx:get_rows_count(): integer
terminal_ctx:get_cols_count(): integer
-- inherited from modctx
terminal_ctx:get_subsystem(): string
terminal_ctx:get_name(): string
terminal_ctx:as(target: string | table): any
-- inherited from object
terminal_ctx:destroy()
terminal_ctx:get_owner(): object
terminal_ctx:get_owner_unsafe(): object
```

## texture
```luau
TextureCtx = require "TextureCtx"

TextureCtx.new(...): texture_ctx
TextureCtx.new_by_name(module_name: string, ...): texture_ctx
TextureCtx.get_instance(module_name: ?string): texture_ctx

texture_ctx:load(filename: string): texture
<!-- texture_ctx:memload(data: cdata, size: integer): texture -->
-- inherited from modctx
texture_ctx:get_subsystem(): string
texture_ctx:get_name(): string
texture_ctx:as(target: string | table): any
-- inherited from object
texture_ctx:destroy()
texture_ctx:get_owner(): object
texture_ctx:get_owner_unsafe(): object

texture:bind(unit: integer): bool
texture:get_width(): integer
texture:get_height(): integer
-- inherited from object
texture:destroy()
texture:get_owner(): object
texture:get_owner_unsafe(): object
```

## window
```luau
WindowCtx = require "WindowCtx"

WindowCtx.new(...): window_ctx
WindowCtx.new_by_name(module_name: string, ...): window_ctx
WindowCtx.get_instance(module_name: ?string): window_ctx

window_ctx:create(monitor: monitor, x: integer, y: integer, width: integer, 
 height: integer, fullscreen: bool, title: string): window
window_ctx:process()
-- inherited from modctx
window_ctx:get_subsystem(): string
window_ctx:get_name(): string
window_ctx:as(target: string | table): any
-- inherited from object
window_ctx:destroy()
window_ctx:get_owner(): object
window_ctx:get_owner_unsafe(): object

window:xed(): bool
window:get_monitor(): monitor
window:get_width(): integer
window:get_height(): integer
-- inherited from object
window:destroy()
window:get_owner(): object
window:get_owner_unsafe(): object
```
