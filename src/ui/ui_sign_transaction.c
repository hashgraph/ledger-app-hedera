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
    // Number of screens is (len text in chars / display size in chars) + 1 for
    // overflowing text
    if (length <= 0) return 1;

    uint8_t screens = length / DISPLAY_SIZE;

    if (length % DISPLAY_SIZE > 0) {
        screens += 1;
    }

    return screens;
}

static void count_screens_of_step(void) {
    st_ctx.display_count = num_screens(strlen(st_ctx.full));
}

static bool last_screen_of_step() {
    return st_ctx.current_page == st_ctx.page_count;
}

static bool first_screen_of_step() { return st_ctx.current_page == 1; }

static void repaint(void) {
    // Slide window (partial) along full entity (full) by DISPLAY_SIZE chars
    MEMCLEAR(st_ctx.partial);
    memmove(st_ctx.partial,
            st_ctx.full + (DISPLAY_SIZE * (st_ctx.display_index - 1)),
            DISPLAY_SIZE);
    UX_REDISPLAY();
}

static void reformat_operator() {
    hedera_snprintf(
        st_ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
        st_ctx.transaction.transactionID.accountID.shardNum,
        st_ctx.transaction.transactionID.accountID.realmNum,
        st_ctx.transaction.transactionID.accountID.account.accountNum);

    count_screens_of_step();

    hedera_snprintf(st_ctx.title, DISPLAY_SIZE, "Operator (%u/%u)",
                    st_ctx.current_page, st_ctx.page_count);

    repaint();
}

static void reformat_senders() {
    switch (st_ctx.type) {
        case Verify:
            reformat_accounts("Account", 0);
            break;

        case Create:
        case Update:
            reformat_stake_target();
            break;

        case Associate:
        case Dissociate:
        case TokenMint:
        case TokenBurn:
            reformat_token();
            break;

        case TokenTransfer:
            reformat_tokens_accounts("Sender", st_ctx.transfer_from_index);
            break;

        case Transfer:
            reformat_accounts("Sender", st_ctx.transfer_from_index);
            break;

        default:
            return;
    }

    repaint();
}

static void reformat_recipients() {
    switch (st_ctx.type) {
        case Create:
        case Update:
            reformat_collect_rewards();
            break;

        case TokenTransfer:
            reformat_tokens_accounts("Recipient", st_ctx.transfer_to_index);
            break;

        case Transfer:
            reformat_accounts("Recipient", st_ctx.transfer_to_index);
            break;

        default:
            return;
    }

    repaint();
}

static void reformat_amount() {
    switch (st_ctx.type) {
        case Create:
            hedera_snprintf(
                st_ctx.full, DISPLAY_SIZE * 3, "%s hbar",
                hedera_format_tinybar(st_ctx.transaction.data
                                          .cryptoCreateAccount.initialBalance));

            break;

        case Update: {
            if (st_ctx.transaction.data.cryptoUpdateAccount
                    .has_accountIDToUpdate) {
                hedera_snprintf(st_ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                                st_ctx.transaction.data.cryptoUpdateAccount
                                    .accountIDToUpdate.shardNum,
                                st_ctx.transaction.data.cryptoUpdateAccount
                                    .accountIDToUpdate.realmNum,
                                st_ctx.transaction.data.cryptoUpdateAccount
                                    .accountIDToUpdate.account.accountNum);
            } else {
                hedera_snprintf(
                    st_ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    st_ctx.transaction.transactionID.accountID.shardNum,
                    st_ctx.transaction.transactionID.accountID.realmNum,
                    st_ctx.transaction.transactionID.accountID.account
                        .accountNum);
            }
        } break;

        case Transfer:
            hedera_snprintf(st_ctx.full, DISPLAY_SIZE * 3, "%s hbar",
                            hedera_format_tinybar(
                                st_ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ st_ctx.transfer_to_index ]
                                    .amount));

            break;

        case TokenMint:
            hedera_snprintf(
                st_ctx.full, DISPLAY_SIZE * 3, "%s",
                hedera_format_amount(
                    st_ctx.transaction.data.tokenMint.amount,
                    0)); // Must be lowest denomination without decimals

            break;

        case TokenBurn:
            hedera_snprintf(
                st_ctx.full, DISPLAY_SIZE * 3, "%s",
                hedera_format_amount(
                    st_ctx.transaction.data.tokenBurn.amount,
                    0)); // Must be lowest denomination without decimals

            break;

        case Associate:
        case Dissociate:
            reformat_target_account();
            break;

        case TokenTransfer:
            hedera_snprintf(
                st_ctx.full, DISPLAY_SIZE * 3, "%s",
                hedera_format_amount(
                    st_ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ st_ctx.transfer_to_index ]
                        .amount,
                    st_ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .expected_decimals.value));

            break;

        default:
            break;
    }

    count_screens_of_step();

    switch (st_ctx.type) {
        case Update:
        case Associate:
        case Dissociate:
            hedera_snprintf(st_ctx.title, DISPLAY_SIZE, "%s (%u/%u)",
                            "Updating", st_ctx.current_page, st_ctx.page_count);
            break;
        case Create:
            hedera_snprintf(st_ctx.title, DISPLAY_SIZE, "%s (%u/%u)", "Balance",
                            st_ctx.current_page, st_ctx.page_count);
            break;
        default:
            hedera_snprintf(st_ctx.title, DISPLAY_SIZE, "%s (%u/%u)", "Amount",
                            st_ctx.current_page, st_ctx.page_count);
            break;
    }

    repaint();
}

static void reformat_fee() {
    hedera_snprintf(st_ctx.full, DISPLAY_SIZE * 3, "%s hbar",
                    hedera_format_tinybar(st_ctx.transaction.transactionFee));

    count_screens_of_step();

    hedera_snprintf(st_ctx.title, DISPLAY_SIZE, "Max Fee (%u/%u)",
                    st_ctx.current_page, st_ctx.page_count);

    repaint();
}

static void reformat_memo() {
    hedera_snprintf(
        st_ctx.full, MAX_MEMO_SIZE, "%s",
        strlen(st_ctx.transaction.memo) > 0 ? st_ctx.transaction.memo : "");

    count_screens_of_step();

    hedera_snprintf(st_ctx.title, DISPLAY_SIZE, "Memo (%u/%u)",
                    st_ctx.current_page, st_ctx.page_count);

    repaint();
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

// Step 1: Transaction Summary --> Operator, Senders
unsigned int ui_tx_summary_step_button(unsigned int button_mask,
                                       unsigned int __attribute__((unused))
                                       button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            st_ctx.current_page = 1;

            if (st_ctx.type == Verify) {
                st_ctx.step = Senders;
                reformat_senders();
            } else {
                st_ctx.step = Operator;
                reformat_operator();
            }
            UX_DISPLAY(ui_tx_intermediate_step, NULL);
            break;
    }

    return 0;
}

void handle_intermediate_left_press() {
    // Navigate Left (scroll or return to previous step)
    switch (st_ctx.step) {
        case Operator: {
            if (first_screen_of_step()) {
                // All: Summary <-- Operator
                st_ctx.current_page = 1;
                st_ctx.step = Summary;
                UX_DISPLAY(ui_tx_summary_step, NULL);
            } else { // Scroll Left
                st_ctx.current_page--;
                reformat_operator();
            }
        } break;

        case Senders: {
            if (first_screen_of_step()) { // Return to Operator
                st_ctx.current_page = 1;

                if (st_ctx.type == Verify) {
                    // Verify: Summary <-- Senders
                    st_ctx.step = Summary;
                    UX_DISPLAY(ui_tx_summary_step, NULL);
                } else {
                    // All (!Verify): Operator <-- Senders
                    st_ctx.step = Operator;
                    reformat_operator();
                }
            } else { // Scroll Left
                st_ctx.current_page--;
                reformat_senders();
            }

        } break;

        case Recipients: {
            if (first_screen_of_step()) {
                // All: Senders <-- Recipients
                st_ctx.current_page = 1;
                st_ctx.step = Senders;
                reformat_senders();
            } else { // Scroll Left
                st_ctx.current_page--;
                reformat_recipients();
            }

        } break;

        case Amount: {
            if (first_screen_of_step()) {
                st_ctx.current_page = 1;

                if (st_ctx.type == TokenMint || st_ctx.type == TokenBurn ||
                    st_ctx.type == Associate || st_ctx.type == Dissociate) {
                    // Mint, Burn, Associate, Dissociate: Senders <-- Amount
                    st_ctx.step = Senders;
                    reformat_senders();
                } else {
                    // All (!Mint, !Burn, !Associate, !Dissociate): Recipients
                    // <-- Amount
                    st_ctx.step = Recipients;
                    reformat_recipients();
                }
            } else { // Scroll left
                st_ctx.current_page--;
                reformat_amount();
            }

        } break;

        case Fee: {
            if (first_screen_of_step()) {
                // All: Amount <-- Fee
                st_ctx.current_page = 1;
                st_ctx.step = Amount;
                reformat_amount();
            } else { // Scroll left
                st_ctx.current_page--;
                reformat_fee();
            }

        } break;

        case Memo: {
            if (first_screen_of_step()) {
                // All: Fee <-- Memo
                st_ctx.current_page = 1;
                st_ctx.step = Fee;
                reformat_fee();
            } else { // Scroll Left
                st_ctx.current_page--;
                reformat_memo();
            }

        } break;

        case Summary:
        case Confirm:
        case Deny:
            // Should not occur, does not handle these steps
            break;
    }
}

void handle_intermediate_right_press() {
    // Navigate Right (scroll or continue to next step)
    switch (st_ctx.step) {
        case Operator: {
            if (last_screen_of_step()) {
                // All: Operator --> Senders
                st_ctx.step = Senders;
                st_ctx.current_page = 1;
                reformat_senders();
            } else { // Scroll Right
                st_ctx.current_page++;
                reformat_operator();
            }

        } break;

        case Senders: {
            if (last_screen_of_step()) {
                st_ctx.current_page = 1;

                if (st_ctx.type == Verify) {
                    // Verify: Senders --> Confirm
                    st_ctx.step = Confirm;
                    UX_DISPLAY(ui_tx_confirm_step, NULL);
                } else if (st_ctx.type == TokenMint ||
                           st_ctx.type == TokenBurn ||
                           st_ctx.type == Associate ||
                           st_ctx.type == Dissociate) {
                    // Mint, Burn: Senders --> Amount
                    st_ctx.step = Amount;
                    reformat_amount();
                } else {
                    // Create, Update, Transfer: Senders --> Recipients
                    st_ctx.step = Recipients;
                    reformat_recipients();
                }
            } else { // Scroll Right
                st_ctx.current_page++;
                reformat_senders();
            }

        } break;

        case Recipients: {
            if (last_screen_of_step()) {
                // All (Create, Update, Transfer): Recipients --> Amount
                st_ctx.step = Amount;
                st_ctx.current_page = 1;
                reformat_amount();
            } else { // Scroll Right
                st_ctx.current_page++;
                reformat_recipients();
            }

        } break;

        case Amount: {
            if (last_screen_of_step()) {
                // All: Amount --> Fee
                st_ctx.step = Fee;
                st_ctx.current_page = 1;
                reformat_fee();
            } else { // Scroll Right
                st_ctx.current_page++;
                reformat_amount();
            }

        } break;

        case Fee: {
            if (last_screen_of_step()) {
                // All: Fee --> Memo
                st_ctx.step = Memo;
                st_ctx.current_page = 1;
                reformat_memo();
            } else { // Scroll Right
                st_ctx.current_page++;
                reformat_fee();
            }

        } break;

        case Memo: {
            if (last_screen_of_step()) {
                // All: Memo --> Confirm
                st_ctx.step = Confirm;
                st_ctx.current_page = 1;
                UX_DISPLAY(ui_tx_confirm_step, NULL);
            } else { // Scroll Right
                st_ctx.current_page++;
                reformat_memo();
            }
        } break;

        case Summary:
        case Confirm:
        case Deny:
            // Should not occur, does not handle these steps
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
            st_ctx.current_page = 1;

            if (st_ctx.type == Verify) {
                // Verify: Senders <-- Confirm
                st_ctx.step = Senders;
                reformat_senders();
            } else {
                // All (!Verify): Memo <-- Confirm
                st_ctx.step = Memo;
                reformat_memo();
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
             {.title = (char*)st_ctx.recipients_title,
              .text = (char*)st_ctx.recipients});

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
// Summary, Operator, Senders, Recipients, Amount, Fee, Memo, Confirm, Deny
UX_DEF(ux_transfer_flow, &ux_tx_flow_1_step, &ux_tx_flow_2_step,
       &ux_tx_flow_3_step, &ux_tx_flow_4_step, &ux_tx_flow_5_step,
       &ux_tx_flow_6_step, &ux_tx_flow_7_step, &ux_tx_flow_8_step,
       &ux_tx_flow_9_step);

// Create UX Flow
// Summary, Operator, Senders (Stake To), Recipients (Collect Rewards), Amount
// (Initial Balance), Fee, Memo, Confirm, Deny
UX_DEF(ux_create_flow, &ux_tx_flow_1_step, &ux_tx_flow_2_step,
       &ux_tx_flow_3_step, &ux_tx_flow_4_step, &ux_tx_flow_5_step,
       &ux_tx_flow_6_step, &ux_tx_flow_7_step, &ux_tx_flow_8_step,
       &ux_tx_flow_9_step);

// Update UX Flow
// Summary, Operator, Senders (Stake To), Recipients (Collect Rewards), Amount
// (Updated Account), Fee, Memo, Confirm, Deny
UX_DEF(ux_update_flow, &ux_tx_flow_1_step, &ux_tx_flow_2_step,
       &ux_tx_flow_3_step, &ux_tx_flow_4_step, &ux_tx_flow_5_step,
       &ux_tx_flow_6_step, &ux_tx_flow_7_step, &ux_tx_flow_8_step,
       &ux_tx_flow_9_step);

// Verify UX Flow
// Summary, Senders (Account), Confirm, Deny
UX_DEF(ux_verify_flow, &ux_tx_flow_1_step, &ux_tx_flow_3_step,
       &ux_tx_flow_8_step, &ux_tx_flow_9_step);

// TokenMint, TokenBurn
// Summary, Operator, Senders (Token), Amount, Fee, Memo, Confirm, Deny
UX_DEF(ux_mint_flow, &ux_tx_flow_1_step, &ux_tx_flow_2_step, &ux_tx_flow_4_step,
       &ux_tx_flow_5_step, &ux_tx_flow_6_step, &ux_tx_flow_7_step,
       &ux_tx_flow_8_step, &ux_tx_flow_9_step);

// Associate, Dissociate
// Summary, Operator, Senders (Token), Recipients (Account), Fee, Memo, Confirm,
// Deny
UX_DEF(ux_associate_flow, &ux_tx_flow_1_step, &ux_tx_flow_2_step,
       &ux_tx_flow_3_step, &ux_tx_flow_4_step, &ux_tx_flow_6_step,
       &ux_tx_flow_7_step, &ux_tx_flow_8_step, &ux_tx_flow_9_step);
#endif

void ui_sign_transaction(void) {
#if defined(TARGET_NANOS)

    UX_DISPLAY(ui_tx_summary_step, NULL);

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

    switch (ctx.type) {
        case Verify:
            ux_flow_init(0, ux_verify_flow, NULL);
            break;
        case TokenMint:
        case TokenBurn:
            ux_flow_init(0, ux_mint_flow, NULL);
            break;
        case Create:
            ux_flow_init(0, ux_create_flow, NULL);
            break;
        case Update:
            ux_flow_init(0, ux_update_flow, NULL);
            break;
        case Associate:
        case Dissociate:
            ux_flow_init(0, ux_associate_flow, NULL);
            break;
        case TokenTransfer:
        case Transfer:
            ux_flow_init(0, ux_transfer_flow, NULL);
            break;

        default:
            break;
    }

#endif
}
