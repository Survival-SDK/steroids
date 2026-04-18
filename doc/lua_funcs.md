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

<!-- modsmgr:get_module_names(dst: table, mods_count: number, 
 modname_size: number, subsystem: string) -->
modsmgr:get_ctor(subsystem: string, module_name: string): cfunction
modsmgr:create_singleton(subsystem: string, module_name: string, 
 params: params): modctx
modsmgr:have_singleton(subsystem: string, module_name: string): boolean
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

angle_ctx:rtod(radians: number): number
angle_ctx:dtor(degrees: number): number
angle_ctx:rnormalized360(radians: number): number
angle_ctx:dnormalized360(degrees: number): number
angle_ctx:rdsin(radians: number): number
angle_ctx:dgsin(degrees: number): number
angle_ctx:rdcos(radians: number): number
angle_ctx:dgcos(degrees: number): number
angle_ctx:rdtan(radians: number): number
angle_ctx:dgtan(degrees: number): number
angle_ctx:rdacos(cos: number): number
angle_ctx:dgacos(cos: number): number
-- inherited from modctx
angle_ctx:get_subsystem(): string
angle_ctx:get_name(): string
angle_ctx:as(target: string | table): any
-- inherited from object
angle_ctx:destroy()
angle_ctx:get_owner(): object
angle_ctx:get_owner_unsafe(): object
```

## atiles
```luau
AtilesCtx = require "AtilesCtx"

AtilesCtx.new(...): atiles_ctx
AtilesCtx.new_by_name(module_name: string, ...): atiles_ctx
AtilesCtx.get_instance(module_name: ?string): atiles_ctx

AtilesCtx.nm_same: number
AtilesCtx.nm_any: number

AtilesCtx.spp_nw: number
AtilesCtx.spp_ne: number
AtilesCtx.spp_se: number
AtilesCtx.spp_sw: number
AtilesCtx.spp_len: number

AtilesCtx.st_north_wall: number
AtilesCtx.st_south_wall: number
AtilesCtx.st_east_wall: number
AtilesCtx.st_west_wall: number
AtilesCtx.st_north_east_external_corner: number
AtilesCtx.st_south_east_external_corner: number
AtilesCtx.st_south_west_external_corner: number
AtilesCtx.st_north_west_external_corner: number
AtilesCtx.st_north_east_internal_corner: number
AtilesCtx.st_south_east_internal_corner: number
AtilesCtx.st_south_west_internal_corner: number
AtilesCtx.st_north_west_internal_corner: number
AtilesCtx.st_entire: number
AtilesCtx.st_empty: number
<!-- AtilesCtx.st_nonempty_len: number
AtilesCtx.st_len: number -->

atiles_ctx:tileset_load(filename: string): cdata
atiles_ctx:tileset_from_texture(texture: texture): cdata
atiles_ctx:tileset_from_atlas(atlas: atlas): cdata
atiles_ctx:add_layer(rows: number, cols: number, tile_width: number,
 tile_height: number, neighbor_mode: number): number
atiles_ctx:update_tile(layer_index: number, row: number, col: number,
 tile: cdata | nil, update_subtiles: boolean)
atiles_ctx:get_tile(layer_index: number, row: number, col: number):
 cdata | nil
atiles_ctx:get_subtile(layer_index: number, row: number, col: number):
 number, number, number, number -- nw, ne, sw, se
atiles_ctx:get_subtile_sprite(layer_index: number, row: number, col: number):
 sprite | nil, sprite | nil, sprite | nil, sprite | nil -- nw, ne, sw, se
atiles_ctx:update()
-- inherited from modctx
atiles_ctx:get_subsystem(): string
atiles_ctx:get_name(): string
atiles_ctx:as(target: string | table): any
-- inherited from object
atiles_ctx:destroy()
atiles_ctx:get_owner(): object
atiles_ctx:get_owner_unsafe(): object
```

## atlas
```luau
AtlasCtx = require "AtlasCtx"

AtlasCtx.new(...): atlas_ctx
AtlasCtx.new_by_name(module_name: string, ...): atlas_ctx
AtlasCtx.get_instance(module_name: ?string): atlas_ctx

atlas_ctx:load(filename: string): atlas
atlas_ctx:create_empty(filename: string): atlas
-- inherited from modctx
atlas_ctx:get_subsystem(): string
atlas_ctx:get_name(): string
atlas_ctx:as(target: string | table): any
-- inherited from object
atlas_ctx:destroy()
atlas_ctx:get_owner(): object
atlas_ctx:get_owner_unsafe(): object

atlas:add_subimage(name: string, x: number, y: number, width: number,
 height: number): boolean
atlas:get_filename(): string
atlas:get_subimages_count(): number
atlas:get_subimage_name(index: number): string | nil
atlas:get_subimage_x(index: number): number
atlas:get_subimage_y(index: number): number
atlas:get_subimage_width(index: number): number
atlas:get_subimage_height(index: number): number
-- inherited from object
atlas:destroy()
atlas:get_owner(): object
atlas:get_owner_unsafe(): object
```

## font
```luau
FontCtx = require "FontCtx"

FontCtx.new(...): font_ctx
FontCtx.new_by_name(module_name: string, ...): font_ctx
FontCtx.get_instance(module_name: ?string): font_ctx

font_ctx:load(filename: string): font
font_ctx:create_empty(line_height: number, base: number,
 texture_width: number, texture_height: number, pages_count: number): font
-- inherited from modctx
font_ctx:get_subsystem(): string
font_ctx:get_name(): string
font_ctx:as(target: string | table): any
-- inherited from object
font_ctx:destroy()
font_ctx:get_owner(): object
font_ctx:get_owner_unsafe(): object

font:add_page(index: number, filename: string): boolean
font:add_char(ucs4code: number, subimage_x: number, subimage_y: number,
 subimage_width: number, subimage_height: number, xoffset: number,
 yoffset: number, xadvance: number, page: number): boolean
font:get_line_height(): number
font:get_base(): number
font:get_sprite(ucs4code: number): sprite | nil
font:get_xoffset(ucs4code: number): number
font:get_yoffset(ucs4code: number): number
font:get_xadvance(ucs4code: number): number
-- inherited from object
font:destroy()
font:get_owner(): object
font:get_owner_unsafe(): object
```

## dpsrvconn
```luau
DpsrvConnCtx = require "DpsrvConnCtx"

DpsrvConnCtx.new(...): dpsrvconn_ctx
DpsrvConnCtx.new_by_name(module_name: string, ...): dpsrvconn_ctx
DpsrvConnCtx.get_instance(module_name: ?string): dpsrvconn_ctx

dpsrvconn_ctx:get_monitors_count(): number
dpsrvconn_ctx:get_primary_monitor_index(): number
dpsrvconn_ctx:get_monitor_by_index(index: number): monitor
dpsrvconn_ctx:get_monitor_by_id(id: number): monitor
dpsrvconn_ctx:get_primary_monitor(): monitor | nil
dpsrvconn_ctx:open_window(monitor: monitor, x: number, y: number, 
 width: number, height: number, fullscreen: boolean, title: string): window
dpsrvconn_ctx:process()
-- inherited from modctx
dpsrvconn_ctx:get_subsystem(): string
dpsrvconn_ctx:get_name(): string
dpsrvconn_ctx:as(target: string | table): any
-- inherited from object
dpsrvconn_ctx:destroy()
dpsrvconn_ctx:get_owner(): object
dpsrvconn_ctx:get_owner_unsafe(): object

monitor:get_width(): number
monitor:get_height(): number
monitor:get_index(): number
monitor:get_name(): string | nil
monitor:is_primary(): boolean
monitor:get_device_handle(): userdata
-- inherited from object
monitor:destroy()
monitor:get_owner(): object
monitor:get_owner_unsafe(): object

window:xed(): boolean
window:get_monitor(): monitor | nil
window:get_width(): number
window:get_height(): number
-- inherited from object
window:destroy()
window:get_owner(): object
window:get_owner_unsafe(): object
```

## gfxctx
```luau
GfxCtxCtx = require "GfxCtxCtx"

GfxCtxCtx.gapi_gl1: number
GfxCtxCtx.gapi_gl11: number
GfxCtxCtx.gapi_gl12: number
GfxCtxCtx.gapi_gl13: number
GfxCtxCtx.gapi_gl14: number
GfxCtxCtx.gapi_gl15: number
GfxCtxCtx.gapi_gl2: number
GfxCtxCtx.gapi_gl21: number
GfxCtxCtx.gapi_gl3: number
GfxCtxCtx.gapi_gl31: number
GfxCtxCtx.gapi_gl32: number
GfxCtxCtx.gapi_gl33: number
GfxCtxCtx.gapi_gl4: number
GfxCtxCtx.gapi_gl41: number
GfxCtxCtx.gapi_gl42: number
GfxCtxCtx.gapi_gl43: number
GfxCtxCtx.gapi_gl44: number
GfxCtxCtx.gapi_gl45: number
GfxCtxCtx.gapi_gl46: number
GfxCtxCtx.gapi_es1: number
GfxCtxCtx.gapi_es11: number
GfxCtxCtx.gapi_es2: number
GfxCtxCtx.gapi_es3: number
GfxCtxCtx.gapi_es31: number
GfxCtxCtx.gapi_es32: number

GfxCtxCtx.new(...): gfxctx_ctx
GfxCtxCtx.new_by_name(module_name: string, ...): gfxctx_ctx
GfxCtxCtx.get_instance(module_name: ?string): gfxctx_ctx

gfxctx_ctx:create(monitor: monitor, window: window, api: number): gfxctx
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

gfxctx:make_current(): boolean
gfxctx:swap_buffers(): boolean
gfxctx:get_window(): window
gfxctx:get_api(): number
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

KeyboardCtx.key_unknown: number
KeyboardCtx.key_space: number
KeyboardCtx.key_0: number
KeyboardCtx.key_1: number
KeyboardCtx.key_2: number
KeyboardCtx.key_3: number
KeyboardCtx.key_4: number
KeyboardCtx.key_5: number
KeyboardCtx.key_6: number
KeyboardCtx.key_7: number
KeyboardCtx.key_8: number
KeyboardCtx.key_9: number
KeyboardCtx.key_a: number
KeyboardCtx.key_b: number
KeyboardCtx.key_c: number
KeyboardCtx.key_d: number
KeyboardCtx.key_e: number
KeyboardCtx.key_f: number
KeyboardCtx.key_g: number
KeyboardCtx.key_h: number
KeyboardCtx.key_i: number
KeyboardCtx.key_j: number
KeyboardCtx.key_k: number
KeyboardCtx.key_l: number
KeyboardCtx.key_m: number
KeyboardCtx.key_n: number
KeyboardCtx.key_o: number
KeyboardCtx.key_p: number
KeyboardCtx.key_q: number
KeyboardCtx.key_r: number
KeyboardCtx.key_s: number
KeyboardCtx.key_t: number
KeyboardCtx.key_u: number
KeyboardCtx.key_v: number
KeyboardCtx.key_w: number
KeyboardCtx.key_x: number
KeyboardCtx.key_y: number
KeyboardCtx.key_z: number
KeyboardCtx.key_backspace: number
KeyboardCtx.key_tab: number
KeyboardCtx.key_return: number
KeyboardCtx.key_pause: number
KeyboardCtx.key_escape: number
KeyboardCtx.key_delete: number
KeyboardCtx.key_home: number
KeyboardCtx.key_left: number
KeyboardCtx.key_up: number
KeyboardCtx.key_right: number
KeyboardCtx.key_down: number
KeyboardCtx.key_page_up: number
KeyboardCtx.key_page_down: number
KeyboardCtx.key_end: number
KeyboardCtx.key_insert: number
KeyboardCtx.key_f1: number
KeyboardCtx.key_f2: number
KeyboardCtx.key_f3: number
KeyboardCtx.key_f4: number
KeyboardCtx.key_f5: number
KeyboardCtx.key_f6: number
KeyboardCtx.key_f7: number
KeyboardCtx.key_f8: number
KeyboardCtx.key_f9: number
KeyboardCtx.key_f10: number
KeyboardCtx.key_f11: number
KeyboardCtx.key_f12: number
KeyboardCtx.key_shift_l: number
KeyboardCtx.key_shift_r: number
KeyboardCtx.key_control_l: number
KeyboardCtx.key_control_r: number
KeyboardCtx.key_alt_l: number
KeyboardCtx.key_alt_r: number
KeyboardCtx.key_super_l: number
KeyboardCtx.key_super_r: number

keyboard_ctx:process()
keyboard_ctx:press(key: number): boolean
keyboard_ctx:release(key: number): boolean
keyboard_ctx:pressed(key: number): boolean
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

LoggerCtx.ll_none: number
LoggerCtx.ll_error: number
LoggerCtx.ll_warning: number
LoggerCtx.ll_info: number
LoggerCtx.ll_debug: number
LoggerCtx.ll_all: number

LoggerCtx.new(...): logger_ctx
LoggerCtx.new_by_name(module_name: string, ...): logger_ctx
LoggerCtx.get_instance(module_name: ?string): logger_ctx

logger_ctx:enable_events(): boolean
logger_ctx:set_stdout_levels(levels: number): boolean
logger_ctx:set_stderr_levels(levels: number): boolean
logger_ctx:set_log_file(filename: string, levels: number): boolean
<!-- logger_ctx:set_callback(callback: function, userdata: ?any, 
 levels: number): boolean -->
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

## mouse
```luau
MouseCtx = require "MouseCtx"

MouseCtx.new(...): mouse_ctx
MouseCtx.new_by_name(module_name: string, ...): mouse_ctx
MouseCtx.get_instance(module_name: ?string): mouse_ctx

MouseCtx.mb_left: number
MouseCtx.mb_middle: number
MouseCtx.mb_right: number

mouse_ctx:process()
mouse_ctx:press(button: number): boolean
mouse_ctx:release(button: number): boolean
mouse_ctx:pressed(button: number): boolean
mouse_ctx:get_wheel_relative(): number
mouse_ctx:moved(): boolean
mouse_ctx:entered(): boolean
mouse_ctx:leaved(): boolean
mouse_ctx:get_x(): number
mouse_ctx:get_y(): number
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

OptsCtx.oa_no: number
OptsCtx.oa_required: number
OptsCtx.oa_optional: number

OptsCtx.new(...): opts_ctx
OptsCtx.new_by_name(module_name: string, ...): opts_ctx
OptsCtx.get_instance(module_name: ?string): opts_ctx

opts_ctx:add_option(short_option: ?string, long_option: ?string, arg: number, 
 arg_fmt: ?string, option_descr: ?string): boolean
opts_ctx:get_str(opt: string): ?string
opts_ctx:get_help(columns: number): ?string
-- inherited from modctx
opts_ctx:get_subsystem(): string
opts_ctx:get_name(): string
opts_ctx:as(target: string | table): any
-- inherited from object
opts_ctx:destroy()
opts_ctx:get_owner(): object
opts_ctx:get_owner_unsafe(): object
```

## render
```luau
Render = require "Render"

Render.new_ctx(angle_ctx: angle_ctx, drawq_ctx: drawq_ctx, dynarr_ctx: dynarr_ctx, logger_ctx: logger_ctx, matrix3x3_ctx: matrix3x3_ctx, sprite_ctx: sprite_ctx, texture_ctx: texture_ctx, vec2_ctx: vec2_ctx, gfxctx: gfxctx): render_ctx

render_ctx:destroy()
render_ctx:put_sprite(sprite: sprite, x: number, y: number, z: number, hscale: number, vscale: number, pivot_x: number, pivot_y: number)
render_ctx:put_sprite_rdangled(sprite: sprite, x: number, y: number, z: number, hscale: number, vscale: number, radians: number, pivot_x: number, pivot_y: number)
render_ctx:put_sprite_dgangled(sprite: sprite, x: number, y: number, z: number, hscale: number, vscale: number, degrees: number, pivot_x: number, pivot_y: number)
render_ctx:put_sprite_rhsheared(sprite: sprite, x: number, y: number, z: number, hscale: number, vscale: number, radians: number, pivot_x: number, pivot_y: number)
render_ctx:put_sprite_dhsheared(sprite: sprite, x: number, y: number, z: number, hscale: number, vscale: number, degrees: number, pivot_x: number, pivot_y: number)
render_ctx:put_sprite_rvsheared(sprite: sprite, x: number, y: number, z: number, hscale: number, vscale: number, radians: number, pivot_x: number, pivot_y: number)
render_ctx:put_sprite_dvsheared(sprite: sprite, x: number, y: number, z: number, hscale: number, vscale: number, degrees: number, pivot_x: number, pivot_y: number)
render_ctx:put_text(font: font, text: string, codepoints: number, x: number, y: number, z: number, hscale: number, vscale: number, pivot_x: number, pivot_y: number)
render_ctx:process()
```

## sprite
```luau
SpriteCtx = require "SpriteCtx"

SpriteCtx.new(...): sprite_ctx
SpriteCtx.new_by_name(module_name: string, ...): sprite_ctx
SpriteCtx.get_instance(module_name: ?string): sprite_ctx

sprite_ctx:from_texture(texture: texture): sprite
sprite_ctx:from_texture_region(texture: texture, x: number, y: number,
 width: number, height: number): sprite
-- inherited from modctx
sprite_ctx:get_subsystem(): string
sprite_ctx:get_name(): string
sprite_ctx:as(target: string | table): any
-- inherited from object
sprite_ctx:destroy()
sprite_ctx:get_owner(): object
sprite_ctx:get_owner_unsafe(): object

sprite:get_texture(): texture
sprite:get_width(): number
sprite:get_height(): number
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

timer_ctx:start(): number
timer_ctx:get_elapsed(start: number): number
timer_ctx:sleep(ms: number)
timer_ctx:sleep_for_fps(fps: number)
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

terminal_ctx:get_rows_count(): number
terminal_ctx:get_cols_count(): number
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
<!-- texture_ctx:memload(data: cdata, size: number): texture -->
-- inherited from modctx
texture_ctx:get_subsystem(): string
texture_ctx:get_name(): string
texture_ctx:as(target: string | table): any
-- inherited from object
texture_ctx:destroy()
texture_ctx:get_owner(): object
texture_ctx:get_owner_unsafe(): object

texture:bind(unit: number): boolean
texture:get_width(): number
texture:get_height(): number
-- inherited from object
texture:destroy()
texture:get_owner(): object
texture:get_owner_unsafe(): object
```
