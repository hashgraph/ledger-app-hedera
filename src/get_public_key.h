#ifndef LEDGER_HEDERA_GET_PUBLIC_KEY_H
#define LEDGER_HEDERA_GET_PUBLIC_KEY_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <printf.h>

#include "globals.h"
#include "debug.h"
#include "errors.h"
#include "handlers.h"
#include "hedera.h"
#include "io.h"
#include "utils.h"
#include "ui.h"

#if defined(TARGET_NANOS)

// Sizes in Characters, not Bytes
// Used for display only
static const uint8_t KEY_SIZE = 64;
static const uint8_t DISPLAY_SIZE = 12;

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

void handle_get_public_key_nanos();

#elif defined(TARGET_NANOX)

void handle_get_public_key_nanox();

#endif // TARGET

#endif // LEDGER_HEDERA_GET_PUBLIC_KEY_H
