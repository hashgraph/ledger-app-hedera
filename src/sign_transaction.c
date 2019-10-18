#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "os.h"
#include "os_io_seproxyhal.h"
#include "errors.h"
#include "io.h"
#include "ui.h"
#include "hedera.h"
#include "pb.h"
#include "pb_decode.h"
#include "utils.h"

// Define context for UI interaction
static struct sign_tx_context_t {
    // ui common
    uint32_t key_index;
    
    // ui_transfer_tx_approve
    char ui_tx_approve_l1[40];
    char ui_tx_approve_l2[40];

    // ui_sign_tx_approve
    char ui_sign_tx_approve_l2[40];

    // Raw transaction from APDU
    uint8_t raw_transaction[MAX_TX_SIZE];
    uint16_t raw_transaction_length;
} ctx;

static const bagl_element_t ui_tx_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

    // X                  O
    //   Create Account
    //   with X Hbar?

    UI_TEXT(0x00, 0, 12, 128, ctx.ui_tx_approve_l1),
    UI_TEXT(0x00, 0, 26, 128, ctx.ui_tx_approve_l2)
};

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
                ctx.raw_transaction, 
                ctx.raw_transaction_length, 
                G_io_apdu_buffer
            );

            io_exchange_with_code(EXCEPTION_OK, tx);  // flush
            ui_idle();
            break;
    }

    return 0;
}

static unsigned int ui_tx_approve_button(
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

    // TODO: Investigate accepting tx bodies larger than MAX_TX_SIZE
    ctx.raw_transaction_length = len - 4;
    if (ctx.raw_transaction_length > MAX_TX_SIZE) {
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    // Extract Transaction Message
    os_memset(ctx.raw_transaction, 0, sizeof(ctx.raw_transaction));
    os_memcpy(ctx.raw_transaction, (buffer + 4), ctx.raw_transaction_length);

    // Try to parse transaction body
    HederaTransactionBody hedera_tx = HederaTransactionBody_init_default;
    
    pb_istream_t stream = pb_istream_from_buffer(
        &ctx.raw_transaction[0], 
        ctx.raw_transaction_length
    );
    
    uint8_t status = pb_decode(
        &stream, 
        HederaTransactionBody_fields, 
        &hedera_tx
    );

    PRINTF("CHECK\n");

    if (status == 0) {
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    switch (hedera_tx.which_data) {
        case HederaTransactionBody_cryptoCreateAccount_tag:
            snprintf(ctx.ui_tx_approve_l1, 40, "Create Account");
            snprintf(ctx.ui_tx_approve_l2, 40, "with %llu tħ?", hedera_tx.data.cryptoCreateAccount.initialBalance);
            break;

        case HederaTransactionBody_cryptoTransfer_tag: {
            HederaAccountAmount* accountAmounts = hedera_tx.data.cryptoTransfer.transfers.accountAmounts;
            
            if (hedera_tx.data.cryptoTransfer.transfers.accountAmounts_count != 2) {
                // Unsupported
                // TODO: Better exception num
                THROW(EXCEPTION_MALFORMED_APDU);
            }

            if (accountAmounts[0].amount == 0) {
                // Trying to send 0 is special-cased as an account ID confirmation
                // The SENDER or the Id we are confirming is the first one

                snprintf(
                    ctx.ui_tx_approve_l1, 
                    40, 
                    "Confirm Account ID"
                );

                snprintf(
                    ctx.ui_tx_approve_l2, 
                    40, 
                    "%llu.%llu.%llu?", 
                    accountAmounts[0].accountID.shardNum,
                    accountAmounts[0].accountID.realmNum,
                    accountAmounts[0].accountID.accountNum
                );
            } else {
                snprintf(
                    ctx.ui_tx_approve_l1, 
                    40, 
                    "Transfer %llu tħ", 
                    accountAmounts[0].amount
                );

                int toIndex = 1;
                int fromIndex = 0;

                if (accountAmounts[0].amount > 0) {
                    toIndex = 0;
                    fromIndex = 1;
                }

                snprintf(
                    ctx.ui_tx_approve_l2, 40, 
                    "from %llu.%llu.%llu to %llu.%llu.%llu?",
                    accountAmounts[0].accountID.shardNum,
                    accountAmounts[0].accountID.realmNum,
                    accountAmounts[0].accountID.accountNum,
                    accountAmounts[1].accountID.shardNum,
                    accountAmounts[1].accountID.realmNum,
                    accountAmounts[1].accountID.accountNum
                );
            }
        } break;

        default:
            // Unsupported
            // TODO: Better exception num
            THROW(EXCEPTION_MALFORMED_APDU);
    }

    UX_DISPLAY(ui_tx_approve, NULL);

    *flags |= IO_ASYNCH_REPLY;
}
