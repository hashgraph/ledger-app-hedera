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

// Define context for UI interaction
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
            if (ctx.do_sign) {
                hedera_sign(
                    ctx.key_index, 
                    ctx.raw_transaction, 
                    ctx.raw_transaction_length, 
                    G_io_apdu_buffer
                );
            
                io_exchange_with_code(EXCEPTION_OK, 64);
                ui_idle();
            } else {
                // Signify "do sign" and change UI text
                ctx.do_sign = true;
                
                // Format For Signing a Transaction
                snprintf(ctx.ui_tx_approve_l1, 40, "Sign Transaction");
                snprintf(ctx.ui_tx_approve_l2, 40, "with Key #%u?", ctx.key_index);

                UX_REDISPLAY();
            }

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
    UNUSED(p2);
    UNUSED(len);
    UNUSED(tx);

    // Key Index
    ctx.key_index = U4LE(buffer, 0);
    
    // At first we don't sign and we are presenting the transdaction approval
    ctx.do_sign = false;

    // Raw Tx Length
    ctx.raw_transaction_length = len - 4;
    
    // Oops Oof Owie
    if (ctx.raw_transaction_length > MAX_TX_SIZE) {
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    // Extract Transaction Message
    os_memmove(ctx.raw_transaction, (buffer + 4), ctx.raw_transaction_length);

    // Make in memory buffer into stream
    pb_istream_t stream = pb_istream_from_buffer(
        ctx.raw_transaction, 
        ctx.raw_transaction_length
    );

    // Decode the Transaction
    if (!pb_decode(
        &stream,
        HederaTransactionBody_fields, 
        &ctx.transaction
    )) {
        // Oh no couldn't ...
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    // Which Tx is it?
    switch (ctx.transaction.which_data) {
        case HederaTransactionBody_cryptoCreateAccount_tag:
            snprintf(ctx.ui_tx_approve_l1, 40, "Create Account");
            snprintf(
                ctx.ui_tx_approve_l2, 40, "with %s hbar?", 
                hedera_format_tinybar(ctx.transaction.data.cryptoCreateAccount.initialBalance));

            break;

        case HederaTransactionBody_cryptoTransfer_tag: {
            if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts_count != 2) {
                // Unsupported
                // TODO: Better exception num
                THROW(EXCEPTION_MALFORMED_APDU);
            }

            if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount == 0) {
                // Trying to send 0 is special-cased as an account ID confirmation
                // The SENDER or the Id we are confirming is the first one

                snprintf(
                    ctx.ui_tx_approve_l1, 
                    40, 
                    "Confirm Account"
                );

                snprintf(
                    ctx.ui_tx_approve_l2, 
                    40, 
                    "%llu.%llu.%llu?", 
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.accountNum
                );
            } else {
                // Find sender based on positive tx amount
                ctx.transfer_to_index = 1;
                if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount > 0) {
                    ctx.transfer_to_index = 0;
                }

                snprintf(
                    ctx.ui_tx_approve_l1, 
                    40, 
                    "Transfer %s hbar", 
                    hedera_format_tinybar(
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].amount)
                );

                snprintf(
                    ctx.ui_tx_approve_l2, 40, 
                    "to %llu.%llu.%llu?",
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.accountNum
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
