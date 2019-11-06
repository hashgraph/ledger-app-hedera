#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <os.h>
#include <os_io_seproxyhal.h>
#include <pb.h>
#include <pb_decode.h>
#include <printf.h>

#include "errors.h"
#include "io.h"
#include "ui.h"
#include "hedera.h"
#include "handlers.h"
#include "utils.h"
#include "TransactionBody.pb.h"

#if defined(TARGET_NANOS)

// Sign Transaction Context for Nano S
static struct sign_tx_context_t {
    // ui common
    uint32_t key_index;

    // temp variables
    uint8_t transfer_to_index;

    // ui_transfer_tx_approve
    char ui_tx_approve_l1[40];
    char ui_tx_approve_l2[40];

    // what step of the UI flow are we on
    bool do_sign;

    // Raw transaction from APDU
    uint8_t raw_transaction[MAX_TX_SIZE];
    uint16_t raw_transaction_length;

    // Parsed transaction
    HederaTransactionBody transaction;
} ctx;

// UI definition for Nano S
static const bagl_element_t ui_tx_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

    // X                  O
    //   Line 1
    //   Line 2

    UI_TEXT(0x00, 0, 12, 128, ctx.ui_tx_approve_l1),
    UI_TEXT(0x00, 0, 26, 128, ctx.ui_tx_approve_l2)
};

unsigned int ui_tx_approve_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

void handle_sign_transaction_nanos();

#elif defined(TARGET_NANOX)

static struct sign_tx_context_t {
   // ui common
    uint32_t key_index;

    // temp variables
    uint8_t transfer_to_index;

    // Raw transaction from APDU
    uint8_t raw_transaction[MAX_TX_SIZE];
    uint16_t raw_transaction_length;

    // Parsed transaction
    HederaTransactionBody transaction;
} ctx;

void handle_sign_transaction_nanox();

#endif // TARGET

void handle_sign_transaction(
        uint8_t p1,
        uint8_t p2,
        uint8_t* buffer,
        uint16_t len,
        /* out */ volatile unsigned int* flags,
        /* out */ volatile unsigned int* tx
);

#endif //LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
