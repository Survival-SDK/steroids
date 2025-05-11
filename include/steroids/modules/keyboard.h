#pragma once

#include "steroids/module.h"

typedef enum {
    ST_KEY_SPACE         = 0x0020,  /* U+0020 SPACE */
    ST_KEY_EXCLAM        = 0x0021,  /* U+0021 EXCLAMATION MARK */
    ST_KEY_QUOTEDBL      = 0x0022,  /* U+0022 QUOTATION MARK */
    ST_KEY_NUMBERSIGN    = 0x0023,  /* U+0023 NUMBER SIGN */
    ST_KEY_DOLLAR        = 0x0024,  /* U+0024 DOLLAR SIGN */
    ST_KEY_PERCENT       = 0x0025,  /* U+0025 PERCENT SIGN */
    ST_KEY_AMPERSAND     = 0x0026,  /* U+0026 AMPERSAND */
    ST_KEY_APOSTROPHE    = 0x0027,  /* U+0027 APOSTROPHE */
    ST_KEY_QUOTERIGHT    = 0x0027,  /* deprecated */
    ST_KEY_PARENLEFT     = 0x0028,  /* U+0028 LEFT PARENTHESIS */
    ST_KEY_PARENRIGHT    = 0x0029,  /* U+0029 RIGHT PARENTHESIS */
    ST_KEY_ASTERISK      = 0x002A,  /* U+002A ASTERISK */
    ST_KEY_PLUS          = 0x002B,  /* U+002B PLUS SIGN */
    ST_KEY_COMMA         = 0x002C,  /* U+002C COMMA */
    ST_KEY_MINUS         = 0x002D,  /* U+002D HYPHEN-MINUS */
    ST_KEY_PERIOD        = 0x002E,  /* U+002E FULL STOP */
    ST_KEY_SLASH         = 0x002F,  /* U+002F SOLIDUS */
    ST_KEY_0             = 0x0030,  /* U+0030 DIGIT ZERO */
    ST_KEY_1             = 0x0031,  /* U+0031 DIGIT ONE */
    ST_KEY_2             = 0x0032,  /* U+0032 DIGIT TWO */
    ST_KEY_3             = 0x0033,  /* U+0033 DIGIT THREE */
    ST_KEY_4             = 0x0034,  /* U+0034 DIGIT FOUR */
    ST_KEY_5             = 0x0035,  /* U+0035 DIGIT FIVE */
    ST_KEY_6             = 0x0036,  /* U+0036 DIGIT SIX */
    ST_KEY_7             = 0x0037,  /* U+0037 DIGIT SEVEN */
    ST_KEY_8             = 0x0038,  /* U+0038 DIGIT EIGHT */
    ST_KEY_9             = 0x0039,  /* U+0039 DIGIT NINE */
    ST_KEY_COLON         = 0x003A,  /* U+003A COLON */
    ST_KEY_SEMICOLON     = 0x003B,  /* U+003B SEMICOLON */
    ST_KEY_LESS          = 0x003C,  /* U+003C LESS-THAN SIGN */
    ST_KEY_EQUAL         = 0x003D,  /* U+003D EQUALS SIGN */
    ST_KEY_GREATER       = 0x003E,  /* U+003E GREATER-THAN SIGN */
    ST_KEY_QUESTION      = 0x003F,  /* U+003F QUESTION MARK */
    ST_KEY_AT            = 0x0040,  /* U+0040 COMMERCIAL AT */
    ST_KEY_A             = 0x0041,  /* U+0041 LATIN CAPITAL LETTER A */
    ST_KEY_B             = 0x0042,  /* U+0042 LATIN CAPITAL LETTER B */
    ST_KEY_C             = 0x0043,  /* U+0043 LATIN CAPITAL LETTER C */
    ST_KEY_D             = 0x0044,  /* U+0044 LATIN CAPITAL LETTER D */
    ST_KEY_E             = 0x0045,  /* U+0045 LATIN CAPITAL LETTER E */
    ST_KEY_F             = 0x0046,  /* U+0046 LATIN CAPITAL LETTER F */
    ST_KEY_G             = 0x0047,  /* U+0047 LATIN CAPITAL LETTER G */
    ST_KEY_H             = 0x0048,  /* U+0048 LATIN CAPITAL LETTER H */
    ST_KEY_I             = 0x0049,  /* U+0049 LATIN CAPITAL LETTER I */
    ST_KEY_J             = 0x004A,  /* U+004A LATIN CAPITAL LETTER J */
    ST_KEY_K             = 0x004B,  /* U+004B LATIN CAPITAL LETTER K */
    ST_KEY_L             = 0x004C,  /* U+004C LATIN CAPITAL LETTER L */
    ST_KEY_M             = 0x004D,  /* U+004D LATIN CAPITAL LETTER M */
    ST_KEY_N             = 0x004E,  /* U+004E LATIN CAPITAL LETTER N */
    ST_KEY_O             = 0x004F,  /* U+004F LATIN CAPITAL LETTER O */
    ST_KEY_P             = 0x0050,  /* U+0050 LATIN CAPITAL LETTER P */
    ST_KEY_Q             = 0x0051,  /* U+0051 LATIN CAPITAL LETTER Q */
    ST_KEY_R             = 0x0052,  /* U+0052 LATIN CAPITAL LETTER R */
    ST_KEY_S             = 0x0053,  /* U+0053 LATIN CAPITAL LETTER S */
    ST_KEY_T             = 0x0054,  /* U+0054 LATIN CAPITAL LETTER T */
    ST_KEY_U             = 0x0055,  /* U+0055 LATIN CAPITAL LETTER U */
    ST_KEY_V             = 0x0056,  /* U+0056 LATIN CAPITAL LETTER V */
    ST_KEY_W             = 0x0057,  /* U+0057 LATIN CAPITAL LETTER W */
    ST_KEY_X             = 0x0058,  /* U+0058 LATIN CAPITAL LETTER X */
    ST_KEY_Y             = 0x0059,  /* U+0059 LATIN CAPITAL LETTER Y */
    ST_KEY_Z             = 0x005A,  /* U+005A LATIN CAPITAL LETTER Z */
    ST_KEY_BRACKETLEFT   = 0x005B,  /* U+005B LEFT SQUARE BRACKET */
    ST_KEY_BACKSLASH     = 0x005C,  /* U+005C REVERSE SOLIDUS */
    ST_KEY_BRACKETRIGHT  = 0x005D,  /* U+005D RIGHT SQUARE BRACKET */
    ST_KEY_ASCIICIRCUM   = 0x005E,  /* U+005E CIRCUMFLEX ACCENT */
    ST_KEY_UNDERSCORE    = 0x005F,  /* U+005F LOW LINE */
    ST_KEY_GRAVE         = 0x0060,  /* U+0060 GRAVE ACCENT */
    ST_KEY_QUOTELEFT     = 0x0060,  /* deprecated */
    ST_KEY_BRACELEFT     = 0x007B,  /* U+007B LEFT CURLY BRACKET */
    ST_KEY_BAR           = 0x007C,  /* U+007C VERTICAL LINE */
    ST_KEY_BRACERIGHT    = 0x007D,  /* U+007D RIGHT CURLY BRACKET */
    ST_KEY_ASCIITILDE    = 0x007E,  /* U+007E TILDE */

    ST_KEY_BACKSPACE     = 0xFF08,  /* Back space, back char */
    ST_KEY_TAB           = 0xFF09,
    ST_KEY_LINEFEED      = 0xFF0a,  /* Linefeed, LF */
    ST_KEY_CLEAR         = 0xFF0B,
    ST_KEY_RETURN        = 0xFF0D,  /* Return, enter */
    ST_KEY_PAUSE         = 0xFF13,  /* Pause, hold */
    ST_KEY_SCROLL_LOCK   = 0xFF14,
    ST_KEY_SYS_REQ       = 0xFF15,
    ST_KEY_ESCAPE        = 0xFF1B,
    ST_KEY_DELETE        = 0xFFFF,  /* Delete, rubout */

    ST_KEY_HOME          = 0xFF50,
    ST_KEY_LEFT          = 0xFF51,  /* Move left, left arrow */
    ST_KEY_UP            = 0xFF52,  /* Move up, up arrow */
    ST_KEY_RIGHT         = 0xFF53,  /* Move right, right arrow */
    ST_KEY_DOWN          = 0xFF54,  /* Move down, down arrow */
    ST_KEY_PRIOR         = 0xFF55,  /* Prior, previous */
    ST_KEY_PAGE_UP       = 0xFF55,
    ST_KEY_NEXT          = 0xFF56,  /* Next */
    ST_KEY_PAGE_DOWN     = 0xFF56,
    ST_KEY_END           = 0xFF57,  /* EOL */
    ST_KEY_BEGIN         = 0xFF58,  /* BOL */

    ST_KEY_SELECT        = 0xFF60,  /* Select, mark */
    ST_KEY_PRINT         = 0xFF61,
    ST_KEY_EXECUTE       = 0xFF62,  /* Execute, run, do */
    ST_KEY_INSERT        = 0xFF63,  /* Insert, insert here */
    ST_KEY_UNDO          = 0xFF65,
    ST_KEY_REDO          = 0xFF66,  /* Redo, again */
    ST_KEY_MENU          = 0xFF67,
    ST_KEY_FIND          = 0xFF68,  /* Find, search */
    ST_KEY_CANCEL        = 0xFF69,  /* Cancel, stop, abort, exit */
    ST_KEY_HELP          = 0xFF6A,  /* Help */
    ST_KEY_BREAK         = 0xFF6B,
    ST_KEY_MODE_SWITCH   = 0xFF7E,  /* Character set switch */
    ST_KEY_SCRIPT_SWITCH = 0xFF7E,  /* Alias for mode_switch */
    ST_KEY_NUM_LOCK      = 0xFF7F,

    ST_KEY_KP_SPACE      = 0xFF80,  /* Space */
    ST_KEY_KP_TAB        = 0xFF89,
    ST_KEY_KP_ENTER      = 0xFF8D,  /* Enter */
    ST_KEY_KP_F1         = 0xFF91,  /* PF1, KP_A, ... */
    ST_KEY_KP_F2         = 0xFF92,
    ST_KEY_KP_F3         = 0xFF93,
    ST_KEY_KP_F4         = 0xFF94,
    ST_KEY_KP_HOME       = 0xFF95,
    ST_KEY_KP_LEFT       = 0xFF96,
    ST_KEY_KP_UP         = 0xFF97,
    ST_KEY_KP_RIGHT      = 0xFF98,
    ST_KEY_KP_DOWN       = 0xFF99,
    ST_KEY_KP_PRIOR      = 0xFF9A,
    ST_KEY_KP_PAGE_UP    = 0xFF9A,
    ST_KEY_KP_NEXT       = 0xFF9B,
    ST_KEY_KP_PAGE_DOWN  = 0xFF9B,
    ST_KEY_KP_END        = 0xFF9C,
    ST_KEY_KP_BEGIN      = 0xFF9D,
    ST_KEY_KP_INSERT     = 0xFF9E,
    ST_KEY_KP_DELETE     = 0xFF9F,
    ST_KEY_KP_EQUAL      = 0xFFBD,  /* Equals */
    ST_KEY_KP_MULTIPLY   = 0xFFAA,
    ST_KEY_KP_ADD        = 0xFFAB,
    ST_KEY_KP_SEPARATOR  = 0xFFAC,  /* Separator, often comma */
    ST_KEY_KP_SUBTRACT   = 0xFFAD,
    ST_KEY_KP_DECIMAL    = 0xFFAE,
    ST_KEY_KP_DIVIDE     = 0xFFAF,

    ST_KEY_KP_0          = 0xFFB0,
    ST_KEY_KP_1          = 0xFFB1,
    ST_KEY_KP_2          = 0xFFB2,
    ST_KEY_KP_3          = 0xFFB3,
    ST_KEY_KP_4          = 0xFFB4,
    ST_KEY_KP_5          = 0xFFB5,
    ST_KEY_KP_6          = 0xFFB6,
    ST_KEY_KP_7          = 0xFFB7,
    ST_KEY_KP_8          = 0xFFB8,
    ST_KEY_KP_9          = 0xFFB9,

    ST_KEY_F1            = 0xFFBE,
    ST_KEY_F2            = 0xFFBF,
    ST_KEY_F3            = 0xFFC0,
    ST_KEY_F4            = 0xFFC1,
    ST_KEY_F5            = 0xFFC2,
    ST_KEY_F6            = 0xFFC3,
    ST_KEY_F7            = 0xFFC4,
    ST_KEY_F8            = 0xFFC5,
    ST_KEY_F9            = 0xFFC6,
    ST_KEY_F10           = 0xFFC7,
    ST_KEY_F11           = 0xFFC8,
    ST_KEY_L1            = 0xFFC8,
    ST_KEY_F12           = 0xFFC9,
    ST_KEY_L2            = 0xFFC9,
    ST_KEY_F13           = 0xFFCA,
    ST_KEY_L3            = 0xFFCA,
    ST_KEY_F14           = 0xFFCB,
    ST_KEY_L4            = 0xFFCB,
    ST_KEY_F15           = 0xFFCC,
    ST_KEY_L5            = 0xFFCC,
    ST_KEY_F16           = 0xFFCD,
    ST_KEY_L6            = 0xFFCD,
    ST_KEY_F17           = 0xFFCE,
    ST_KEY_L7            = 0xFFCE,
    ST_KEY_F18           = 0xFFCF,
    ST_KEY_L8            = 0xFFCF,
    ST_KEY_F19           = 0xFFD0,
    ST_KEY_L9            = 0xFFD0,
    ST_KEY_F20           = 0xFFD1,
    ST_KEY_L10           = 0xFFD1,
    ST_KEY_F21           = 0xFFD2,
    ST_KEY_R1            = 0xFFD2,
    ST_KEY_F22           = 0xFFD3,
    ST_KEY_R2            = 0xFFD3,
    ST_KEY_F23           = 0xFFD4,
    ST_KEY_R3            = 0xFFD4,
    ST_KEY_F24           = 0xFFD5,
    ST_KEY_R4            = 0xFFD5,
    ST_KEY_F25           = 0xFFD6,
    ST_KEY_R5            = 0xFFD6,
    ST_KEY_F26           = 0xFFD7,
    ST_KEY_R6            = 0xFFD7,
    ST_KEY_F27           = 0xFFD8,
    ST_KEY_R7            = 0xFFD8,
    ST_KEY_F28           = 0xFFD9,
    ST_KEY_R8            = 0xFFD9,
    ST_KEY_F29           = 0xFFDA,
    ST_KEY_R9            = 0xFFDA,
    ST_KEY_F30           = 0xFFDB,
    ST_KEY_R10           = 0xFFDB,
    ST_KEY_F31           = 0xFFDC,
    ST_KEY_R11           = 0xFFDC,
    ST_KEY_F32           = 0xFFDD,
    ST_KEY_R12           = 0xFFDD,
    ST_KEY_F33           = 0xFFDE,
    ST_KEY_R13           = 0xFFDE,
    ST_KEY_F34           = 0xFFDF,
    ST_KEY_R14           = 0xFFDF,
    ST_KEY_F35           = 0xFFE0,
    ST_KEY_R15           = 0xFFE0,

    ST_KEY_SHIFT_L       = 0xFFE1,  /* Left shift */
    ST_KEY_SHIFT_R       = 0xFFE2,  /* Right shift */
    ST_KEY_CONTROL_L     = 0xFFE3,  /* Left control */
    ST_KEY_CONTROL_R     = 0xFFE4,  /* Right control */
    ST_KEY_CAPS_LOCK     = 0xFFE5,  /* Caps lock */
    ST_KEY_SHIFT_LOCK    = 0xFFE6,  /* Shift lock */

    ST_KEY_META_L        = 0xFFE7,  /* Left meta */
    ST_KEY_META_R        = 0xFFE8,  /* Right meta */
    ST_KEY_ALT_L         = 0xFFE9,  /* Left alt */
    ST_KEY_ALT_R         = 0xFFEA,  /* Right alt */
    ST_KEY_SUPER_L       = 0xFFEB,  /* Left super */
    ST_KEY_SUPER_R       = 0xFFEC,  /* Right super */
    ST_KEY_HYPER_L       = 0xFFED,  /* Left hyper */
    ST_KEY_HYPER_R       = 0xFFEE,  /* Right hyper */
} st_key_t;

typedef st_modctx_t *(*st_keyboard_init_t)(st_modctx_t *events_ctx,
 st_modctx_t *logger_ctx);
typedef void (*st_keyboard_quit_t)(st_modctx_t *keyboard_ctx);

typedef void (*st_keyboard_process_t)(st_modctx_t *keyboard_ctx);
typedef bool (*st_keyboard_press_t)(const st_modctx_t *keyboard_ctx,
 st_key_t key);
typedef bool (*st_keyboard_release_t)(const st_modctx_t *keyboard_ctx,
 st_key_t key);
typedef bool (*st_keyboard_pressed_t)(const st_modctx_t *keyboard_ctx,
 st_key_t key);
typedef const char *(*st_keyboard_get_input_t)(const st_modctx_t *keyboard_ctx);

typedef struct {
    st_keyboard_init_t      keyboard_init;
    st_keyboard_quit_t      keyboard_quit;
    st_keyboard_process_t   keyboard_process;
    st_keyboard_press_t     keyboard_press;
    st_keyboard_release_t   keyboard_release;
    st_keyboard_pressed_t   keyboard_pressed;
    st_keyboard_get_input_t keyboard_get_input;
} st_keyboard_funcs_t;
