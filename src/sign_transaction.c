#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pb.h>
#include <pb_decode.h>

#include "printf.h"
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
    char ui_tx_approve_l3[40];
    char ui_tx_approve_l4[40];

    // what step of the UI flow are we on
    bool do_sign;

    // verify account transaction
    bool do_verify;

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

// Each UI element has a macro defined function that is its
// button handler, which must be named after the element with _button
// appended. This function is called on every single iteration of 
// the app loop while the async reply flag is set. The events consume
// that flag and allow the app to continue. 
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
                // Step 2
                hedera_sign(
                    ctx.key_index, 
                    ctx.raw_transaction, 
                    ctx.raw_transaction_length, 
                    G_io_apdu_buffer
                );
            
                io_exchange_with_code(EXCEPTION_OK, 64);
                ui_idle();
            } else {
                // Step 1
                // Signify "do sign" and change UI text
                ctx.do_sign = true;

                // if this is a verify account transaction (1 Sender, 0 Value)
                // then format for account verification
                if (ctx.do_verify) {
                    hedera_snprintf(ctx.ui_tx_approve_l1, 40, "Verify Account ID");
                } else {
                    // Format for Signing a Transaction
                    hedera_snprintf(ctx.ui_tx_approve_l1, 40, "Sign Transaction");
                }

                hedera_snprintf(ctx.ui_tx_approve_l2, 40, "with Key #%u?", ctx.key_index);

                UX_REDISPLAY();
            }

            break;
    }

    return 0;
}

#elif defined(TARGET_NANOX)

unsigned int io_seproxyhal_confirm_tx_approve(const bagl_element_t *e) {
    hedera_sign(
        ctx.key_index,
        ctx.raw_transaction,
        ctx.raw_transaction_length,
        G_io_apdu_buffer
    );
    io_exchange_with_code(EXCEPTION_OK, 64);
    ui_idle();
    return 0;
}

unsigned int io_seproxyhal_confirm_tx_reject(const bagl_element_t *e) {
     io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
     ui_idle();
     return 0;
}

UX_STEP_NOCB(
    ux_confirm_tx_flow_1_step,
    bnn,
    {
        "Transaction Details",
        ctx.ui_tx_approve_l1,
        ctx.ui_tx_approve_l2
    }
);

UX_STEP_NOCB(
    ux_confirm_tx_flow_2_step,
    bnn,
    {
        "Confirm Transaction",
        ctx.ui_tx_approve_l3,
        ctx.ui_tx_approve_l4
    }
);


UX_STEP_VALID(
    ux_confirm_tx_flow_3_step,
    pb,
    io_seproxyhal_confirm_tx_approve(NULL),
    {
        &C_icon_validate_14,
        "Accept"
    }
);

UX_STEP_VALID(
    ux_confirm_tx_flow_4_step,
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
    &ux_confirm_tx_flow_3_step,
    &ux_confirm_tx_flow_4_step
);

#endif

void handle_transaction_body() {
#if defined(TARGET_NANOS)
    // init at sign step 1, not verifying
    ctx.do_sign = false;
    ctx.do_verify = false;
#elif defined(TARGET_NANOX)
    // init key line for nano x
    hedera_snprintf(ctx.ui_tx_approve_l3, 40, "Sign Transaction");
    hedera_snprintf(ctx.ui_tx_approve_l4, 40, "with Key #%u?", ctx.key_index);
#endif
    // Handle parsed protobuf message of transaction body
    switch (ctx.transaction.which_data) {
        // It's a "Create Account" transaction
        case HederaTransactionBody_cryptoCreateAccount_tag:
            hedera_snprintf(ctx.ui_tx_approve_l1, 40, "Create Account");
            hedera_snprintf(
                    ctx.ui_tx_approve_l2, 40, "with %s hbar?",
                    hedera_format_tinybar(ctx.transaction.data.cryptoCreateAccount.initialBalance));
            break;

        // It's a "Transfer" transaction
        case HederaTransactionBody_cryptoTransfer_tag: {
            if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts_count > 2) {
                // Unsupported (number of accounts > 2)
                THROW(EXCEPTION_MALFORMED_APDU);
            }

            // It's actually a "Verify Account" transaction (login)
            if ( // Only 1 Account (Sender), Fee 1 Tinybar, and Value 0 Tinybar
                ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount == 0 && 
                ctx.transaction.data.cryptoTransfer.transfers.accountAmounts_count == 1 &&
                ctx.transaction.transactionFee == 1) {

                #if defined(TARGET_NANOS)
                    ctx.do_verify = true;
                #elif defined(TARGET_NANOX)
                    hedera_snprintf(ctx.ui_tx_approve_l3, 40, "Verify Account ID");
                #endif  

                hedera_snprintf(
                    ctx.ui_tx_approve_l1,
                    40,
                    "Confirm Account"
                );

                hedera_snprintf(
                    ctx.ui_tx_approve_l2,
                    40,
                    "%llu.%llu.%llu?",
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.accountNum
                );
            } else {
                // It's a transfer transaction between two parties
                // Find sender based on positive tx amount
                ctx.transfer_to_index = 1;
                if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount > 0) {
                    ctx.transfer_to_index = 0;
                }

                hedera_snprintf(
                        ctx.ui_tx_approve_l1,
                        40,
                        "Transfer %s hbar",
                        hedera_format_tinybar(
                                ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].amount)
                );

                hedera_snprintf(
                        ctx.ui_tx_approve_l2,
                        40,
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

#if defined(TARGET_NANOS)
    UX_DISPLAY(ui_tx_approve, NULL);
#elif defined(TARGET_NANOX)
    ux_flow_init(0, ux_confirm_tx_flow, NULL);
#endif
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

    handle_transaction_body();

    *flags |= IO_ASYNCH_REPLY;
}
