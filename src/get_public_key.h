#ifndef LEDGER_HEDERA_GET_PUBLIC_KEY_H
#define LEDGER_HEDERA_GET_PUBLIC_KEY_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <printf.h>

#include "errors.h"
#include "hedera.h"
#include "io.h"
#include "ui.h"
#include "debug.h"
#include "utils.h"

#if defined(TARGET_NANOS)

// Sizes in Characters, not Bytes
// Used for display only
static const uint8_t KEY_SIZE = 64;
static const uint8_t DISPLAY_SIZE = 12;

// Arbitrary IDs for Buttons
static const uint8_t LEFT_ID = 0x01;
static const uint8_t RIGHT_ID = 0x02;

static struct get_public_key_context_t {
    uint32_t key_index;
    
    // Lines on the UI Screen
    char ui_approve_l2[40];

    cx_ecfp_public_key_t public;

    // Public Key Compare
    uint8_t display_index;
    uint8_t full_key[KEY_SIZE + 1];
    uint8_t partial_key[DISPLAY_SIZE + 1];
} ctx;

static const bagl_element_t ui_get_public_key_compare[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ID, BAGL_GLYPH_ICON_RIGHT),
    // <=                  =>
    //      Compare:         
    //      <partial>        
    //                       
    UI_TEXT(0x00, 0, 12, 128, "Public Key"),
    UI_TEXT(0x00, 0, 26, 128, ctx.partial_key)
};

void shift_partial_key();

static unsigned int ui_get_public_key_compare_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

static const bagl_element_t* ui_prepro_get_public_key_compare(
    const bagl_element_t* element
);

void send_pk();
void compare_pk();

static const bagl_element_t ui_get_public_key_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),
    //
    //    Export Public
    //       Key #123?
    //
    UI_TEXT(0x00, 0, 12, 128, "Export Public"),
    UI_TEXT(0x00, 0, 26, 128, ctx.ui_approve_l2),
};

static unsigned int ui_get_public_key_approve_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

void handle_get_public_key_nanos();

#elif defined(TARGET_NANOX)

static struct get_public_key_context_t {
    uint32_t key_index;
    cx_ecfp_public_key_t public;
} ctx;

void handle_get_public_key_nanox();

#endif // TARGET

void handle_get_public_key(
        uint8_t p1,
        uint8_t p2,
        const uint8_t* const buffer,
        uint16_t len,
        /* out */ volatile unsigned int* flags,
        /* out */ volatile const unsigned int* const tx
);

#endif // LEDGER_HEDERA_GET_PUBLIC_KEY_H
