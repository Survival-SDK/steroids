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

-- modsmgr:get_module_names(dst: table, mods_count: integer, 
--  modname_size: integer, subsystem: string)
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
Angle = require "Angle"

Angle.new_ctx(logger_ctx: logger_ctx): angle_ctx

angle_ctx:destroy()
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
angle_ctx:dgacos(cos : double): double
```

## atlas
```luau
Atlas = require "Atlas"

Atlas.new_ctx(logger_ctx: logger_ctx): atlas_ctx

atlas_ctx:destroy()
```

## bitmap
```luau
Bitmap = require "Bitmap"

Bitmap.new_ctx(logger_ctx: logger_ctx): bitmap_ctx

bitmap_ctx:destroy()
```

## drawq
```luau
Drawq = require "Drawq"

Drawq.new_ctx(dynarr_ctx: dynarr_ctx, logger_ctx: logger_ctx, sprite_ctx: sprite_ctx): drawq_ctx

drawq_ctx:destroy()
```

## dynarr
```luau
Dynarr = require "Dynarr"

Dynarr.new_ctx(logger_ctx: logger_ctx): dynarr_ctx

dynarr_ctx:destroy()
```

## events
```luau
Events = require "Events"

Events.new_ctx(logger_ctx: logger_ctx): events_ctx

events_ctx:destroy()
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
GfxCtx = require "GfxCtx"

GfxCtx.new_ctx(logger_ctx: logger_ctx, monitor_ctx: monitor_ctx): gfxctx_ctx
GfxCtx.gapi_gl11: integer
GfxCtx.gapi_gl12: integer
GfxCtx.gapi_gl13: integer
GfxCtx.gapi_gl14: integer
GfxCtx.gapi_gl15: integer
GfxCtx.gapi_gl2: integer
GfxCtx.gapi_gl21: integer
GfxCtx.gapi_gl3: integer
GfxCtx.gapi_gl31: integer
GfxCtx.gapi_gl32: integer
GfxCtx.gapi_gl33: integer
GfxCtx.gapi_gl4: integer
GfxCtx.gapi_gl41: integer
GfxCtx.gapi_gl42: integer
GfxCtx.gapi_gl43: integer
GfxCtx.gapi_gl44: integer
GfxCtx.gapi_gl45: integer
GfxCtx.gapi_gl46: integer
GfxCtx.gapi_es1: integer
GfxCtx.gapi_es11: integer
GfxCtx.gapi_es2: integer
GfxCtx.gapi_es3: integer
GfxCtx.gapi_es31: integer
GfxCtx.gapi_es32: integer

gfxctx_ctx:destroy()
gfxctx_ctx:create(monitor: monitor, window: window, api: integer): gfxctx
gfxctx_ctx:create_shared(monitor: monitor, window: window, gfxctx: gfxctx): gfxctx

gfxctx:make_current(): bool
gfxctx:swap_buffers(): bool
gfxctx:get_api(): integer
gfxctx:destroy()
```

## keyboard
```luau
Keyboard = require "Keyboard"

Keyboard.new_ctx(events_ctx: events_ctx, logger_ctx: logger_ctx): keyboard_ctx

Keyboard.key_space: integer
Keyboard.key_exclam: integer
Keyboard.key_quotedbl: integer
Keyboard.key_numbersign: integer
Keyboard.key_dollar: integer
Keyboard.key_percent: integer
Keyboard.key_ampersand: integer
Keyboard.key_apostrophe: integer
Keyboard.key_quoteright: integer
Keyboard.key_parenleft: integer
Keyboard.key_parenright: integer
Keyboard.key_asterisk: integer
Keyboard.key_plus: integer
Keyboard.key_comma: integer
Keyboard.key_minus: integer
Keyboard.key_period: integer
Keyboard.key_slash: integer
Keyboard.key_0: integer
Keyboard.key_1: integer
Keyboard.key_2: integer
Keyboard.key_3: integer
Keyboard.key_4: integer
Keyboard.key_5: integer
Keyboard.key_6: integer
Keyboard.key_7: integer
Keyboard.key_8: integer
Keyboard.key_9: integer
Keyboard.key_colon: integer
Keyboard.key_semicolon: integer
Keyboard.key_less: integer
Keyboard.key_equal: integer
Keyboard.key_greater: integer
Keyboard.key_question: integer
Keyboard.key_at: integer
Keyboard.key_a: integer
Keyboard.key_b: integer
Keyboard.key_c: integer
Keyboard.key_d: integer
Keyboard.key_e: integer
Keyboard.key_f: integer
Keyboard.key_g: integer
Keyboard.key_h: integer
Keyboard.key_i: integer
Keyboard.key_j: integer
Keyboard.key_k: integer
Keyboard.key_l: integer
Keyboard.key_m: integer
Keyboard.key_n: integer
Keyboard.key_o: integer
Keyboard.key_p: integer
Keyboard.key_q: integer
Keyboard.key_r: integer
Keyboard.key_s: integer
Keyboard.key_t: integer
Keyboard.key_u: integer
Keyboard.key_v: integer
Keyboard.key_w: integer
Keyboard.key_x: integer
Keyboard.key_y: integer
Keyboard.key_z: integer
Keyboard.key_bracketleft: integer
Keyboard.key_backslash: integer
Keyboard.key_bracketright: integer
Keyboard.key_asciicircum: integer
Keyboard.key_underscore: integer
Keyboard.key_grave: integer
Keyboard.key_quoteleft: integer
Keyboard.key_braceleft: integer
Keyboard.key_bar: integer
Keyboard.key_braceright: integer
Keyboard.key_asciitilde: integer

Keyboard.key_backspace: integer
Keyboard.key_tab: integer
Keyboard.key_linefeed: integer
Keyboard.key_clear: integer
Keyboard.key_return: integer
Keyboard.key_pause: integer
Keyboard.key_scroll_lock: integer
Keyboard.key_sys_req: integer
Keyboard.key_escape: integer
Keyboard.key_delete: integer

Keyboard.key_home: integer
Keyboard.key_left: integer
Keyboard.key_up: integer
Keyboard.key_right: integer
Keyboard.key_down: integer
Keyboard.key_prior: integer
Keyboard.key_page_up: integer
Keyboard.key_next: integer
Keyboard.key_page_down: integer
Keyboard.key_end: integer
Keyboard.key_begin: integer

Keyboard.key_select: integer
Keyboard.key_print: integer
Keyboard.key_execute: integer
Keyboard.key_insert: integer
Keyboard.key_undo: integer
Keyboard.key_redo: integer
Keyboard.key_menu: integer
Keyboard.key_find: integer
Keyboard.key_cancel: integer
Keyboard.key_help: integer
Keyboard.key_break: integer
Keyboard.key_mode_switch: integer
Keyboard.key_script_switch: integer
Keyboard.key_num_lock: integer

Keyboard.key_kp_space: integer
Keyboard.key_kp_tab: integer
Keyboard.key_kp_enter: integer
Keyboard.key_kp_f1: integer
Keyboard.key_kp_f2: integer
Keyboard.key_kp_f3: integer
Keyboard.key_kp_f4: integer
Keyboard.key_kp_home: integer
Keyboard.key_kp_left: integer
Keyboard.key_kp_up: integer
Keyboard.key_kp_right: integer
Keyboard.key_kp_down: integer
Keyboard.key_kp_prior: integer
Keyboard.key_kp_page_up: integer
Keyboard.key_kp_next: integer
Keyboard.key_kp_page_down: integer
Keyboard.key_kp_end: integer
Keyboard.key_kp_begin: integer
Keyboard.key_kp_insert: integer
Keyboard.key_kp_delete: integer
Keyboard.key_kp_equal: integer
Keyboard.key_kp_multiply: integer
Keyboard.key_kp_add: integer
Keyboard.key_kp_separator: integer
Keyboard.key_kp_subtract: integer
Keyboard.key_kp_decimal: integer
Keyboard.key_kp_divide: integer

Keyboard.key_kp_0: integer
Keyboard.key_kp_1: integer
Keyboard.key_kp_2: integer
Keyboard.key_kp_3: integer
Keyboard.key_kp_4: integer
Keyboard.key_kp_5: integer
Keyboard.key_kp_6: integer
Keyboard.key_kp_7: integer
Keyboard.key_kp_8: integer
Keyboard.key_kp_9: integer

Keyboard.key_f1: integer
Keyboard.key_f2: integer
Keyboard.key_f3: integer
Keyboard.key_f4: integer
Keyboard.key_f5: integer
Keyboard.key_f6: integer
Keyboard.key_f7: integer
Keyboard.key_f8: integer
Keyboard.key_f9: integer
Keyboard.key_f10: integer
Keyboard.key_f11: integer
Keyboard.key_l1: integer
Keyboard.key_f12: integer
Keyboard.key_l2: integer
Keyboard.key_f13: integer
Keyboard.key_l3: integer
Keyboard.key_f14: integer
Keyboard.key_l4: integer
Keyboard.key_f15: integer
Keyboard.key_l5: integer
Keyboard.key_f16: integer
Keyboard.key_l6: integer
Keyboard.key_f17: integer
Keyboard.key_l7: integer
Keyboard.key_f18: integer
Keyboard.key_l8: integer
Keyboard.key_f19: integer
Keyboard.key_l9: integer
Keyboard.key_f20: integer
Keyboard.key_l10: integer
Keyboard.key_f21: integer
Keyboard.key_r1: integer
Keyboard.key_f22: integer
Keyboard.key_r2: integer
Keyboard.key_f23: integer
Keyboard.key_r3: integer
Keyboard.key_f24: integer
Keyboard.key_r4: integer
Keyboard.key_f25: integer
Keyboard.key_r5: integer
Keyboard.key_f26: integer
Keyboard.key_r6: integer
Keyboard.key_f27: integer
Keyboard.key_r7: integer
Keyboard.key_f28: integer
Keyboard.key_r8: integer
Keyboard.key_f29: integer
Keyboard.key_r9: integer
Keyboard.key_f30: integer
Keyboard.key_r10: integer
Keyboard.key_f31: integer
Keyboard.key_r11: integer
Keyboard.key_f32: integer
Keyboard.key_r12: integer
Keyboard.key_f33: integer
Keyboard.key_r13: integer
Keyboard.key_f34: integer
Keyboard.key_r14: integer
Keyboard.key_f35: integer
Keyboard.key_r15: integer

Keyboard.key_shift_l: integer
Keyboard.key_shift_r: integer
Keyboard.key_control_l: integer
Keyboard.key_control_r: integer
Keyboard.key_caps_lock: integer
Keyboard.key_shift_lock: integer

Keyboard.key_meta_l: integer
Keyboard.key_meta_r: integer
Keyboard.key_alt_l: integer
Keyboard.key_alt_r: integer
Keyboard.key_super_l: integer
Keyboard.key_super_r: integer
Keyboard.key_hyper_l: integer
Keyboard.key_hyper_r: integer

keyboard_ctx:destroy()
keyboard_ctx:process()
keyboard_ctx:press(key: integer): bool
keyboard_ctx:release(key: integer): bool
keyboard_ctx:pressed(key: integer): bool
keyboard_ctx:get_input(): ?string
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
-- logger_ctx:set_callback(callback: function, userdata: ?any, 
--  levels: integer): bool
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

## matrix3x3
```luau
Matrix3x3 = require "Matrix3x3"

Matrix3x3.new_ctx(logger_ctx: logger_ctx, angle_ctx: angle_ctx): matrix3x3_ctx

matrix3x3_ctx:destroy()
matrix3x3_ctx:custom(r1c1: double, r1c2: double, r1c3: double, r2c1: double, r2c2: double, r2c3: double): matrix3x3
matrix3x3_ctx:identity(): matrix3x3
matrix3x3_ctx:translation(x: double, y: double): matrix3x3
matrix3x3_ctx:scaling(hscale: double, vscale: double): matrix3x3
matrix3x3_ctx:rrotation(radians: double): matrix3x3
matrix3x3_ctx:drotation(degrees: double): matrix3x3
matrix3x3_ctx:rhshearing(radians: double): matrix3x3
matrix3x3_ctx:dhshearing(degrees: double): matrix3x3
matrix3x3_ctx:rvshearing(radians: double): matrix3x3
matrix3x3_ctx:dvshearing(degrees: double): matrix3x3

matrix3x3:clone(): matrix3x3
matrix3x3:apply(other: matrix): matrix3x3
matrix3x3:translate(x: double, y: double): matrix3x3
matrix3x3:scale(hscale: double, vscale: double): matrix3x3
matrix3x3:rrotate(radians: double): matrix3x3
matrix3x3:drotate(degrees: double): matrix3x3
matrix3x3:rhshear(radians: double): matrix3x3
matrix3x3:dhshear(degrees: double): matrix3x3
matrix3x3:rvshear(radians: double): matrix3x3
matrix3x3:dvshear(degrees: double): matrix3x3
matrix3x3:get_data(): double, double, double, double, double, double
```

## monitor
```luau
MonitorCtx = require "MonitorCtx"

MonitorCtx.new(...): monitor_ctx
MonitorCtx.new_by_name(module_name: string, ...): monitor_ctx
MonitorCtx.get_instance(module_name: ?string): monitor_ctx

monitor_ctx:get_monitors_count(): integer
monitor_ctx:open(index: integer): monitor
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
-- inherited from object
monitor:destroy()
monitor:get_owner(): object
monitor:get_owner_unsafe(): object
```

## mouse
```luau
Mouse = require "Mouse"

Mouse.new_ctx(events_ctx: events_ctx, logger_ctx: logger_ctx): mouse_ctx
Mouse.mb_left: integer
Mouse.mb_middle: integer
Mouse.mb_right: integer

mouse_ctx:destroy()
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
Sprite = require "Sprite"

Sprite.new_ctx(logger_ctx: logger_ctx): sprite_ctx

sprite_ctx:destroy()
sprite_ctx:from_texture(texture: texture): sprite
sprite:destroy()
sprite:get_width(): integer
sprite:get_height(): integer
```

## timer
```luau
Timer = require "Timer"

Timer.new_ctx(logger_ctx: logger_ctx): timer_ctx

timer_ctx:destroy()
timer_ctx:start(): integer
timer_ctx:get_elapsed(start: integer): integer
timer_ctx:sleep(ms: integer)
timer_ctx:sleep_for_fps(fps: integer)
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
Texture = require "Texture"

Texture.new_ctx(bitmap_ctx: bitmap_ctx, logger_ctx: logger_ctx, gfxctx: gfxctx): texture_ctx

texture_ctx:destroy()
texture_ctx:load(filename: string): texture
texture:destroy()
texture:get_width(): integer
texture:get_height(): integer
```

## vec2
```luau
Vec2 = require "Vec2"

Vec2.new_ctx(logger_ctx: logger_ctx, angle_ctx: angle_ctx): vec2_ctx

vec2_ctx:destroy()
vec2_ctx:sum(first_x: double, first_y: double, second_x: double, second_y: double): double, double
vec2_ctx:diff(first_x: double, first_y: double, second_x: double, second_y: double): double, double
vec2_ctx:product(first_x: double, first_y: double, second_x: double, second_y: double): double, double
vec2_ctx:len(x: double, y: double): double
vec2_ctx:distance(first_x: double, first_y: double, second_x: double, second_y: double): double
vec2_ctx:unit(x: double, y: double): double, double
vec2_ctx:dot_product(first_x: double, first_y: double, second_x: double, second_y: double): double
vec2_ctx:rangle(first_x: double, first_y: double, second_x: double, second_y: double): double
vec2_ctx:dangle(first_x: double, first_y: double, second_x: double, second_y: double): double
vec2_ctx:rrotation(x: double, y: double, radians: double): double, double
vec2_ctx:drotation(x: double, y: double, degrees: double): double, double
vec2_ctx:rotation90(x: double, y: double): double, double
vec2_ctx:rotation180(x: double, y: double): double, double
vec2_ctx:rotation270(x: double, y: double): double, double
vec2_ctx:applying_matrix3x3(x: double, y: double, matrix: matrix3x3): double, double
vec2_ctx:default_basis_xvec(): double, double
vec2_ctx:default_basis_yvec(): double, double
```

## window
```luau
Window = require "Window"

Window.new_ctx(events_ctx: events_ctx, logger_ctx: logger_ctx, monitor_ctx: monitor_ctx): window_ctx

window_ctx:destroy()
window_ctx:create(monitor_ctx: monitor_ctx, x: integer, y: integer, width: integer, height: integer, fullscreen: bool, title: string): window
window_ctx:process()
window:destroy()
window:xed(): bool
window:get_width(): integer
window:get_height(): integer
```
