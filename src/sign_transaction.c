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

#if defined(TARGET_NANOS)
static struct sign_tx_context_t {
    // ui common
    uint32_t key_index;
    uint8_t transfer_to_index;
    uint8_t transfer_from_index;

    // Transaction Summary
    char summary_line_1[DISPLAY_SIZE + 1];
    char summary_line_2[DISPLAY_SIZE + 1];
    char title[DISPLAY_SIZE + 1];
    
    // Account ID: uint64_t.uint64_t.uint64_t
    // Most other entities are shorter
    char full[ACCOUNT_ID_SIZE + 1];
    char partial[DISPLAY_SIZE + 1];
    
    // Steps correspond to parts of the transaction proto
    // type is set based on proto
    enum TransactionStep step;
    enum TransactionType type;

    uint8_t display_index;  // 1 -> Number Screens
    uint8_t display_count;  // Number Screens

    // Parsed transaction
    HederaTransactionBody transaction;
} ctx;

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

// Step 2 - 7: Operator, Senders, Recipients, Amount, Fee, Memo
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

// Step 8: Confirm
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

// Step 9: Deny
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
    unsigned int __attribute__ ((unused)) button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            if (ctx.type == Verify) {
                ctx.step = Senders;
                ctx.display_index = 1;
                reformat_senders();
            } else {
                ctx.step = Operator;
                ctx.display_index = 1;
                reformat_operator();
            }
            UX_DISPLAY(ui_tx_intermediate_step, NULL);
            break;
    }

    return 0;
}

void handle_intermediate_left_press() {
    // Navigate Left (scroll or return to previous step)
    switch (ctx.step) {
        case Operator: {
            if (first_screen()) {  // Return to Summary
                ctx.step = Summary;
                ctx.display_index = 1;
                UX_DISPLAY(ui_tx_summary_step, NULL);
            } else {  // Scroll Left
                ctx.display_index--;
                reformat_operator();
                UX_REDISPLAY();
            }
        } break;

        case Senders: {
            if (first_screen()) {  // Return to Operator
                if (ctx.type == Verify) {
                    ctx.step = Summary;
                    ctx.display_index = 1;
                    UX_DISPLAY(ui_tx_summary_step, NULL);
                } else {
                    ctx.step = Operator;
                    ctx.display_index = 1;
                    reformat_operator();
                }
            } else {  // Scroll Left
                ctx.display_index--;
                reformat_senders();
            }
            UX_REDISPLAY();
        } break;

        case Recipients: {
            if (first_screen()) {  // Return to Senders
                ctx.step = Senders;
                ctx.display_index = 1;
                reformat_senders();
            } else {  // Scroll Left
                ctx.display_index--;
                reformat_recipients();
            }
            UX_REDISPLAY();
        } break;

        case Amount: {
            if (first_screen()) {
                if (ctx.type == Create) {  // Return to Operator
                    ctx.step = Operator;
                    ctx.display_index = 1;
                    reformat_operator();
                } else if (ctx.type == Transfer) {  // Return to Recipients
                    ctx.step = Recipients;
                    ctx.display_index = 1;
                    reformat_recipients();
                }
            } else {  // Scroll left
                ctx.display_index--;
                reformat_amount();
            }
            UX_REDISPLAY();
        } break;

        case Fee: {
            if (first_screen()) {  // Return to Amount
                ctx.step = Amount;
                ctx.display_index = 1;
                reformat_amount();
            } else {  // Scroll left
                ctx.display_index--;
                reformat_fee();
            }
            UX_REDISPLAY();
        } break;

        case Memo: {
            if (first_screen()) {  // Return to Fee
                ctx.step = Fee;
                ctx.display_index = 1;
                reformat_fee();
            } else {  // Scroll Left
                ctx.display_index--;
                reformat_memo();
            }
            UX_REDISPLAY();
        } break;

        case Summary:
        case Confirm:
        case Deny:
            // ignore left button on Summary, Confirm, and Deny screens
            break;
    }
}

void handle_intermediate_right_press() {
    // Navigate Right (scroll or continue to next step)
    switch (ctx.step) {
        case Operator: {
            if (last_screen()) {
                if (ctx.type == Create) {  // Continue to Amount
                    ctx.step = Amount;
                    ctx.display_index = 1;
                    reformat_amount();
                } else {  // Continue to Senders
                    ctx.step = Senders;
                    ctx.display_index = 1;
                    reformat_senders();
                }
            } else {  // Scroll Right
                ctx.display_index++;
                reformat_operator();
            }
            UX_REDISPLAY();
        } break;

        case Senders: {
            if (last_screen()) {
                if (ctx.type == Verify) {  // Continue to Confirm
                    ctx.step = Confirm;
                    UX_DISPLAY(ui_tx_confirm_step, NULL);
                } else {  // Continue to Recipients
                    ctx.step = Recipients;
                    ctx.display_index = 1;
                    reformat_recipients();
                }
            } else {  // Scroll Right
                ctx.display_index++;
                reformat_senders();
            }
            UX_REDISPLAY();
        } break;

        case Recipients: {
            if (last_screen()) {  // Continue to Amount
                ctx.step = Amount;
                ctx.display_index = 1;
                reformat_amount();
            } else {  // Scroll Right
                ctx.display_index++;
                reformat_recipients();
            }
            UX_REDISPLAY();
        } break;

        case Amount: {
            if (last_screen()) {  // Continue to Fee
                ctx.step = Fee;
                ctx.display_index = 1;
                reformat_fee();
            } else {  // Scroll Right
                ctx.display_index++;
                reformat_amount();
            }
            UX_REDISPLAY();
        } break;

        case Fee: {
            if (last_screen()) {  // Continue to Memo
                ctx.step = Memo;
                ctx.display_index = 1;
                reformat_memo();
            } else {  // Scroll Right
                ctx.display_index++;
                reformat_fee();
            }
            UX_REDISPLAY();
        } break;

        case Memo: {
            if (last_screen()) {  // Continue to Confirm
                ctx.step = Confirm;
                ctx.display_index = 1;
                UX_DISPLAY(ui_tx_confirm_step, NULL);
            } else {  // Scroll Right
                ctx.display_index++;
                reformat_memo();
                UX_REDISPLAY();
            }
        } break;

        case Summary:
        case Confirm:
        case Deny:
            // ignore left button on Summary, Confirm, and Deny screens
            break;
    }
}

// Step 2 - 7: Operator, Senders, Recipients, Amount, Fee, Memo
unsigned int ui_tx_intermediate_step_button(
    unsigned int button_mask,
    unsigned int __attribute__ ((unused)) button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            handle_intermediate_left_press();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            handle_intermediate_right_press();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Skip to confirm screen
            ctx.step = Confirm;
            UX_DISPLAY(ui_tx_confirm_step, NULL);
            break;
    }

    return 0;
}

unsigned int ui_tx_confirm_step_button(
    unsigned int button_mask,
    unsigned int __attribute__ ((unused)) button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            if (ctx.type == Verify) {  // Return to Senders
                ctx.step = Senders;
                ctx.display_index = 1;
                reformat_senders();
            } else { // Return to Memo
                ctx.step = Memo;
                ctx.display_index = 1;
                reformat_memo();
            }
            UX_DISPLAY(ui_tx_intermediate_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Continue to Deny
            ctx.step = Deny;
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

unsigned int ui_tx_deny_step_button(
    unsigned int button_mask,
    unsigned int __attribute__ ((unused)) button_mask_counter
) {
    switch(button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Return to Confirm
            ctx.step = Confirm;
            UX_DISPLAY(ui_tx_confirm_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Reject
            ctx.step = Unknown;
            io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
            ui_idle();
            break;
    }

    return 0;
}

uint8_t num_screens(size_t length) {
    // Number of screens is len / display size + 1 for overflow
    if (length == 0) return 1;
    uint8_t screens = length / DISPLAY_SIZE;
    if (length % DISPLAY_SIZE > 0) screens += 1;
    return screens;
}

void count_screens() {
    ctx.display_count = num_screens(strlen(ctx.full));
}

void shift_display() {
    // Slide window (partial) along full entity (full) by DISPLAY_SIZE chars
    memset(ctx.partial, '\0', DISPLAY_SIZE + 1);
    memmove(
        ctx.partial,
        ctx.full + (DISPLAY_SIZE * (ctx.display_index - 1)),
        DISPLAY_SIZE
    );
}

bool last_screen() {
    return ctx.display_index == ctx.display_count;
}

bool first_screen() {
    return ctx.display_index == 1;
}

void reformat_operator() {
    hedera_snprintf(
        ctx.full,
        ACCOUNT_ID_SIZE,
        "%llu.%llu.%llu",
        ctx.transaction.transactionID.accountID.shardNum,
        ctx.transaction.transactionID.accountID.realmNum,
        ctx.transaction.transactionID.accountID.accountNum
    );

    count_screens();
    
    hedera_snprintf(
        ctx.title,
        DISPLAY_SIZE,
        "Operator (%u/%u)",
        ctx.display_index,
        ctx.display_count
    );

    shift_display();
}

void reformat_accounts(char* title_part, uint8_t transfer_index) {
    hedera_snprintf(
        ctx.full,
        ACCOUNT_ID_SIZE,
        "%llu.%llu.%llu",
        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[transfer_index].accountID.shardNum,
        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[transfer_index].accountID.realmNum,
        ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[transfer_index].accountID.accountNum
    );

    count_screens();

    hedera_snprintf(
        ctx.title,
        DISPLAY_SIZE,
        "%s (%u/%u)",
        title_part,
        ctx.display_index,
        ctx.display_count
    );
}

void reformat_senders() {
    if (ctx.type == Verify) {
        reformat_accounts("Account", 0);
    } else {
        reformat_accounts("Sender", ctx.transfer_from_index);
    }

    shift_display();
}

void reformat_recipients() {
    reformat_accounts("Recipient", ctx.transfer_to_index);
    shift_display();
}

void reformat_amount() {
    switch (ctx.type) {
        case Create:
            hedera_snprintf(
                ctx.full,
                DISPLAY_SIZE * 3,
                "%s hbar",
                hedera_format_tinybar(ctx.transaction.data.cryptoCreateAccount.initialBalance)
            );

            break;

        case Transfer:
            hedera_snprintf(
                ctx.full,
                DISPLAY_SIZE * 3,
                "%s hbar",
                hedera_format_tinybar(ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_to_index].amount)
            );

            break;

        case Unknown:
        case Verify:
            // no amount to format on Unknown and Verify screens
            return;
    }

    count_screens();

    hedera_snprintf(
        ctx.title,
        DISPLAY_SIZE,
        "%s (%u/%u)",
        ctx.type == Create ? "Balance" : "Amount",
        ctx.display_index,
        ctx.display_count
    );

    shift_display();
}

void reformat_fee() {
    hedera_snprintf(
        ctx.full,
        DISPLAY_SIZE * 3,
        "%s hbar",
        hedera_format_tinybar(ctx.transaction.transactionFee)
    );

    count_screens();

    hedera_snprintf(
        ctx.title,
        DISPLAY_SIZE,
        "Max Fee (%u/%u)",
        ctx.display_index,
        ctx.display_count
    );

    shift_display();
}

void reformat_memo() {
    hedera_snprintf(
        ctx.full,
        MAX_MEMO_SIZE,
        "%s",
        ctx.transaction.memo
    );

    if (strlen(ctx.full) > MAX_MEMO_SIZE) {
        // :grimacing:
        THROW(EXCEPTION_MALFORMED_APDU); 
    }

    count_screens();

    hedera_snprintf(
        ctx.title,
        DISPLAY_SIZE,
        "Memo (%u/%u)",
        ctx.display_index,
        ctx.display_count
    );

    shift_display();
}

void handle_transaction_body() {
    memset(ctx.summary_line_1, '\0', DISPLAY_SIZE + 1);
    memset(ctx.summary_line_2, '\0', DISPLAY_SIZE + 1);
    memset(ctx.full, '\0', ACCOUNT_ID_SIZE + 1);
    memset(ctx.partial, '\0', DISPLAY_SIZE + 1);

    // Step 1, Unknown Type, Screen 1 of 1
    ctx.step = Summary;
    ctx.type = Unknown;
    ctx.display_index = 1;
    ctx.display_count = 1;

    // <Do Action> 
    // with Key #X?
    hedera_snprintf(
        ctx.summary_line_2,
        DISPLAY_SIZE,
        "with Key #%u?",
        ctx.key_index
    );

    // Handle parsed protobuf message of transaction body
    switch (ctx.transaction.which_data) {
        case HederaTransactionBody_cryptoCreateAccount_tag:
            // Create Account Transaction
            ctx.type = Create;
            hedera_snprintf(
                ctx.summary_line_1,
                DISPLAY_SIZE,
                "Create Account"
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
                    ctx.type = Verify;
                    hedera_snprintf(
                        ctx.summary_line_1,
                        DISPLAY_SIZE,
                        "Verify Account"
                    );

            } else { // Number of Accounts == 2
                // Some other Transfer Transaction
                // Determine Sender based on amount
                ctx.type = Transfer;

                hedera_snprintf(
                    ctx.summary_line_1,
                    DISPLAY_SIZE,
                    "Transfer"
                );

                ctx.transfer_to_index = 1;
                ctx.transfer_from_index = 0;
                if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount > 0) {
                    ctx.transfer_to_index = 0;
                    ctx.transfer_from_index = 1;
                }
            }
        } break;

        default:
            // Unsupported
            THROW(EXCEPTION_MALFORMED_APDU);
    }

    UX_DISPLAY(ui_tx_summary_step, NULL);
}

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

static struct sign_tx_context_t {
    // ui common
    uint32_t key_index;
    uint8_t transfer_from_index;
    uint8_t transfer_to_index;

    // Transaction Summary
    char summary_line_1[DISPLAY_SIZE + 1];
    char summary_line_2[DISPLAY_SIZE + 1];
    char senders_title[DISPLAY_SIZE + 1];
    char amount_title[DISPLAY_SIZE + 1];
    char partial[DISPLAY_SIZE + 1];

    enum TransactionType type;

    // Transaction Operator
    char operator[DISPLAY_SIZE * 2 + 1];

    // Transaction Senders
    char senders[DISPLAY_SIZE * 2 + 1];

    // Transaction Recipients
    char recipients[DISPLAY_SIZE * 2 + 1];

    // Transaction Amount
    char amount[DISPLAY_SIZE * 2 + 1];
    
    // Transaction Fee
    char fee[DISPLAY_SIZE * 2 + 1];

    // Transaction Memo
    char memo[MAX_MEMO_SIZE + 1];

    // Parsed transaction
    HederaTransactionBody transaction;
} ctx;

// UI Definition for Nano X

// Confirm Callback
unsigned int io_seproxyhal_tx_approve(const bagl_element_t* e) {
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

UX_STEP_NOCB(
    ux_tx_flow_1_step,
    bnn,
    {
        "Transaction Summary",
        ctx.summary_line_1,
        ctx.summary_line_2
    }
);

UX_STEP_NOCB(
    ux_tx_flow_2_step,
    bnnn_paging,
    {
        .title = "Operator",
        .text = (char*) ctx.operator
    }
);

UX_STEP_NOCB(
    ux_tx_flow_3_step,
    bnnn_paging,
    {
        .title = (char*) ctx.senders_title,
        .text = (char*) ctx.senders
    }
);

UX_STEP_NOCB(
    ux_tx_flow_4_step,
    bnnn_paging,
    {
        .title = "Recipient",
        .text = (char*) ctx.recipients
    }
);

UX_STEP_NOCB(
    ux_tx_flow_5_step,
    bnnn_paging,
    {
        .title = (char*) ctx.amount_title,
        .text = (char*) ctx.amount
    }
);

UX_STEP_NOCB(
    ux_tx_flow_6_step,
    bnnn_paging,
    {
        .title = "Max Fee",
        .text = (char*) ctx.fee
    }
);

UX_STEP_NOCB(
    ux_tx_flow_7_step,
    bnnn_paging,
    {
        .title = "Memo",
        .text = (char*) ctx.memo
    }
);

UX_STEP_VALID(
    ux_tx_flow_8_step,
    pb,
    io_seproxyhal_tx_approve(NULL),
    {
        &C_icon_validate_14,
        "Confirm"
    }
);

UX_STEP_VALID(
    ux_tx_flow_9_step,
    pb,
    io_seproxyhal_tx_reject(NULL),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

// Transfer UX Flow
UX_DEF(
    ux_transfer_flow,
    &ux_tx_flow_1_step,
    &ux_tx_flow_2_step,
    &ux_tx_flow_3_step,
    &ux_tx_flow_4_step,
    &ux_tx_flow_5_step,
    &ux_tx_flow_6_step,
    &ux_tx_flow_7_step,
    &ux_tx_flow_8_step,
    &ux_tx_flow_9_step
);

// Create UX Flow
UX_DEF(
    ux_create_flow,
    &ux_tx_flow_1_step,
    &ux_tx_flow_2_step,
    &ux_tx_flow_5_step,
    &ux_tx_flow_6_step,
    &ux_tx_flow_7_step,
    &ux_tx_flow_8_step,
    &ux_tx_flow_9_step
);

// Verify UX Flow
UX_DEF(
    ux_verify_flow,
    &ux_tx_flow_1_step,
    &ux_tx_flow_3_step,
    &ux_tx_flow_8_step,
    &ux_tx_flow_9_step
);

void handle_transaction_body() {
    memset(ctx.summary_line_1, '\0', DISPLAY_SIZE + 1);
    memset(ctx.summary_line_2, '\0', DISPLAY_SIZE + 1);
    memset(ctx.amount_title, '\0', DISPLAY_SIZE + 1);
    memset(ctx.senders_title, '\0', DISPLAY_SIZE + 1);
    memset(ctx.operator, '\0', DISPLAY_SIZE * 2 + 1);
    memset(ctx.senders, '\0', DISPLAY_SIZE * 2 + 1);
    memset(ctx.recipients, '\0', DISPLAY_SIZE * 2 + 1);
    memset(ctx.fee, '\0', DISPLAY_SIZE * 2 + 1);
    memset(ctx.amount, '\0', DISPLAY_SIZE * 2 + 1);
    memset(ctx.memo, '\0', MAX_MEMO_SIZE + 1);

    ctx.type = Unknown;

    // <Do Action> 
    // with Key #X?
    hedera_snprintf(
        ctx.summary_line_2,
        DISPLAY_SIZE,
        "with Key #%u?",
        ctx.key_index
    );

    hedera_snprintf(
        ctx.operator,
        DISPLAY_SIZE * 2,
        "%llu.%llu.%llu",
        ctx.transaction.transactionID.accountID.shardNum,
        ctx.transaction.transactionID.accountID.realmNum,
        ctx.transaction.transactionID.accountID.accountNum
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

    hedera_sprintf(
        ctx.amount_title,
        "Amount"
    );

    hedera_sprintf(
        ctx.senders_title,
        "Sender"
    );

    // Handle parsed protobuf message of transaction body
    switch (ctx.transaction.which_data) {
        case HederaTransactionBody_cryptoCreateAccount_tag:
            ctx.type = Create;
            // Create Account Transaction
            hedera_sprintf(
                ctx.summary_line_1,
                "Create Account"
            );
            hedera_sprintf(
                ctx.amount_title,
                "Balance"
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
                    ctx.type = Verify;
                    hedera_sprintf(
                        ctx.summary_line_1,
                        "Verify Account"
                    );
                    hedera_sprintf(
                        ctx.senders_title,
                        "Account"
                    );
                    hedera_snprintf(
                        ctx.senders,
                        DISPLAY_SIZE * 2,
                        "%llu.%llu.%llu",
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
                ctx.type = Transfer;
                hedera_sprintf(
                    ctx.summary_line_1,
                    "Transfer"
                );

                ctx.transfer_from_index = 0;
                ctx.transfer_to_index = 1;
                if (ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount > 0) {
                    ctx.transfer_from_index = 1;
                    ctx.transfer_to_index = 0;
                }

                hedera_snprintf(
                    ctx.senders,
                    DISPLAY_SIZE * 2,
                    "%llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_from_index].accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_from_index].accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[ctx.transfer_from_index].accountID.accountNum
                );
                hedera_snprintf(
                    ctx.recipients,
                    DISPLAY_SIZE * 2,
                    "%llu.%llu.%llu",
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
            break;
    }

    switch (ctx.type) {
        case Verify:
            ux_flow_init(0, ux_verify_flow, NULL);
            break;
        case Create:
            ux_flow_init(0, ux_create_flow, NULL);
            break;
        case Transfer:
            ux_flow_init(0, ux_transfer_flow, NULL);
            break;
    }
}
#endif

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
    memmove(raw_transaction, (buffer + 4), raw_transaction_length);

    // Sign Transaction
    // TODO: handle error return here (internal error?!)
    if (!hedera_sign(
        ctx.key_index,
        raw_transaction,
        raw_transaction_length,
        G_io_apdu_buffer
    )) {
        THROW(EXCEPTION_INTERNAL);
    }

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
