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
    uint8_t transfer_to_index;

    // Transaction Summary
    char summary_line_1[40];
    char summary_line_2[40];
    
    // Transaction Amount
    char amount[DISPLAY_SIZE * 3 + 1];
    char amount_title[40];
    char partial_amount[DISPLAY_SIZE + 1];
    uint8_t amount_display_index;  // current
    uint8_t amount_display_count;  // total
    
    // Transaction Fee
    char fee[DISPLAY_SIZE * 3 + 1];
    char fee_title[40];
    char partial_fee[DISPLAY_SIZE + 1];
    uint8_t fee_display_index;  // current
    uint8_t fee_display_count;  // total

    // Raw transaction from APDU
    uint8_t raw_transaction[MAX_TX_SIZE];
    uint16_t raw_transaction_length;

    // Parsed transaction
    HederaTransactionBody transaction;
} ctx;

#if defined(TARGET_NANOS)
// UI Definition for Nano S
// Step 1: Transaction Summary
static const bagl_element_t ui_tx_summary_step[] = {
    UI_BACKGROUND(),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // ()       >>
    // Line 1
    // Line 2

    UI_TEXT(LINE_1_ID, 0, 12, 128, ctx.summary_line_1),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.summary_line_2)
};

// Step 2: Amount
static const bagl_element_t ui_tx_amount_step[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // <<       >>
    // Amount (1/X)
    // <Partial Amount>

    UI_TEXT(LINE_1_ID, 0, 12, 128, ctx.amount_title),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.partial_amount)
};

// Step 3: Fee
static const bagl_element_t ui_tx_fee_step[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // <<       >>
    // Fee (1/X)
    // <Partial Fee>

    UI_TEXT(LINE_1_ID, 0, 12, 128, ctx.fee_title),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.partial_fee)
};

// Step 4: Confirm
static const bagl_element_t ui_tx_confirm_step[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // <<       >>
    //    Confirm
    //    <Check>

    UI_TEXT(LINE_1_ID, 0, 12, 128, "Confirm"),
    UI_ICON(LINE_2_ID, 0, 24, 128, BAGL_GLYPH_ICON_CHECK)
};

// Step 5: Deny
static const bagl_element_t ui_tx_deny_step[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),

    // <<       ()
    //    Deny
    //      X

    UI_TEXT(LINE_1_ID, 0, 12, 128, "Deny"),
    UI_ICON(LINE_2_ID, 0, 24, 128, BAGL_GLYPH_ICON_CROSS)
};

// Step 1: Transaction Summary
unsigned int ui_tx_summary_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            ctx.amount_display_index = 1;
            UX_DISPLAY(ui_tx_amount_step, NULL);
            break;
    }

    return 0;
}

// Step 2: Amount
unsigned int ui_tx_amount_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Scroll left or return to summary
            if (ctx.amount_display_index == 1) {
                UX_DISPLAY(ui_tx_summary_step, NULL);
            } else {
                ctx.amount_display_index--;
                reformat_amount();
                UX_REDISPLAY();
            }
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Scroll right or continue to fee
            if (ctx.amount_display_index == ctx.amount_display_count) {
                ctx.fee_display_index = 1;
                reformat_fee();
                UX_DISPLAY(ui_tx_fee_step, NULL);
            } else {
                ctx.amount_display_index++;
                reformat_amount();
                UX_REDISPLAY();
            }
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Continue to Fee, regardless of current position
            ctx.fee_display_index = 1;
            UX_DISPLAY(ui_tx_fee_step, NULL);
            break;
    }

    return 0;
}

// Step 3: Fee
unsigned int ui_tx_fee_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Scroll left or return to amount
            if (ctx.fee_display_index == 1) {
                ctx.amount_display_index = 1;
                reformat_amount();
                UX_DISPLAY(ui_tx_amount_step, NULL);
            } else {
                ctx.fee_display_index--;
                reformat_fee();
                UX_REDISPLAY();
            }
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Scroll right or continue to confirm
            if (ctx.fee_display_index == ctx.fee_display_count) {
                UX_DISPLAY(ui_tx_confirm_step, NULL);
            } else {
                ctx.fee_display_index++;
                reformat_fee();
                UX_REDISPLAY();
            }
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Continue to confirm, regardless of current position
            UX_DISPLAY(ui_tx_confirm_step, NULL);
            break;
    }

    return 0;
}

// Step 4: Confirm
unsigned int ui_tx_confirm_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Return to Fee
            ctx.fee_display_index = 1;
            reformat_fee();
            UX_DISPLAY(ui_tx_fee_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Continue to Deny
            UX_DISPLAY(ui_tx_deny_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Sign and exchange ok
            hedera_sign(
                ctx.key_index,
                ctx.raw_transaction,
                ctx.raw_transaction_length,
                G_io_apdu_buffer
            );
            io_exchange_with_code(EXCEPTION_OK, 64);
            ui_idle();
            break;
    }

    return 0;
}

// Step 5: Deny
unsigned int ui_tx_deny_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Return to Confirm
            UX_DISPLAY(ui_tx_confirm_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Reject
            io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
            ui_idle();
            break;
    }

    return 0;
}

void reformat_amount() {
    hedera_snprintf(
        ctx.amount_title,
        40, 
        "Amount (%u/%u)",
        ctx.amount_display_index,
        ctx.amount_display_count
    );
    os_memset(ctx.partial_amount, '\0', DISPLAY_SIZE + 1);
    os_memmove(
        ctx.partial_amount,
        ctx.amount + (DISPLAY_SIZE * (ctx.amount_display_index - 1)),
        DISPLAY_SIZE
    );
}

void reformat_fee() {
    hedera_snprintf(
        ctx.fee_title,
        40,
        "Fee (%u/%u)",
        ctx.fee_display_index,
        ctx.fee_display_count
    );
    os_memset(ctx.partial_fee, '\0', DISPLAY_SIZE + 1);
    os_memmove(
        ctx.partial_fee,
        ctx.fee + (DISPLAY_SIZE * (ctx.fee_display_index - 1)),
        DISPLAY_SIZE
    );
}

uint8_t num_screens(char* text) {
    uint8_t screens = 0;
    size_t len = strlen(text);
    screens += len / DISPLAY_SIZE;
    if (len % DISPLAY_SIZE != 0) screens += 1;
    return screens;
}

void setup_nanos_paging() {
    ctx.amount_display_index = 1;
    ctx.amount_display_count = num_screens(ctx.amount);
    ctx.fee_display_index = 1;
    ctx.fee_display_count = num_screens(ctx.fee);
    reformat_amount();
    reformat_fee();
}

#elif defined(TARGET_NANOX)
// UI Definition for Nano X

// Confirm Callback
unsigned int io_seproxyhal_tx_approve(const bagl_element_t* e) {
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

// Reject Callback
unsigned int io_seproxyhal_tx_reject(const bagl_element_t* e) {
    io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
    ui_idle();
    return 0;
}

// Step 1: Summary
UX_STEP_NOCB(
    ux_tx_flow_1_step,
    bnn,
    {
        "Transaction Summary",
        ctx.summary_line_1,
        ctx.summary_line_2
    }
);

// Step 2: Amount
UX_STEP_NOCB(
    ux_tx_flow_2_step,
    bnnn_paging,
    {
        .title = "Amount",
        .text = (char*) ctx.amount
    }
);

// Step 3: Fee
UX_STEP_NOCB(
    ux_tx_flow_3_step,
    bnnn_paging,
    {
        .title = "Fee",
        .text = (char*) ctx.fee
    }
);

// Step 4: Confirm
UX_STEP_VALID(
    ux_tx_flow_4_step,
    pbb,
    io_seproxyhal_tx_approve(NULL),
    {
        &C_icon_validate_14,
        "Confirm"
    }
);

// Step 5: Reject
UX_STEP_VALID(
    ux_tx_flow_5_step,
    pbb,
    io_seproxyhal_tx_reject(NULL),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

// Transaction UX Flow
UX_DEF(
    ux_tx_flow,
    &ux_tx_flow_1_step,
    &ux_tx_flow_2_step,
    &ux_tx_flow_3_step,
    &ux_tx_flow_4_step,
    &ux_tx_flow_5_step
);

#endif

void handle_transaction_body() {
    os_memset(ctx.fee, '\0', DISPLAY_SIZE * 3 + 1);
    os_memset(ctx.amount, '\0', DISPLAY_SIZE * 3 + 1);
    // <Do Action> 
    // with Key #X?
    hedera_snprintf(
        ctx.summary_line_2,
        40,
        "with Key #%u?",
        ctx.key_index
    );

    hedera_snprintf(
        ctx.fee,
        DISPLAY_SIZE * 3,
        "%s hbar",
        hedera_format_tinybar(ctx.transaction.transactionFee)
    );

    // Handle parsed protobuf message of transaction body
    switch (ctx.transaction.which_data) {
        case HederaTransactionBody_cryptoCreateAccount_tag:
            // Create Account Transaction
            hedera_snprintf(
                ctx.summary_line_1,
                40,
                "Create Account"
            );
            hedera_snprintf(
                ctx.amount,
                DISPLAY_SIZE * 3,
                "%s hbar",
                hedera_format_tinybar(ctx.transaction.data.cryptoCreateAccount.initialBalance)
            );
            break;

        case HederaTransactionBody_cryptoTransfer_tag: {
            // Transfer Transaction
            if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts_count > 2) {
                // Unsupported (number of accounts > 2)
                THROW(EXCEPTION_MALFORMED_APDU);
            }

            if ( // Only 1 Account (Sender), Fee 1 Tinybar, and Value 0 Tinybar
                ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount == 0 && 
                ctx.transaction.data.cryptoTransfer.transfers.accountAmounts_count == 1 &&
                ctx.transaction.transactionFee == 1) {
                    // Verify Account Transaction
                    hedera_snprintf(
                        ctx.summary_line_1,
                        40,
                        "Verify %llu.%llu.%llu",
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.shardNum,
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.realmNum,
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.accountNum
                    );
                    hedera_snprintf(
                        ctx.amount,
                        DISPLAY_SIZE * 3,
                        "%s hbar",
                        hedera_format_tinybar(ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount)
                    );

            } else { // Number of Accounts == 2
                // Some other Transfer Transaction
                // Determine Sender based on amount
                ctx.transfer_to_index = 1;
                if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount > 0) {
                    ctx.transfer_to_index = 0;
                }
                hedera_snprintf(
                    ctx.summary_line_1,
                    40,
                    "Transfer to %llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.accountNum
                );
                hedera_snprintf(
                    ctx.amount,
                    DISPLAY_SIZE * 3,
                    "%s hbar",
                    hedera_format_tinybar(ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].amount)
                );
            }
        } break;

        default:
            // Unsupported
            THROW(EXCEPTION_MALFORMED_APDU);
    }

#if defined(TARGET_NANOS)
    setup_nanos_paging();
    UX_DISPLAY(ui_tx_summary_step, NULL);
#elif defined(TARGET_NANOX)
    ux_flow_init(0, ux_tx_flow, NULL);
#endif
}

// Sign Handler
// Decodes and handles transaction message
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
