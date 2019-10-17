#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "os.h"
#include "os_io_seproxyhal.h"
#include "errors.h"
#include "io.h"
#include "ui.h"
#include "hedera.h"
#include "utils.h"

// Define context for UI interaction
static struct sign_tx_context_t {
    // ui common
    uint32_t key_index;
    
    // ui_transfer_tx_approve
    char ui_transfer_tx_approve_l1[40];
    char ui_transfer_tx_approve_l2[40];

    // ui_create_account_tx_approve
    char ui_create_account_tx_l2[40];

    // ui_sign_tx_approve
    char ui_sign_tx_approve_l2[40];

    // Transaction from APDU
    uint8_t transaction[MAX_TX_SIZE];
    uint8_t transaction_length;
} ctx;

static unsigned int continue_to_sign_tx(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
            ui_idle();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            UX_DISPLAY(ui_sign_tx_approve, NULL);
            break;
    }
}

static const bagl_element_t ui_transfer_tx_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

    // X                  O
    //   Transfer X Hbar
    //   from 0.0.Y to 0.0.Z?

    UI_TEXT(0x00, 0, 12, 128, ctx.ui_transfer_tx_approve_l1),
    UI_TEXT(0x00, 0, 26, 128, ctx.ui_transfer_tx_approve_l2)
};

static unsigned int ui_transfer_tx_approve_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    continue_to_sign_tx(button_mask, button_mask_counter);
}

static const bagl_element_t ui_create_account_tx_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

    // X                  O
    //   Create Account
    //   with X Hbar?

    UI_TEXT(0x00, 0, 12, 128, "Create Account"),
    UI_TEXT(0x00, 0, 26, 128, ctx.ui_create_account_tx_l2)
};

static unsigned int ui_create_account_tx_approve_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    continue_to_sign_tx(button_mask, button_mask_counter);
}

static const bagl_element_t ui_sign_tx_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

    // X                  O
    //   Sign Transaction
    //   with Key #0?

    UI_TEXT(0x00, 0, 12, 128, "Sign Transaction"),
    UI_TEXT(0x00, 0, 26, 128, ctx.ui_sign_tx_approve_l2)
};

// ui_sign_tx_approve
static unsigned int ui_sign_tx_approve_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
    ) {
    uint16_t tx = 0;

    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:  // Reject
            io_exchange_with_code(EXCEPTION_USER_REJECTED, tx);
            ui_idle();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:  // Approve
            tx += hedera_sign(
                ctx.key_index, 
                ctx.transaction, 
                ctx.transaction_length, 
                G_io_apdu_buffer
            );
            io_exchange_with_code(EXCEPTION_OK, tx);  // flush
            ui_idle();
            break;
    }

    return 0;
}

// Handle parsing APDU and displaying UI element
void handle_sign_transaction(
    uint8_t p1,
    uint8_t p2,
    uint8_t* buffer,
    uint16_t len,
    /* out */ volatile unsigned int* flags,
    /* out */ volatile unsigned int* tx
) {
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(len);
    UNUSED(tx);

    // Get Key Index and Prepare Message
    ctx.key_index = U4LE(buffer, 0);
    snprintf(ctx.ui_sign_tx_approve_l2, 40, "with Key #%d?", ctx.key_index);

    uint8_t* tx = buffer + 4;
    uint8_t tx_len = len - 4;
    ctx.transaction_length = tx_len;

    // TODO: Use P1_MORE to accept a streaming body (for > max(APDU))
    if (sizeof(tx) > MAX_TX_SIZE) {
        throw(EXCEPTION_MALFORMED_APDU);
    }

    // Extract Transaction Message
    os_memset(ctx.transaction, 0, sizeof(ctx.transaction));
    os_memcpy(ctx.transaction, tx, tx_len * sizeof(uint8_t));

    // Try to parse transaction body
    HederaCryptoTransferTransactionBody transfer_tx;
    HederaCryptoCreateTransactionBody create_tx;
    hedera_unpack_tx(
        ctx.transaction, 
        ctx.transaction_length, 
        &transfer_tx,  // out
        &create_tx  // out
    );

    // Check each Transaction Struct, unsuccessful parse = NULL
    if (&transfer_tx == NULL && &create_tx == NULL) {
        throw(EXCEPTION_MALFORMED_APDU);
    } else if (&transfer_tx != NULL) {
        snprintf(
            ctx.ui_transfer_tx_approve_l1, 
            40, 
            "Transfer %.2f Hbar", 
            transfer_tx.transfers.accountAmounts[0].amount
        );
        snprintf(
            ctx.ui_transfer_tx_approve_l2,
            40,
            "from %s to %s?",
            sprintf(
                "%s.%s.%s", 
                transfer_tx.transfers.accountAmounts[0].accountID.shardNum,
                transfer_tx.transfers.accountAmounts[0].accountID.realmNum,
                transfer_tx.transfers.accountAmounts[0].accountID.accountNum
            ),
            sprintf(
                "%s.%s.%s",
                transfer_tx.transfers.accountAmounts[1].accountID.shardNum,
                transfer_tx.transfers.accountAmounts[1].accountID.realmNum,
                transfer_tx.transfers.accountAmounts[1].accountID.accountNum
            )
        );
        UX_DISPLAY(ui_transfer_tx_approve, NULL);
    } else if (&ui_create_account_tx_approve != NULL) {
        snprintf(
            ctx.ui_create_account_tx_l2,
            40,
            "with %.2f Hbar?",
            create_tx.initialBalance
        );
        UX_DISPLAY(ui_transfer_tx_approve, NULL);
    }

    *flags |= IO_ASYNCH_REPLY;
}
