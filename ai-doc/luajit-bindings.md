# LuaJIT FFI биндинги

## Структура биндинг-модуля

После унификации все LuaJIT биндинги имеют стандартную структуру.

### CMakeLists.txt (2 строки!)

```cmake
include(../luajitbind.cmake)
st_build_luajitbind_module("modulename" ST_MODULE_LUAJITBIND_MODULENAME)
```

Функция `st_build_luajitbind_module` автоматически:
- Генерирует `config.h` из общего шаблона
- Генерирует `<modulename>.c` из `luajitbind.c.in`
- Встраивает `embedded.luajit` через функцию `embed()`
- Настраивает все include пути и зависимости

### embedded.luajit - LuaJIT FFI код

Типичная структура:

```lua
local ffi = require("ffi")
local Object = require("Object")
local ModCtx = require("ModCtx")
local ModsMgr = require("ModsMgr")

-- 1. C-декларации
ffi.cdef[[
    typedef struct st_xxxctx {
        st_modctx_t __st_parent;
    } st_xxxctx_t;
    
    typedef struct st_xxxctx_funcs {
        st_modctx_funcs_t __st_parent;
        st_xxx_t *(*create)(st_xxxctx_t *ctx);
    } st_xxxctx_funcs_t;
    
    typedef struct st_xxx {
        st_object_t __st_parent;
    } st_xxx_t;
    
    typedef struct st_xxx_funcs {
        st_object_funcs_t __st_parent;
        unsigned (*get_value)(const st_xxx_t *obj);
    } st_xxx_funcs_t;
]]

-- 2. Регистрация модуля контекста
package.preload["Xxx"] = function()
    local M = {
        __ctype = "st_xxxctx_t *",  -- ВАЖНО для as()
    }
    
    -- 3. Методы контекста
    M.methods = setmetatable({
        create = function(self)
            local obj_handle = ffi.cast("const st_xxxctx_funcs_t *",
                ffi.cast("st_object_t *", self.handle).funcs
            ).create(self.handle)
            if not obj_handle then
                error("Xxx: Failed to create object")
            end
            return Object.__mtnew(XxxObject.__mt, 
                ffi.cast("st_xxx_t *", obj_handle), true)
        end,
    }, { __index = ModCtx.methods })
    
    -- 4. Конструкторы
    M.new_by_name = function(name, ...)
        local modsmgr = ModsMgr.get_instance()
        local ctor = modsmgr:get_ctor("xxx", name)
        if not ctor then
            error(string.format("Xxx.new_by_name: constructor not found for '%s'", 
                name or "nil"))
        end
        local params = ModCtx.__create_modctx_params(modsmgr, ...)
        local ctx = ctor(params)
        if not ctx then
            error(string.format("Xxx.new_by_name: failed to create context '%s'", 
                name or "nil"))
        end
        return Object.__mtnew(M.__mt, ffi.cast("st_xxxctx_t *", ctx), true)
    end
    
    M.new = function(...)
        return M.new_by_name(nil, ...)
    end
    
    M.get_instance = function(module_name)
        local modsmgr = ModsMgr.get_instance()
        local modctx = modsmgr:get_singleton("xxx", module_name)
        if not modctx then
            error("Xxx.get_instance: singleton not found")
        end
        return Object.__mtnew(M.__mt, 
            ffi.cast("st_xxxctx_t *", modctx.handle), false)
    end
    
    M.__mt = {}
    M.__mt.__index = M.methods
    
    return M
end

```

**Примечание об объектах:** На данный момент у нас нет примеров биндингов для модулей, создающих объекты (например, `st_ini_t`, `st_monitor_t`). Точная структура LuaJIT биндингов для таких объектов ещё не определена и будет добавлена позже.

## Важные правила

1. **Всегда добавляй `__ctype`** — нужен для метода `as()` (приведение типов)
2. **Всегда добавляй `error()` при nil** — не возвращай просто `nil`, используй `error()` для информативных сообщений
3. **`owned=true` для конструкторов `new`/`new_by_name`** — Lua будет управлять памятью созданных объектов
4. **`owned=false` для `get_instance`** — C-код управляет памятью singleton'ов
5. **Наследуй методы через `setmetatable`** — `{ __index = Parent.methods }`
6. **Используй `ffi.cast` для всех C-указателей** — LuaJIT FFI строго типизирован

## Примеры биндингов

- **Простой модуль:** `src/modules/luajitbind/terminal/`, `src/modules/luajitbind/logger/` — контекст без создания объектов
- **Модуль с константами:** `src/modules/luajitbind/opts/` — контекст + enum константы
- **Модуль с объектами:** Пока нет примеров (будет добавлено позже)

## Добавление нового биндинга

1. Создать директорию `src/modules/luajitbind/newmodule/`
2. Создать `CMakeLists.txt`:
   ```cmake
   include(../luajitbind.cmake)
   st_build_luajitbind_module("newmodule" ST_MODULE_LUAJITBIND_NEWMODULE)
   ```
3. Создать `embedded.luajit` с LuaJIT FFI кодом
4. Добавить опцию `ST_MODULE_LUAJITBIND_NEWMODULE` в `cmake/options.cmake`

