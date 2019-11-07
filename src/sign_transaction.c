#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <printf.h>
#include <pb.h>
#include <pb_decode.h>

#include "globals.h"
#include "debug.h"
#include "errors.h"
#include "handlers.h"
#include "hedera.h"
#include "io.h"
#include "TransactionBody.pb.h"
#include "utils.h"
#include "ui.h"
#include "sign_transaction.h"

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

#if defined(TARGET_NANOS)

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

void handle_sign_transaction_nanos() {
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
}

#elif defined(TARGET_NANOX)

unsigned int io_seproxyhal_sign_tx_approve(const bagl_element_t *e) {
    hedera_sign(
        ctx.key_index,
        ctx.raw_transaction,
        ctx.raw_transaction_length,
        G_io_apdu_buffer
    );
    io_exchange_with_code(EXCEPTION_OK, 32);
    ui_idle();
    return 0;
}

unsigned int io_seproxyhal_sign_tx_reject(const bagl_element_t *e) {
     io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
     ui_idle();
     return 0;
}

UX_STEP_NOCB(
    ux_sign_tx_flow_1_step,
    nn,
    {
        ctx.ui_tx_approve_l1,
        ctx.ui_tx_approve_l2
    }
);

UX_STEP_VALID(
    ux_sign_tx_flow_2_step,
    pb,
    io_seproxyhal_sign_tx_approve(NULL),
    {
        &C_icon_validate_14,
        "Approve"
    }
);

UX_STEP_VALID(
    ux_sign_tx_flow_3_step,
    pb,
    io_seproxyhal_sign_tx_reject(NULL),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

UX_DEF(
    ux_sign_tx_flow,
    &ux_sign_tx_flow_1_step,
    &ux_sign_tx_flow_2_step,
    &ux_sign_tx_flow_3_step
);

unsigned int io_seproxyhal_confirm_tx_approve(const bagl_element_t *e) {
    SPRINTF(ctx.ui_tx_approve_l1, "Sign Transaction");
    SPRINTF(ctx.ui_tx_approve_l2, "with Key #%u?", ctx.key_index);
    ux_flow_init(0, ux_sign_tx_flow, NULL);
    return 0;
}

unsigned int io_seproxyhal_confirm_tx_reject(const bagl_element_t *e) {
     io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
     ui_idle();
     return 0;
}

UX_STEP_NOCB(
    ux_confirm_tx_flow_1_step,
    nn,
    {
        ctx.ui_tx_approve_l1,
        ctx.ui_tx_approve_l2
    }
);

UX_STEP_VALID(
    ux_confirm_tx_flow_2_step,
    pb,
    io_seproxyhal_confirm_tx_approve(NULL),
    {
        &C_icon_validate_14,
        "Accept"
    }
);

UX_STEP_VALID(
    ux_confirm_tx_flow_3_step,
    pb,
    io_seproxyhal_confirm_tx_reject(NULL),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

UX_DEF(
    ux_confirm_tx_flow,
    &ux_confirm_tx_flow_1_step,
    &ux_confirm_tx_flow_2_step,
    &ux_confirm_tx_flow_3_step
);

void handle_sign_transaction_nanox() {
    // Which Tx is it?
    switch (ctx.transaction.which_data) {
        case HederaTransactionBody_cryptoCreateAccount_tag:
            SPRINTF(ctx.ui_tx_approve_l1, "Create Account");
            SPRINTF(ctx.ui_tx_approve_l2, "with %s hbar?",
                    hedera_format_tinybar(ctx.transaction.data.cryptoCreateAccount.initialBalance));
            break;

        case HederaTransactionBody_cryptoTransfer_tag: {
            if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts_count != 2) {
                // Unsupported
                THROW(EXCEPTION_MALFORMED_APDU);
            }

            if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount == 0) {
                // Trying to send 0 is special-cased as an account ID confirmation
                // The SENDER or the Id we are confirming is the first one
                SPRINTF(
                    ctx.ui_tx_approve_l1,
                    "Confirm Account"
                );

                SPRINTF(
                    ctx.ui_tx_approve_l2,
                    "%u.%u.%u?",
                    (unsigned int)ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.shardNum,
                    (unsigned int)ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.realmNum,
                    (unsigned int)ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.accountNum
                );
            } else {
                // Find sender based on positive tx amount
                ctx.transfer_to_index = 1;

                if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount > 0) {
                    ctx.transfer_to_index = 0;
                }

                SPRINTF(
                    ctx.ui_tx_approve_l1,
                    "Transfer %s hbar",
                    hedera_format_tinybar(
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].amount)
                );

                SPRINTF(
                    ctx.ui_tx_approve_l2,
                    "to %u.%u.%u?",
                    (unsigned int)ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.shardNum,
                    (unsigned int)ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.realmNum,
                    (unsigned int)ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.accountNum
                );
            }
        } break;

        default:
            THROW(EXCEPTION_MALFORMED_APDU);
    }

    ux_flow_init(0, ux_confirm_tx_flow, NULL);
}

#endif

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

#if defined(TARGET_NANOS)

    // At first, don't actually sign. Redisplay after approval, then sign.
    ctx.do_sign = false;
    handle_sign_transaction_nanos();

#elif defined(TARGET_NANOX)

    handle_sign_transaction_nanox();

#endif

    *flags |= IO_ASYNCH_REPLY;
}
