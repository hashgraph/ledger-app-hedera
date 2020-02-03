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
    char summary_line_1[DISPLAY_SIZE + 1];
    char summary_line_2[DISPLAY_SIZE + 1];
    char title[DISPLAY_SIZE + 1];
    char partial[DISPLAY_SIZE + 1];
    uint8_t step;
    bool verify;
    
    // Transaction Amount
    char amount[DISPLAY_SIZE * 2 + 1];
    uint8_t amount_display_index;  // current
    uint8_t amount_display_count;  // total
    
    // Transaction Fee
    char fee[DISPLAY_SIZE * 2 + 1];
    uint8_t fee_display_index;  // current
    uint8_t fee_display_count;  // total

    // Transaction Memo
    char memo[MAX_MEMO_SIZE + 1];
    uint8_t memo_display_index;
    uint8_t memo_display_count;

    // Parsed transaction
    HederaTransactionBody transaction;
} ctx;

#if defined(TARGET_NANOS)
// UI Definition for Nano S
// Step 1: Transaction Summary
static const bagl_element_t ui_tx_summary_step[] = {
    UI_BACKGROUND(),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),
    UI_ICON_LEFT(LEFT_ICON_ID_VERIFY, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(RIGHT_ICON_ID_VERIFY, BAGL_GLYPH_ICON_CHECK),

    // ()       >>
    // Line 1
    // Line 2

    UI_TEXT(LINE_1_ID, 0, 12, 128, ctx.summary_line_1),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.summary_line_2)
};

// Step 2 - 4: Amount, Fee, Memo
static const bagl_element_t ui_tx_intermediate_step[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // <<       >>
    // <Title>
    // <Partial>

    UI_TEXT(LINE_1_ID, 0, 12, 128, ctx.title),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.partial)
};

// Step 5: Confirm
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

// Step 6: Deny
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
// For Verify Transactions, one step only
static const bagl_element_t* ui_prepro_tx_summary_step(
    const bagl_element_t* element
) {
    if (element->component.userid == LEFT_ICON_ID_VERIFY 
        && ctx.verify == false)
        return NULL; // Hide Reject on Non-Verify transactions
    if (element->component.userid == RIGHT_ICON_ID_VERIFY 
        && ctx.verify == false) 
        return NULL; // Hide Accept on Non-Verify transactions
    if (element->component.userid == RIGHT_ICON_ID
        && ctx.verify == true)
        return NULL; // Hide Next on Verify transactions
    return element;
}

// Step 1: Transaction Summary
unsigned int ui_tx_summary_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            if (!ctx.verify) {
                ctx.step = 2;
                ctx.amount_display_index = 1;
                reformat_amount();
                UX_DISPLAY(ui_tx_intermediate_step, NULL);
            } else {
                // verify account transaction
                io_exchange_with_code(EXCEPTION_OK, 64);
                ui_idle();
            }
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            if (ctx.verify) {
                // verify account transaction
                io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
                ui_idle();
            }
            break;
    }

    return 0;
}

// Step 2 - 4 : Amount, Fee, Memo
// Was separate BAGL stacks, but ran out of memory
unsigned int ui_tx_intermediate_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Left Button released
            if (ctx.step == 2 && ctx.amount_display_index == 1) {
                // First screen of amount step
                UX_DISPLAY(ui_tx_summary_step, NULL);
            } else if (ctx.step == 2 && ctx.amount_display_index > 1) {
                // Not the first screen of amount step
                ctx.amount_display_index--;
                reformat_amount();
                UX_REDISPLAY();
            } else if (ctx.step == 3 && ctx.fee_display_index == 1) {
                // First screen of fee step
                ctx.step = 2;
                ctx.amount_display_index = 1;
                reformat_amount();
                UX_REDISPLAY();
            } else if (ctx.step == 3 && ctx.fee_display_index > 1) {
                // Not the first screen of the fee step
                ctx.fee_display_index--;
                reformat_fee();
                UX_REDISPLAY();
            } else if (ctx.step == 4 && ctx.memo_display_index == 1) {
                // First screen of the memo step
                ctx.step = 3;
                ctx.fee_display_index = 1;
                reformat_fee();
                UX_REDISPLAY();
            } else if (ctx.step == 4 && ctx.memo_display_index > 1) {
                // Not the first scren of the memo step
                ctx.memo_display_index--;
                reformat_memo();
                UX_REDISPLAY();
            }
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Right button released
            if (ctx.step == 2 && ctx.amount_display_index == ctx.amount_display_count) {
                // last screen of the amount step
                ctx.step = 3;
                ctx.fee_display_index = 1;
                reformat_fee();
                UX_REDISPLAY();
            } else if (ctx.step == 2 && ctx.amount_display_index < ctx.amount_display_count) {
                // not the last screen of the amount step
                ctx.amount_display_index++;
                reformat_amount();
                UX_REDISPLAY();
            } else if (ctx.step == 3 && ctx.fee_display_index == ctx.fee_display_count) {
                // last screen of the fee step
                ctx.step = 4;
                ctx.memo_display_index = 1;
                reformat_memo();
                UX_REDISPLAY();
            } else if (ctx.step == 3 && ctx.fee_display_index < ctx.fee_display_count) {
                // not the last screen of the fee step
                ctx.fee_display_index++;
                reformat_fee();
                UX_REDISPLAY();
            } else if (ctx.step == 4 && ctx.memo_display_index == ctx.memo_display_count) {
                // last screen of the memo step
                UX_DISPLAY(ui_tx_confirm_step, NULL);
            } else if (ctx.step == 4 && ctx.memo_display_index < ctx.memo_display_count) {
                // not the last screen of the memo step
                ctx.memo_display_index++;
                reformat_memo();
                UX_REDISPLAY();
            }
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Skip to confirm screen
            UX_DISPLAY(ui_tx_confirm_step, NULL);
            break;
    }

    return 0;
}

// Step 5: Confirm
unsigned int ui_tx_confirm_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Return to Memo (and other steps)
            ctx.step = 4;
            ctx.memo_display_index = 1;
            reformat_memo();
            UX_DISPLAY(ui_tx_intermediate_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Continue to Deny
            UX_DISPLAY(ui_tx_deny_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Exchange Signature (OK)
            io_exchange_with_code(EXCEPTION_OK, 64);
            ui_idle();
            break;
    }

    return 0;
}

// Step 6: Deny
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
        ctx.title,
        DISPLAY_SIZE, 
        "Amount (%u/%u)",
        ctx.amount_display_index,
        ctx.amount_display_count
    );
    os_memset(ctx.partial, '\0', DISPLAY_SIZE + 1);
    os_memmove(
        ctx.partial,
        ctx.amount + (DISPLAY_SIZE * (ctx.amount_display_index - 1)),
        DISPLAY_SIZE
    );
}

void reformat_fee() {
    hedera_snprintf(
        ctx.title,
        DISPLAY_SIZE,
        "Fee (%u/%u)",
        ctx.fee_display_index,
        ctx.fee_display_count
    );
    os_memset(ctx.partial, '\0', DISPLAY_SIZE + 1);
    os_memmove(
        ctx.partial,
        ctx.fee + (DISPLAY_SIZE * (ctx.fee_display_index - 1)),
        DISPLAY_SIZE
    );
}

void reformat_memo() {
    hedera_snprintf(
        ctx.title,
        DISPLAY_SIZE,
        "Memo (%u/%u)",
        ctx.memo_display_index,
        ctx.memo_display_count
    );
    os_memset(ctx.partial, '\0', DISPLAY_SIZE + 1);
    os_memmove(
        ctx.partial,
        ctx.memo + (DISPLAY_SIZE * (ctx.memo_display_index - 1)),
        DISPLAY_SIZE
    );
}

uint8_t num_screens(size_t length) {
    if (length == 0) return 1;
    uint8_t screens = length / DISPLAY_SIZE;
    if (length % DISPLAY_SIZE > 0) screens += 1;
    return screens;
}

void setup_nanos_paging() {
    ctx.amount_display_index = 1;
    ctx.amount_display_count = num_screens(strlen(ctx.amount));
    ctx.fee_display_index = 1;
    ctx.fee_display_count = num_screens(strlen(ctx.fee));
    ctx.memo_display_index = 1;
    ctx.memo_display_count = num_screens(strlen(ctx.memo));
    reformat_amount();
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

UX_STEP_NOCB(
    ux_tx_flow_4_step,
    bnnn_paging,
    {
        .title = "Memo",
        .text = (char*) ctx.memo
    }
);

// Step 5: Confirm
UX_STEP_VALID(
    ux_tx_flow_5_step,
    pbb,
    io_seproxyhal_tx_approve(NULL),
    {
        &C_icon_validate_14,
        "Confirm"
    }
);

// Step 6: Reject
UX_STEP_VALID(
    ux_tx_flow_6_step,
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
    &ux_tx_flow_5_step,
    &ux_tx_flow_6_step
);

// Verify UX Flow
UX_DEF(
    ux_verify_flow,
    &ux_tx_flow_1_step,
    &ux_tx_flow_5_step,
    &ux_tx_flow_6_step
);

#endif

void handle_transaction_body() {
    os_memset(ctx.summary_line_1, '\0', DISPLAY_SIZE + 1);
    os_memset(ctx.summary_line_2, '\0', DISPLAY_SIZE + 1);
    os_memset(ctx.fee, '\0', DISPLAY_SIZE * 2 + 1);
    os_memset(ctx.amount, '\0', DISPLAY_SIZE * 2 + 1);
    os_memset(ctx.memo, '\0', MAX_MEMO_SIZE + 1);

    ctx.verify = false;

    // <Do Action> 
    // with Key #X?
    hedera_snprintf(
        ctx.summary_line_2,
        DISPLAY_SIZE,
        "with Key #%u?",
        ctx.key_index
    );

    hedera_snprintf(
        ctx.fee,
        DISPLAY_SIZE * 2,
        "%s hbar",
        hedera_format_tinybar(ctx.transaction.transactionFee)
    );

    hedera_snprintf(
        ctx.memo,
        MAX_MEMO_SIZE,
        "%s",
        ctx.transaction.memo
    );

    // Handle parsed protobuf message of transaction body
    switch (ctx.transaction.which_data) {
        case HederaTransactionBody_cryptoCreateAccount_tag:
            // Create Account Transaction
            hedera_snprintf(
                ctx.summary_line_1,
                DISPLAY_SIZE,
                "Create Account"
            );
            hedera_snprintf(
                ctx.amount,
                DISPLAY_SIZE * 2,
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
                    ctx.verify = true;
                    hedera_snprintf(
                        ctx.summary_line_1,
                        DISPLAY_SIZE,
                        "Verify %llu.%llu.%llu",
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.shardNum,
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.realmNum,
                        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.accountNum
                    );
                    hedera_snprintf(
                        ctx.amount,
                        DISPLAY_SIZE * 2,
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
                    DISPLAY_SIZE,
                    "Send to %llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].accountID.accountNum
                );
                hedera_snprintf(
                    ctx.amount,
                    DISPLAY_SIZE * 2,
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
    UX_DISPLAY(ui_tx_summary_step, ui_prepro_tx_summary_step);
#elif defined(TARGET_NANOX)
    if (ctx.verify) {
        ux_flow_init(0, ux_verify_flow, NULL);
    } else {
        ux_flow_init(0, ux_tx_flow, NULL);
    }
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
    
    // Raw Tx
    uint8_t raw_transaction[MAX_TX_SIZE];
    int raw_transaction_length = len - 4;

    // Oops Oof Owie
    if (raw_transaction_length > MAX_TX_SIZE) {
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    // copy raw transaction
    os_memmove(raw_transaction, (buffer + 4), raw_transaction_length);

    // Sign Transaction
    hedera_sign(
        ctx.key_index,
        raw_transaction,
        raw_transaction_length,
        G_io_apdu_buffer
    );

    // Make in memory buffer into stream
    pb_istream_t stream = pb_istream_from_buffer(
        raw_transaction, 
        raw_transaction_length
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
