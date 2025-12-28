# Специфика модуля gfxctx

Этот файл содержит специфичные правила для работы с модулем графического контекста (`gfxctx`).

## Обработка ошибок критических функций

### Функции графического контекста (gfxctx)

**Обязательно проверяй возвращаемые значения** для следующих функций:

#### st_gfxctx_make_current
```c
bool (*make_current)(const st_gfxctx_t *gfxctx);
```

Эта функция делает графический контекст текущим для потока. Возвращает `false` при ошибке.

**Где используется:**
- Перед любыми OpenGL операциями
- В начале рендера кадра
- Перед загрузкой текстур

**Пример правильного использования:**
```c
if (!ST_GFXCTX_CALL(render_ctx->gfxctx, make_current)) {
    ST_LOGGERCTX_CALL(ctx->logger_ctx, error,
        "%s_%s: Failed to make context current", 
        st_module_subsystem, st_module_name);
    return false;
}
```

#### st_gfxctx_swap_buffers
```c
bool (*swap_buffers)(const st_gfxctx_t *gfxctx);
```

Эта функция меняет местами front и back буферы. Возвращает `false` при ошибке.

**Где используется:**
- В конце рендера кадра
- После всех OpenGL draw calls

**Пример правильного использования:**
```c
if (!ST_GFXCTX_CALL(render_ctx->gfxctx, swap_buffers)) {
    ST_LOGGERCTX_CALL(ctx->logger_ctx, error, "%s_%s: Failed to swap buffers", 
        st_module_subsystem, st_module_name);
    return;
}
```
