#ifndef LEDGER_HEDERA_UI_H
#define LEDGER_HEDERA_UI_H 1

#include "glyphs.h"
#include "globals.h"

#if defined(TARGET_NANOS)

// Common UI element definitions for Nano S
#define UI_BACKGROUND() {{BAGL_RECTANGLE,0,0,0,128,32,0,0,BAGL_FILL,0,0xFFFFFF,0,0},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_LEFT(userid, glyph) {{BAGL_ICON,userid,3,12,7,7,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_ICON_RIGHT(userid, glyph) {{BAGL_ICON,userid,117,13,8,6,0,0,0,0xFFFFFF,0,0,glyph},NULL,0,0,0,NULL,NULL,NULL}
#define UI_TEXT(userid, x, y, w, text) {{BAGL_LABELINE,userid,x,y,w,12,0,0,0,0xFFFFFF,0,BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER,0},(char *)(text),0,0,0,NULL,NULL,NULL}

#elif defined(TARGET_NANOX)

#include "ux.h"

// Common UI element definitions for Nano X

#endif // TARGET

// Forwards for UX and UI IO functions
extern unsigned int io_seproxyhal_touch_settings(const bagl_element_t *e);
extern unsigned int io_seproxyhal_touch_exit(const bagl_element_t *e);
extern unsigned int io_seproxyhal_touch_tx_ok(const bagl_element_t *e);
extern unsigned int io_seproxyhal_touch_tx_cancel(const bagl_element_t *e);
extern unsigned int io_seproxyhal_touch_address_ok(const bagl_element_t *e);
extern unsigned int io_seproxyhal_touch_address_cancel(const bagl_element_t *e);
extern void ui_idle(void);


#endif // LEDGER_HEDERA_UI_H
