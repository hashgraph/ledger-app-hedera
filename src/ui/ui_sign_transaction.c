#include <pb.h>
#include <pb_decode.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "TransactionBody.pb.h"
#include "debug.h"
#include "globals.h"
#include "glyphs.h"
#include "handlers.h"
#include "hedera.h"
#include "hedera_format.h"
#include "io.h"
#include "printf.h"
#include "sign_transaction.h"
#include "src/errors.h"
#include "ui_common.h"
#include "ui_flows.h"
#include "utils.h"
#include "ux.h"

#if defined(TARGET_NANOS)

static uint8_t num_screens(size_t length) {
    // Number of screens is len / display size + 1 for overflow
    if (length == 0) return 1;

    uint8_t screens = length / DISPLAY_SIZE;

    if (length % DISPLAY_SIZE > 0) {
        screens += 1;
    }

    return screens;
}

static void update_display_count(void) {
    st_ctx.display_count = num_screens(strlen(st_ctx.full));
}

static void shift_display(void) {
    // Slide window (partial) along full entity (full) by DISPLAY_SIZE chars
    MEMCLEAR(st_ctx.partial);
    memmove(st_ctx.partial,
            st_ctx.full + (DISPLAY_SIZE * (st_ctx.display_index - 1)),
            DISPLAY_SIZE);
}

static void reformat_senders(void) {
    switch (st_ctx.type) {
        case Verify:
            reformat_verify_account();
            break;

        case Associate:
            reformat_token_associate();
            break;

        case TokenMint:
            reformat_token_mint();
            break;

        case TokenBurn:
            reformat_token_burn();
            break;

        case TokenTransfer:
            reformat_tokens_account_sender();
            break;

        case Transfer:
            reformat_sender_account();
            break;

        default:
            return;
    }
}

static void reformat_recipients(void) {
    switch (st_ctx.type) {
        case TokenTransfer:
            reformat_tokens_account_recipient();
            break;

        case Transfer:
            reformat_recipient_account();
            break;

        default:
            return;
    }
}

static void reformat_amount(void) {
    switch (st_ctx.type) {
        case Create:
            reformat_amount_balance();
            break;

        case Transfer:
            reformat_amount_transfer();
            break;

        case TokenMint:
            reformat_amount_mint();
            break;

        case TokenBurn:
            reformat_amount_burn();
            break;

        case TokenTransfer:
            reformat_token_tranfer();
            break;

        default:
            return;
    }
}

// Forward declarations for Nano S UI
// Step 1
unsigned int ui_tx_summary_step_button(unsigned int button_mask,
                                       unsigned int button_mask_counter);

// Step 2 - 7
void handle_intermediate_left_press();
void handle_intermediate_right_press();
unsigned int ui_tx_intermediate_step_button(unsigned int button_mask,
                                            unsigned int button_mask_counter);

// Step 8
unsigned int ui_tx_confirm_step_button(unsigned int button_mask,
                                       unsigned int button_mask_counter);

// Step 9
unsigned int ui_tx_deny_step_button(unsigned int button_mask,
                                    unsigned int button_mask_counter);

// UI Definition for Nano S
// Step 1: Transaction Summary
static const bagl_element_t ui_tx_summary_step[] = {
    UI_BACKGROUND(), UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // ()       >>
    // Line 1
    // Line 2

    UI_TEXT(LINE_1_ID, 0, 12, 128, st_ctx.summary_line_1),
    UI_TEXT(LINE_2_ID, 0, 26, 128, st_ctx.summary_line_2)};

// Step 2 - 7: Operator, Senders, Recipients, Amount, Fee, Memo
static const bagl_element_t ui_tx_intermediate_step[] = {
    UI_BACKGROUND(), UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // <<       >>
    // <Title>
    // <Partial>

    UI_TEXT(LINE_1_ID, 0, 12, 128, st_ctx.title),
    UI_TEXT(LINE_2_ID, 0, 26, 128, st_ctx.partial)};

// Step 8: Confirm
static const bagl_element_t ui_tx_confirm_step[] = {
    UI_BACKGROUND(), UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // <<       >>
    //    Confirm
    //    <Check>

    UI_TEXT(LINE_1_ID, 0, 12, 128, "Confirm"),
    UI_ICON(LINE_2_ID, 0, 24, 128, BAGL_GLYPH_ICON_CHECK)};

// Step 9: Deny
static const bagl_element_t ui_tx_deny_step[] = {
    UI_BACKGROUND(), UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),

    // <<       ()
    //    Deny
    //      X

    UI_TEXT(LINE_1_ID, 0, 12, 128, "Deny"),
    UI_ICON(LINE_2_ID, 0, 24, 128, BAGL_GLYPH_ICON_CROSS)};

// Step 1: Transaction Summary
unsigned int ui_tx_summary_step_button(unsigned int button_mask,
                                       unsigned int __attribute__((unused))
                                       button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            if (st_ctx.type == Verify || st_ctx.type == Associate ||
                st_ctx.type == TokenMint || st_ctx.type == TokenBurn) {
                st_ctx.step = Senders;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_senders();
                shift_display();
            } else {
                st_ctx.step = Operator;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_operator();
                shift_display();
            }
            UX_DISPLAY(ui_tx_intermediate_step, NULL);
            break;
    }

    return 0;
}

bool first_screen() { return st_ctx.display_index == 1; }

bool last_screen() { return st_ctx.display_index == st_ctx.display_count; }

void handle_intermediate_left_press() {
    // Navigate Left (scroll or return to previous step)
    switch (st_ctx.step) {
        case Operator: {
            if (first_screen()) { // Return to Summary
                st_ctx.step = Summary;
                st_ctx.display_index = 1;
                UX_DISPLAY(ui_tx_summary_step, NULL);
            } else { // Scroll Left
                st_ctx.display_index--;
                update_display_count();
                reformat_operator();
                shift_display();
                UX_REDISPLAY();
            }
        } break;

        case Senders: {
            if (first_screen()) { // Return to Operator
                if (st_ctx.type == Verify || st_ctx.type == Associate ||
                    st_ctx.type == TokenMint || st_ctx.type == TokenBurn) {
                    st_ctx.step = Summary;
                    st_ctx.display_index = 1;
                    UX_DISPLAY(ui_tx_summary_step, NULL);
                } else {
                    st_ctx.step = Operator;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_operator();
                    shift_display();
                }
            } else { // Scroll Left
                st_ctx.display_index--;
                update_display_count();
                reformat_senders();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Recipients: {
            if (first_screen()) { // Return to Senders
                st_ctx.step = Senders;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_senders();
                shift_display();
            } else { // Scroll Left
                st_ctx.display_index--;
                update_display_count();
                reformat_recipients();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Amount: {
            if (first_screen()) {
                if (st_ctx.type == Create) { // Return to Operator
                    st_ctx.step = Operator;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_operator();
                    shift_display();
                } else if (st_ctx.type == Transfer ||
                           st_ctx.type ==
                               TokenTransfer) { // Return to Recipients
                    st_ctx.step = Recipients;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_recipients();
                    shift_display();
                } else if (st_ctx.type == TokenMint ||
                           st_ctx.type == TokenBurn) { // Return to Senders
                    st_ctx.step = Senders;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_senders();
                    shift_display();
                }
            } else { // Scroll left
                st_ctx.display_index--;
                update_display_count();
                reformat_amount();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Fee: {
            if (first_screen()) { // Return to Amount
                st_ctx.step = Amount;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_amount();
                shift_display();
            } else { // Scroll left
                st_ctx.display_index--;
                update_display_count();
                reformat_fee();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Memo: {
            if (first_screen()) { // Return to Fee
                st_ctx.step = Fee;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_fee();
                shift_display();
            } else { // Scroll Left
                st_ctx.display_index--;
                update_display_count();
                reformat_memo();
                shift_display();
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
    switch (st_ctx.step) {
        case Operator: {
            if (last_screen()) {
                if (st_ctx.type == Create) { // Continue to Amount
                    st_ctx.step = Amount;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_amount();
                    shift_display();
                } else { // Continue to Senders
                    st_ctx.step = Senders;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_senders();
                    shift_display();
                }
            } else { // Scroll Right
                st_ctx.display_index++;
                update_display_count();
                reformat_operator();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Senders: {
            if (last_screen()) {
                if (st_ctx.type == Verify ||
                    st_ctx.type == Associate) { // Continue to Confirm
                    st_ctx.step = Confirm;
                    UX_DISPLAY(ui_tx_confirm_step, NULL);
                } else if (st_ctx.type == TokenMint ||
                           st_ctx.type == TokenBurn) {
                    st_ctx.step = Amount;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_amount();
                    shift_display();
                } else { // Continue to Recipients
                    st_ctx.step = Recipients;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_recipients();
                    shift_display();
                }
            } else { // Scroll Right
                st_ctx.display_index++;
                update_display_count();
                reformat_senders();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Recipients: {
            if (last_screen()) { // Continue to Amount
                st_ctx.step = Amount;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_amount();
                shift_display();
            } else { // Scroll Right
                st_ctx.display_index++;
                update_display_count();
                reformat_recipients();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Amount: {
            if (last_screen()) {
                if (st_ctx.type == TokenMint || st_ctx.type == TokenBurn) {
                    // Continue to Confirm
                    st_ctx.step = Confirm;
                    st_ctx.display_index = 1;
                    UX_DISPLAY(ui_tx_confirm_step, NULL);
                } else {
                    // Continue to Fee
                    st_ctx.step = Fee;
                    st_ctx.display_index = 1;
                    update_display_count();
                    reformat_fee();
                    shift_display();
                }
            } else { // Scroll Right
                st_ctx.display_index++;
                update_display_count();
                reformat_amount();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Fee: {
            if (last_screen()) { // Continue to Memo
                st_ctx.step = Memo;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_memo();
                shift_display();
            } else { // Scroll Right
                st_ctx.display_index++;
                update_display_count();
                reformat_fee();
                shift_display();
            }
            UX_REDISPLAY();
        } break;

        case Memo: {
            if (last_screen()) { // Continue to Confirm
                st_ctx.step = Confirm;
                st_ctx.display_index = 1;
                UX_DISPLAY(ui_tx_confirm_step, NULL);
            } else { // Scroll Right
                st_ctx.display_index++;
                update_display_count();
                reformat_memo();
                shift_display();
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
unsigned int ui_tx_intermediate_step_button(unsigned int button_mask,
                                            unsigned int __attribute__((unused))
                                            button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            handle_intermediate_left_press();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            handle_intermediate_right_press();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Skip to confirm screen
            st_ctx.step = Confirm;
            UX_DISPLAY(ui_tx_confirm_step, NULL);
            break;
    }

    return 0;
}

unsigned int ui_tx_confirm_step_button(unsigned int button_mask,
                                       unsigned int __attribute__((unused))
                                       button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            if (st_ctx.type == Verify ||
                st_ctx.type == Associate) { // Return to Senders
                st_ctx.step = Senders;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_senders();
                shift_display();
            } else if (st_ctx.type == TokenMint ||
                       st_ctx.type == TokenBurn) { // Return to Amount
                st_ctx.step = Amount;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_amount();
                shift_display();
            } else { // Return to Memo
                st_ctx.step = Memo;
                st_ctx.display_index = 1;
                update_display_count();
                reformat_memo();
                shift_display();
            }
            UX_DISPLAY(ui_tx_intermediate_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Continue to Deny
            st_ctx.step = Deny;
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

unsigned int ui_tx_deny_step_button(unsigned int button_mask,
                                    unsigned int __attribute__((unused))
                                    button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Return to Confirm
            st_ctx.step = Confirm;
            UX_DISPLAY(ui_tx_confirm_step, NULL);
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            // Reject
            st_ctx.step = Unknown;
            io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
            ui_idle();
            break;
    }

    return 0;
}

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

// UI Definition for Nano X

// Confirm Callback
unsigned int io_seproxyhal_tx_approve(const bagl_element_t* e) {
    UNUSED(e);
    io_exchange_with_code(EXCEPTION_OK, 64);
    ui_idle();
    return 0;
}

// Reject Callback
unsigned int io_seproxyhal_tx_reject(const bagl_element_t* e) {
    UNUSED(e);
    io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
    ui_idle();
    return 0;
}

UX_STEP_NOCB(ux_tx_flow_1_step, bnn,
             {"Transaction Summary", st_ctx.summary_line_1,
              st_ctx.summary_line_2});

UX_STEP_NOCB(
    ux_tx_flow_2_step,
    bnnn_paging,
    {
        .title = "Operator",
        .text = (char*) st_ctx.operator
    }
);

UX_STEP_NOCB(ux_tx_flow_3_step, bnnn_paging,
             {.title = (char*)st_ctx.senders_title,
              .text = (char*)st_ctx.senders});

UX_STEP_NOCB(ux_tx_flow_4_step, bnnn_paging,
             {.title = "Recipient", .text = (char*)st_ctx.recipients});

UX_STEP_NOCB(ux_tx_flow_5_step, bnnn_paging,
             {.title = (char*)st_ctx.amount_title,
              .text = (char*)st_ctx.amount});

UX_STEP_NOCB(ux_tx_flow_6_step, bnnn_paging,
             {.title = "Max Fee", .text = (char*)st_ctx.fee});

UX_STEP_NOCB(ux_tx_flow_7_step, bnnn_paging,
             {.title = "Memo", .text = (char*)st_ctx.memo});

UX_STEP_VALID(ux_tx_flow_8_step, pb, io_seproxyhal_tx_approve(NULL),
              {&C_icon_validate_14, "Confirm"});

UX_STEP_VALID(ux_tx_flow_9_step, pb, io_seproxyhal_tx_reject(NULL),
              {&C_icon_crossmark, "Reject"});

// Transfer UX Flow
UX_DEF(ux_transfer_flow, &ux_tx_flow_1_step, &ux_tx_flow_2_step,
       &ux_tx_flow_3_step, &ux_tx_flow_4_step, &ux_tx_flow_5_step,
       &ux_tx_flow_6_step, &ux_tx_flow_7_step, &ux_tx_flow_8_step,
       &ux_tx_flow_9_step);

// Create UX Flow
UX_DEF(ux_create_flow, &ux_tx_flow_1_step, &ux_tx_flow_2_step,
       &ux_tx_flow_5_step, &ux_tx_flow_6_step, &ux_tx_flow_7_step,
       &ux_tx_flow_8_step, &ux_tx_flow_9_step);

// Verify UX Flow
UX_DEF(ux_verify_flow, &ux_tx_flow_1_step, &ux_tx_flow_3_step,
       &ux_tx_flow_8_step, &ux_tx_flow_9_step);

// Burn/Mint UX Flow
UX_DEF(ux_burn_mint_flow, &ux_tx_flow_1_step, &ux_tx_flow_3_step,
       &ux_tx_flow_5_step, &ux_tx_flow_8_step, &ux_tx_flow_9_step);

#endif

void ui_sign_transaction(void) {
#if defined(TARGET_NANOS)

    UX_DISPLAY(ui_tx_summary_step, NULL);

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

    switch (st_ctx.type) {
        case Associate:
        case Verify:
            ux_flow_init(0, ux_verify_flow, NULL);
            break;
        case Create:
            ux_flow_init(0, ux_create_flow, NULL);
            break;
        case TokenTransfer:
        case Transfer:
            ux_flow_init(0, ux_transfer_flow, NULL);
            break;
        case TokenMint:
        case TokenBurn:
            ux_flow_init(0, ux_burn_mint_flow, NULL);
            break;

        default:
            break;
    }

#endif
}
