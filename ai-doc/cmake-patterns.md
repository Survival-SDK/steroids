# CMake паттерны в проекте

## Стандартный CMakeLists.txt модуля

### Минимальный пример (без third-party зависимостей)

Модули без third-party зависимостей обычно имеют имя `xxx_simple`:

```cmake
if (${ST_MODULE_XXX_SIMPLE} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "simple")
set(ST_MODULE_TYPE ${ST_MODULE_XXX_SIMPLE})
set(ST_MODULE_SUBSYSTEM "xxx")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

configure_file(
    "config.h.in"
    "config.h"
)

bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
bb_add_more_warnings(
    LANG C
    CATEGORIES basic array asciiz format preprocessor
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_XXX_SIMPLE})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

target_compile_options(${ST_MODULE_TARGET} PRIVATE 
    ${C_COMPILE_OPTIONS} -fms-extensions)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "simple.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

# Минимальный набор include путей
target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)
```

**Пример:** `src/modules/fs/simple/CMakeLists.txt`

### Расширенный пример (с third-party зависимостями)

```cmake
if (${ST_MODULE_XXX_YYY} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "yyy")
set(ST_MODULE_TYPE ${ST_MODULE_XXX_YYY})
set(ST_MODULE_SUBSYSTEM "xxx")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

configure_file(
    "config.h.in"
    "config.h"
)

bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
bb_add_more_warnings(
    LANG C
    CATEGORIES basic alloc array asciiz format preprocessor
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_XXX_YYY})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

# Поиск third-party библиотек (см. раздел "Поиск библиотек" ниже)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED luajit)

target_compile_options(${ST_MODULE_TARGET} PRIVATE 
    ${C_COMPILE_OPTIONS} -fms-extensions)
target_sources(${ST_MODULE_TARGET} PRIVATE
    "yyy.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

# Расширенный набор include путей
target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"      # Публичные заголовки движка
    "${CMAKE_SOURCE_DIR}/src"          # Внутренние заголовки (например, dlist.h)
    ${CMAKE_CURRENT_BINARY_DIR}        # Сгенерированные файлы (config.h)
    ${LUAJIT_INCLUDE_DIRS}             # Заголовки third-party библиотек
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${LUAJIT_STATIC_LDFLAGS}
)
```

**Примеры:** `src/modules/luajit/luajit/CMakeLists.txt`, `src/modules/ini/inih/CMakeLists.txt`

### Примечания по target_include_directories

- **`"${CMAKE_SOURCE_DIR}/include"`** — всегда требуется для публичных заголовков
- **`"${CMAKE_SOURCE_DIR}/src"`** — добавляется, если нужен доступ к внутренним заголовкам (например, `#include "dlist.h"`)
- **`${CMAKE_CURRENT_BINARY_DIR}`** — всегда требуется для сгенерированных файлов (`config.h`)
- **Пути к third-party библиотекам** — добавляются в зависимости от используемых библиотек

## Поиск библиотек

### Системные библиотеки через find_package

```cmake
find_package(X11 REQUIRED)
target_link_libraries(${ST_MODULE_TARGET} PRIVATE ${X11_LIBRARIES})
```

### Библиотеки через pkg-config

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED luajit)

target_include_directories(${ST_MODULE_TARGET} PRIVATE ${LUAJIT_INCLUDE_DIRS})
target_link_libraries(${ST_MODULE_TARGET} PRIVATE ${LUAJIT_STATIC_LDFLAGS})
```

### Библиотеки через bb_find_library

```cmake
bb_find_library(
    VAR_PREFIX INIH
    NAME inih
    FILENAMES inih
    COMPILER_OPTIONS -linih
    REQUIRED
)
target_link_libraries(${ST_MODULE_TARGET} PRIVATE ${INIH_LIBRARY})
```

## Обязательные элементы

- **`-fms-extensions`** — требуется для безымянных структур (наследование)

## Стандартный config.h.in

```c
#pragma once

#define ST_MODULE_TYPE_@ST_MODULE_TYPE@
#define ST_MODULE_TYPE "@ST_MODULE_TYPE@"
```

## Embedding файлов в C-код

Функция `embed()` определена в `cmake/embed.cmake`. На данный момент используется для встраивания LuaJIT FFI кода:

```cmake
embed(${ST_MODULE_TARGET} "embedded.luajit" "embedded_luajit.h" 
    "EMBEDDED_LUAJIT")
```

Это создаст `embedded_luajit.h` с макросом:
```c
#pragma once
#define EMBEDDED_LUAJIT R""""(
// содержимое embedded.luajit
)"""";
```

## LuaJIT биндинг модули

Используют унифицированную функцию из `src/modules/luajitbind/luajitbind.cmake`:

```cmake
include(../luajitbind.cmake)
st_build_luajitbind_module("modulename" ST_MODULE_LUAJITBIND_MODULENAME)
```

Эта функция автоматически выполняет все необходимые шаги:
- Генерирует `config.h`
- Генерирует `<modulename>.c` из шаблона `luajitbind.c.in`
- Вызывает `embed()` для встраивания `embedded.luajit`
- Настраивает все include пути
- Линкует LuaJIT

**Результат:** CMakeLists.txt биндинга содержит всего 2 строки!
