# Система модулей движка Steroids

## Терминология

- **Исходники модуля** — C-файлы, CMakeLists.txt, заголовочные файлы и другие файлы в `src/modules/<subsystem>/<implementation>/`
- **Модуль** — результат сборки исходников: статическая библиотека (линкуется к бинарнику движка) или динамическая библиотека (плагин)
- **Контекст модуля** — объект (`st_xxxctx_t`), который позволяет использовать функции модуля как методы. В теории можно создать несколько контекстов одного модуля в runtime (хотя обычно это не нужно).

## Структура модуля

Каждый модуль определяется через `st_moddata_t` и имеет:
- **subsystem** — подсистема (например, "logger", "terminal", "ini")
- **name** — реализация (например, "libsir", "simple", "inih")
- **prereqs** — массив зависимостей (других модулей)
- **init** — конструктор контекста (принимает `const st_param_t params[]`)

## Регистрация модуля

```c
// Пример объявления зависимостей (может быть разным для каждого модуля)
static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },      // Зависимость от любой реализации logger
    { "htable", "simple", },  // Зависимость от конкретной реализации
    {0},  // Терминатор
};

st_moddata_t *st_module_xxx_yyy_init(st_modsmgr_t *modsmgr) {
    return st_moddata_new(
        "xxx",           // subsystem
        "yyy",           // name
        ST_MODULE_TYPE,  // Тип модуля (shared/static)
        mod_prereqs,     // Зависимости
        st_xxx_init,     // Конструктор контекста
        modsmgr          // Менеджер модулей
    );
}

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr) {
    return st_module_xxx_yyy_init(modsmgr);
}
#endif
```

## Конструктор контекста модуля

```c
static st_xxxctx_t *st_xxx_init(const st_param_t params[]) {
    // 1. Получаем modsmgr
    st_modsmgr_t *modsmgr = st_modctx_get_param_as_ptr(params, "modsmgr");
    
    // 2. Получаем зависимости
    st_loggerctx_t *logger_ctx = (st_loggerctx_t *)ST_MODSMGR_CALL(modsmgr,
        get_singleton, "logger", NULL);
    
    if (!logger_ctx) {
        // Допустимо использовать fprintf или вообще ничего не выводить
        fprintf(stderr, "xxx_yyy: Unable to get logger context\n");
        return NULL;
    }
    
    // 3. Создаем контекст
    st_xxxctx_t *ctx = (st_xxxctx_t *)st_modctx_new(
        "xxx", "yyy",
        sizeof(st_xxxctx_t), 
        NULL,  // owner (пока всегда NULL)
        &xxxctx_funcs,
        (st_object_dtor_t)st_xxx_quit
    );
    
    if (!ctx) {
        ST_LOGGERCTX_CALL(logger_ctx, error,
            "xxx_yyy: Unable to create context");
        return NULL;
    }
    
    // 4. Сохраняем зависимости в контексте
    ctx->logger_ctx = logger_ctx;
    
    // 5. Инициализация дополнительных полей контекста (если есть)
    // Например: создание хеш-таблиц, списков и других внутренних структур
    // См. пример в src/modules/luajit/luajit/luajit.c
    
    // 6. Финальное сообщение об инициализации
    ST_LOGGERCTX_CALL(logger_ctx, info,
        "xxx_yyy: Context initialized");
    
    return ctx;
}
```

## Деструктор контекста

Деструктор может быть разным в зависимости от модуля. Простой пример:

```c
static void st_xxx_quit(st_xxxctx_t *ctx) {
    ST_LOGGERCTX_CALL(ctx->logger_ctx, info,
        "xxx_yyy: Context destroyed");
    free(ctx);
}
```

Более сложный пример (с очисткой ресурсов):

```c
static void st_xxx_quit(st_xxxctx_t *ctx) {
    // Очистка списков, хеш-таблиц и других ресурсов
    st_dlist_destroy(ctx->some_list);
    ST_HTABLE_CALL(ctx->some_table, destroy);
    
    ST_LOGGERCTX_CALL(ctx->logger_ctx, info,
        "xxx_yyy: Context destroyed");
    free(ctx);
}
```

## Зависимости

Почти все модули должны зависеть от `logger`:

```c
static const st_modprerq_t mod_prereqs[] = {
    { "logger", NULL, },
    {0},
};
```

**Исключение:** Сам модуль `logger` не зависит от `logger`.

## Примеры модулей

- **Без зависимости от logger:** `src/modules/logger/libsir/`
- **Простой модуль (только контекст):** `src/modules/terminal/simple/`
- **Модуль с объектами:** `src/modules/ini/inih/`, `src/modules/monitor/xlib/`

