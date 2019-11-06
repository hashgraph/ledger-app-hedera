#ifndef LEDGER_HEDERA_UI_H
#define LEDGER_HEDERA_UI_H 1

#include "globals.h"
#include "glyphs.h"

#if defined(TARGET_NANOS)

// Forwards for UX System
ux_state_t ux;
unsigned int ux_step;
unsigned int ux_step_count;

// Common UI element definitions for Nano S
#define UI_BACKGROUND() {{BAGL_RECTANGLE,0,0,0,128,32,0,0,BAGL_FILL,0,0xFFFFFF,0,0},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_LEFT(userid, glyph) {{BAGL_ICON,userid,3,12,7,7,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_RIGHT(userid, glyph) {{BAGL_ICON,userid,117,13,8,6,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,0xFFFFFF,0,BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER,0},(char *)(text),0,0,0,NULL,NULL,NULL}
#elif defined(TARGET_NANOX)

// Forwards for UX System
#include "ux.h"
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

// Common UI element definitions for Nano X

#endif // TARGET

extern void ui_idle();

#endif // LEDGER_HEDERA_UI_H
