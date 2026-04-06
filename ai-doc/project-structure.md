Структура проекта:

ai-doc/ - данные по проекту для агентов Cursor

cmake/ - CMake-модули

conan/ - профили и конфигурация Conan для сборки зависимостей под разные платформы и конфигурации

cmake_build/ - директория, в которой находится собранный при помощи CMake-проект. Содержимое директории является сгенерированным

doc/ - пользовательская документация

dockerfiles/ - dockerfile'ы образов, в которых собирается проект. Dockerfile'ы поддерживаются вручную

/include/steroids - заголовочные файлы, которые используются различными модулями движка, ядром движка, а так же могут быть необходимы для разработки third-party модулей сторонними разработчиками

scripts/ - вспомогательные скрипты для сборки проекта

src/ - исходный код ядра и модулей движка (см. детализацию ниже)

/ - в корневой директории находятся Makefile, который используется, как набор шоткатов для запуска CMake, CMakeLists.txt верхнего уровня, main.luajit - тестовый скрипт для проверки работы скриптовой подсистемы движка, README.md - ридми-файл.

## Детализация src/

```
src/
├── core/              - Ядро движка (modules_manager, main)
├── modules/           - Модули движка
│   ├── <subsystem>/   - Подсистема (logger, terminal, ini, luajit и т.д.)
│   │   └── <implementation>/  - Конкретная реализация (libsir, simple, inih и т.д.)
│   └── luajitbind/    - LuaJIT FFI биндинги для модулей
└── internal headers   - Внутренние заголовки (dlist.h и другие), используемые модулями
```

**Примеры модулей:**
- `src/modules/logger/libsir/` - реализация логгера через libsir
- `src/modules/terminal/simple/` - простая реализация работы с терминалом
- `src/modules/ini/inih/` - парсер INI-файлов через libinih
- `src/modules/luajitbind/logger/` - LuaJIT биндинг для logger

## Детализация include/steroids/

```
include/steroids/
├── modules/           - Публичные интерфейсы модулей
│   ├── logger.h      - Интерфейс модуля logger
│   ├── terminal.h    - Интерфейс модуля terminal
│   ├── ini.h         - Интерфейс модуля ini
│   ├── luajit.h      - Интерфейс модуля luajit
│   └── ...           - Другие интерфейсы модулей
├── object.h          - Базовый класс st_object_t
├── modctx.h          - Базовый класс st_modctx_t для контекстов модулей
├── params.h          - Типы для передачи параметров
├── modsmgr.h         - Менеджер модулей
└── ...               - Другие публичные API движка
```

## Ключевые файлы в cmake/

- `embed.cmake` - Функция для встраивания текстовых файлов в C-код (используется для LuaJIT биндингов)
- Другие CMake-модули и функции для сборки проекта

## Структура модуля (стандартная)

Типичный модуль в `src/modules/<subsystem>/<implementation>/`:

```
<implementation>/
├── CMakeLists.txt       - Конфигурация сборки
├── config.h.in          - Шаблон для генерации config.h (обычно только #define ST_MODULE_TYPE)
├── types.h              - Внутренние типы модуля (st_xxxctx_t, вспомогательные структуры)
├── <implementation>.h   - Заголовок модуля (обычно минимален: include config.h, types.h, interface)
├── <implementation>.c   - Реализация модуля
└── iwyu.imp (опционально) - IWYU mapping для include-what-you-use
```

## Структура LuaJIT binding модуля

Модули в `src/modules/luajitbind/<module>/`:

```
<module>/
├── CMakeLists.txt       - Всего 2 строки (include и вызов st_build_luajitbind_module)
└── embedded.luajit      - LuaJIT FFI код биндинга
```

Общие файлы в `src/modules/luajitbind/`:
- `types.h` - общий для всех биндингов
- `luajitbind.h` - общий заголовок
- `luajitbind.c.in` - шаблон для генерации C-кода
- `config.h.in` - шаблон конфига
- `luajitbind.cmake` - CMake-функция для сборки биндингов

Все биндинги используют унифицированную структуру, C-код генерируется из шаблона автоматически.
