#ifndef LEDGER_HEDERA_GET_PUBLIC_KEY_H
#define LEDGER_HEDERA_GET_PUBLIC_KEY_H 1

// Sizes in Characters, not Bytes
// Used for Display Only
static const uint8_t KEY_SIZE = 64;
static const uint8_t DISPLAY_SIZE = 12;

void get_pk();

#if defined(TARGET_NANOS)
#include <printf.h>

// Arbitrary IDs for Buttons
static const uint8_t LEFT_ID = 0x01;
static const uint8_t RIGHT_ID = 0x02;

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

static unsigned int ui_get_public_key_approve_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

unsigned int io_seproxyhal_touch_pk_ok(const bagl_element_t *e);
unsigned int io_seproxyhal_touch_pk_cancel(const bagl_element_t *e);
void handle_get_public_key_nanos();

#elif defined(TARGET_NANOX)

void handle_get_public_key_nanox();

#endif // TARGET

#endif // LEDGER_HEDERA_GET_PUBLIC_KEY_H
