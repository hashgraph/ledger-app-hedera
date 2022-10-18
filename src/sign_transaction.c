#include "sign_transaction.h"

#if defined(TARGET_NANOS)
/*
 * Supported Transactions:
 *
 * Verify:
 * "Verify Account with Key #0?" (Summary) <--> "Account" (Senders) <--> Confirm
<--> Deny
 *
 * Create:
 * "Create Account with Key #0?" (Summary) <--> Operator <--> "Stake to"
(Senders) <--> "Collect Rewards? Yes / No" (Recipients) <--> "Initial Balance"
(Amount) <--> Fee <--> Memo <--> Confirm <--> Deny
 *
 * Update:
 * "Update Account 0.0.0 with Key #0?" (Summary) <--> Operator <--> "Stake to"
(Senders) <--> "Collect Rewards (Yes / No)" (Recipients) <--> "Updated Account"
(Amount) <--> Fee <--> Memo <--> Confirm <--> Deny
 *
 * Transfer:
 * "Transfer with Key #0?" (Summary) <--> Operator <--> Senders <--> Recipients
<--> Amount <--> Fee <--> Memo <--> Confirm <--> Deny
 *
 * Associate:
 * "Associate Token with Key #0?" (Summary) <--> Operator <--> "Token" (Senders)
<--> "Updating" (Amount) <--> Fee <--> Memo <--> Confirm <--> Deny
 *
 * Dissociate:
 * "Dissociate Token with Key #0?" (Summary) <--> Operator <--> "Token"
(Senders) <--> "Updating" (Amount) <--> Fee <--> Memo <--> Confirm <--> Deny
 *
 * TokenMint:
 * "Mint Token with Key #0?" (Summary) <--> Operator <--> "Token" (Senders) <-->
Amount <--> Fee <--> Memo <--> Confirm <--> Deny
 *
 * TokenBurn:
 * "Burn Token with Key #0?" (Summary) <--> Operator <--> "Token" (Senders) <-->
Amount <--> Fee <--> Memo <--> Confirm <--> Deny
 *
 * I chose the steps for the originally supported CreateAccount and Transfer
transactions, and the additional transactions have been added since then. Steps
may be skipped or modified (as described above) from the original transfer flow.
The implementation of the steps is in the 'intermediate' screen button handlers.
These functions control iterating through the steps and control paging for
entities that overflow display on a single screen. The nano X has a paging macro
for this in its UX system. We don't have much RAM to work with, so I couldn't
define entirely separate UI elements for each flow, which lead me to using the
screens defined below as singletons.
 */

// Step 1: Transaction Summary
static const bagl_element_t ui_tx_summary_step[] = {
    UI_BACKGROUND(), UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // ()       >>
    // Line 1
    // Line 2

    UI_TEXT(LINE_1_ID, 0, 12, 128, ctx.summary_line_1),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.summary_line_2)};

// Step 2 - 7: Operator, Senders, Recipients, Amount, Fee, Memo
static const bagl_element_t ui_tx_intermediate_step[] = {
    UI_BACKGROUND(), UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),

    // <<       >>
    // <Title>
    // <Partial>

    UI_TEXT(LINE_1_ID, 0, 12, 128, ctx.title),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.partial)};

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
            ctx.current_page = 1;

            if (ctx.type == Verify) {
                ctx.step = Senders;
                reformat_senders();
            } else {
                ctx.step = Operator;
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
            if (first_screen_of_step()) {
                // All: Summary <-- Operator
                ctx.current_page = 1;
                ctx.step = Summary;
                UX_DISPLAY(ui_tx_summary_step, NULL);
            } else { // Scroll Left
                ctx.current_page--;
                reformat_operator();
            }
        } break;

        case Senders: {
            if (first_screen_of_step()) { // Return to Operator
                ctx.current_page = 1;

                if (ctx.type == Verify) {
                    // Verify: Summary <-- Senders
                    ctx.step = Summary;
                    UX_DISPLAY(ui_tx_summary_step, NULL);
                } else {
                    // All (!Verify): Operator <-- Senders
                    ctx.step = Operator;
                    reformat_operator();
                }
            } else { // Scroll Left
                ctx.current_page--;
                reformat_senders();
            }

        } break;

        case Recipients: {
            if (first_screen_of_step()) {
                // All: Senders <-- Recipients
                ctx.current_page = 1;
                ctx.step = Senders;
                reformat_senders();
            } else { // Scroll Left
                ctx.current_page--;
                reformat_recipients();
            }

        } break;

        case Amount: {
            if (first_screen_of_step()) {
                ctx.current_page = 1;

                if (ctx.type == TokenMint || ctx.type == TokenBurn ||
                    ctx.type == Associate || ctx.type == Dissociate) {
                    // Mint, Burn, Associate, Dissociate: Senders <-- Amount
                    ctx.step = Senders;
                    reformat_senders();
                } else {
                    // All (!Mint, !Burn, !Associate, !Dissociate): Recipients
                    // <-- Amount
                    ctx.step = Recipients;
                    reformat_recipients();
                }
            } else { // Scroll left
                ctx.current_page--;
                reformat_amount();
            }

        } break;

        case Fee: {
            if (first_screen_of_step()) {
                // All: Amount <-- Fee
                ctx.current_page = 1;
                ctx.step = Amount;
                reformat_amount();
            } else { // Scroll left
                ctx.current_page--;
                reformat_fee();
            }

        } break;

        case Memo: {
            if (first_screen_of_step()) {
                // All: Fee <-- Memo
                ctx.current_page = 1;
                ctx.step = Fee;
                reformat_fee();
            } else { // Scroll Left
                ctx.current_page--;
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
    switch (ctx.step) {
        case Operator: {
            if (last_screen_of_step()) {
                // All: Operator --> Senders
                ctx.step = Senders;
                ctx.current_page = 1;
                reformat_senders();
            } else { // Scroll Right
                ctx.current_page++;
                reformat_operator();
            }

        } break;

        case Senders: {
            if (last_screen_of_step()) {
                ctx.current_page = 1;

                if (ctx.type == Verify) {
                    // Verify: Senders --> Confirm
                    ctx.step = Confirm;
                    UX_DISPLAY(ui_tx_confirm_step, NULL);
                } else if (ctx.type == TokenMint || ctx.type == TokenBurn ||
                           ctx.type == Associate || ctx.type == Dissociate) {
                    // Mint, Burn: Senders --> Amount
                    ctx.step = Amount;
                    reformat_amount();
                } else {
                    // Create, Update, Transfer: Senders --> Recipients
                    ctx.step = Recipients;
                    reformat_recipients();
                }
            } else { // Scroll Right
                ctx.current_page++;
                reformat_senders();
            }

        } break;

        case Recipients: {
            if (last_screen_of_step()) {
                // All (Create, Update, Transfer): Recipients --> Amount
                ctx.step = Amount;
                ctx.current_page = 1;
                reformat_amount();
            } else { // Scroll Right
                ctx.current_page++;
                reformat_recipients();
            }

        } break;

        case Amount: {
            if (last_screen_of_step()) {
                // All: Amount --> Fee
                ctx.step = Fee;
                ctx.current_page = 1;
                reformat_fee();
            } else { // Scroll Right
                ctx.current_page++;
                reformat_amount();
            }

        } break;

        case Fee: {
            if (last_screen_of_step()) {
                // All: Fee --> Memo
                ctx.step = Memo;
                ctx.current_page = 1;
                reformat_memo();
            } else { // Scroll Right
                ctx.current_page++;
                reformat_fee();
            }

        } break;

        case Memo: {
            if (last_screen_of_step()) {
                // All: Memo --> Confirm
                ctx.step = Confirm;
                ctx.current_page = 1;
                UX_DISPLAY(ui_tx_confirm_step, NULL);
            } else { // Scroll Right
                ctx.current_page++;
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
            // Skip to confirm screen on double press
            ctx.step = Confirm;
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
            ctx.current_page = 1;

            if (ctx.type == Verify) {
                // Verify: Senders <-- Confirm
                ctx.step = Senders;
                reformat_senders();
            } else {
                // All (!Verify): Memo <-- Confirm
                ctx.step = Memo;
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

unsigned int ui_tx_deny_step_button(unsigned int button_mask,
                                    unsigned int __attribute__((unused))
                                    button_mask_counter) {
    switch (button_mask) {
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
    // Number of screens is (len text in chars / display size in chars) + 1 for
    // overflowing text
    if (length <= 0) return 1;

    uint8_t screens = length / DISPLAY_SIZE;

    if (length % DISPLAY_SIZE > 0) {
        screens += 1;
    }

    return screens;
}

void count_screens_of_step() { ctx.page_count = num_screens(strlen(ctx.full)); }

void repaint() {
    // Slide window (partial) along full entity (full) by DISPLAY_SIZE chars
    explicit_bzero(ctx.partial, DISPLAY_SIZE + 1);
    memmove(ctx.partial, ctx.full + (DISPLAY_SIZE * (ctx.current_page - 1)),
            DISPLAY_SIZE);
    UX_REDISPLAY();
}

bool last_screen_of_step() { return ctx.current_page == ctx.page_count; }

bool first_screen_of_step() { return ctx.current_page == 1; }

void reformat_operator() {
    hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account.accountNum);

    count_screens_of_step();

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "Operator (%u/%u)",
                    ctx.current_page, ctx.page_count);

    repaint();
}

void reformat_accounts(char* title_part, uint8_t transfer_index) {
    hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts[ transfer_index ]
                        .accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts[ transfer_index ]
                        .accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts[ transfer_index ]
                        .accountID.account.accountNum);

    count_screens_of_step();

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "%s (%u/%u)", title_part,
                    ctx.current_page, ctx.page_count);
}

void reformat_stake_target() {
    switch (ctx.type) {
        case Create: {
            if (ctx.transaction.data.cryptoCreateAccount.which_staked_id ==
                Hedera_CryptoCreateTransactionBody_staked_account_id_tag) {
                // An account ID and not a Node ID
                hedera_snprintf(
                    ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.cryptoCreateAccount.staked_id
                        .staked_account_id.shardNum,
                    ctx.transaction.data.cryptoCreateAccount.staked_id
                        .staked_account_id.realmNum,
                    ctx.transaction.data.cryptoCreateAccount.staked_id
                        .staked_account_id.account.accountNum);
            } else if (ctx.transaction.data.cryptoCreateAccount
                           .which_staked_id ==
                       Hedera_CryptoCreateTransactionBody_staked_node_id_tag) {
                hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "Node %lld",
                                ctx.transaction.data.cryptoCreateAccount
                                    .staked_id.staked_node_id);
            } else {
                hedera_snprintf(ctx.full, DISPLAY_SIZE, "%s", "None");
            }
        } break;

        case Update: {
            if (ctx.transaction.data.cryptoUpdateAccount.which_staked_id ==
                Hedera_CryptoUpdateTransactionBody_staked_account_id_tag) {
                // An account ID and not a Node ID
                hedera_snprintf(
                    ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.cryptoUpdateAccount.staked_id
                        .staked_account_id.shardNum,
                    ctx.transaction.data.cryptoUpdateAccount.staked_id
                        .staked_account_id.realmNum,
                    ctx.transaction.data.cryptoUpdateAccount.staked_id
                        .staked_account_id.account.accountNum);
            } else if (ctx.transaction.data.cryptoUpdateAccount
                           .which_staked_id ==
                       Hedera_CryptoUpdateTransactionBody_staked_node_id_tag) {
                hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "Node %lld",
                                ctx.transaction.data.cryptoUpdateAccount
                                    .staked_id.staked_node_id);
            } else {
                hedera_snprintf(ctx.full, DISPLAY_SIZE, "%s", "None");
            }
        } break;

        default:
            break;
    }

    count_screens_of_step();

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "Stake To (%u/%u)",
                    ctx.current_page, ctx.page_count);
}

void reformat_token() {
    switch (ctx.type) {
        case Associate:
            hedera_snprintf(
                ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                ctx.transaction.data.tokenAssociate.tokens[ 0 ].shardNum,
                ctx.transaction.data.tokenAssociate.tokens[ 0 ].realmNum,
                ctx.transaction.data.tokenAssociate.tokens[ 0 ].tokenNum);

            break;

        case Dissociate:
            hedera_snprintf(
                ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                ctx.transaction.data.tokenDissociate.tokens[ 0 ].shardNum,
                ctx.transaction.data.tokenDissociate.tokens[ 0 ].realmNum,
                ctx.transaction.data.tokenDissociate.tokens[ 0 ].tokenNum);

            break;

        case TokenMint:
            hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                            ctx.transaction.data.tokenMint.token.shardNum,
                            ctx.transaction.data.tokenMint.token.realmNum,
                            ctx.transaction.data.tokenMint.token.tokenNum);

            break;

        case TokenBurn:
            hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                            ctx.transaction.data.tokenBurn.token.shardNum,
                            ctx.transaction.data.tokenBurn.token.realmNum,
                            ctx.transaction.data.tokenBurn.token.tokenNum);

            break;

        default:
            break;
    }

    count_screens_of_step();

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "Token (%u/%u)", ctx.current_page,
                    ctx.page_count);
}

void reformat_tokens_accounts(char* title_part, uint8_t transfer_index) {
    hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ transfer_index ]
                        .accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ transfer_index ]
                        .accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ transfer_index ]
                        .accountID.account.accountNum);

    count_screens_of_step();

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "%s (%u/%u)", title_part,
                    ctx.current_page, ctx.page_count);
}

void reformat_senders() {
    switch (ctx.type) {
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
            reformat_tokens_accounts("Sender", ctx.transfer_from_index);
            break;

        case Transfer:
            reformat_accounts("Sender", ctx.transfer_from_index);
            break;

        default:
            return;
    }

    repaint();
}

void reformat_collect_rewards() {
    switch (ctx.type) {
        case Create: {
            bool declineRewards =
                ctx.transaction.data.cryptoCreateAccount.decline_reward;
            hedera_snprintf(ctx.full, MAX_MEMO_SIZE, "%s",
                            !declineRewards ? "Yes" : "No");
        } break;

        case Update: {
            if (ctx.transaction.data.cryptoUpdateAccount.has_decline_reward) {
                bool declineRewards = ctx.transaction.data.cryptoUpdateAccount
                                          .decline_reward.value;
                hedera_snprintf(ctx.full, MAX_MEMO_SIZE, "%s",
                                declineRewards ? "No" : "Yes");
            } else {
                hedera_snprintf(ctx.full, MAX_MEMO_SIZE, "%s", "-");
            }
        } break;

        default:
            break;
    }

    count_screens_of_step(); // 1

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "Collect Rewards?",
                    ctx.current_page, ctx.page_count);

    repaint();
}

void reformat_target_account() {
    switch (ctx.type) {
        case Associate: {
            bool hasAccount = ctx.transaction.data.tokenAssociate.has_account;
            if (hasAccount) {
                hedera_snprintf(
                    ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.tokenAssociate.account.shardNum,
                    ctx.transaction.data.tokenAssociate.account.realmNum,
                    ctx.transaction.data.tokenAssociate.account.account
                        .accountNum);
            } else {
                hedera_snprintf(
                    ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account.accountNum);
            }
        } break;

        case Dissociate: {
            bool hasAccount = ctx.transaction.data.tokenDissociate.has_account;
            if (hasAccount) {
                hedera_snprintf(
                    ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.tokenDissociate.account.shardNum,
                    ctx.transaction.data.tokenDissociate.account.realmNum,
                    ctx.transaction.data.tokenDissociate.account.account
                        .accountNum);
            } else {
                hedera_snprintf(
                    ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account.accountNum);
            }
        } break;

        default:
            break;
    }
}

void reformat_recipients() {
    switch (ctx.type) {
        case Create:
        case Update:
            reformat_collect_rewards();
            break;

        case TokenTransfer:
            reformat_tokens_accounts("Recipient", ctx.transfer_to_index);
            break;

        case Transfer:
            reformat_accounts("Recipient", ctx.transfer_to_index);
            break;

        default:
            return;
    }

    repaint();
}

void reformat_amount() {
    switch (ctx.type) {
        case Create:
            hedera_snprintf(
                ctx.full, DISPLAY_SIZE * 3, "%s hbar",
                hedera_format_tinybar(
                    ctx.transaction.data.cryptoCreateAccount.initialBalance));

            break;

        case Update: {
            if (ctx.transaction.data.cryptoUpdateAccount
                    .has_accountIDToUpdate) {
                hedera_snprintf(ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                                ctx.transaction.data.cryptoUpdateAccount
                                    .accountIDToUpdate.shardNum,
                                ctx.transaction.data.cryptoUpdateAccount
                                    .accountIDToUpdate.realmNum,
                                ctx.transaction.data.cryptoUpdateAccount
                                    .accountIDToUpdate.account.accountNum);
            } else {
                hedera_snprintf(
                    ctx.full, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account.accountNum);
            }
        } break;

        case Transfer:
            hedera_snprintf(ctx.full, DISPLAY_SIZE * 3, "%s hbar",
                            hedera_format_tinybar(
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ ctx.transfer_to_index ]
                                    .amount));

            break;

        case TokenMint:
            hedera_snprintf(
                ctx.full, DISPLAY_SIZE * 3, "%s",
                hedera_format_amount(
                    ctx.transaction.data.tokenMint.amount,
                    0)); // Must be lowest denomination without decimals

            break;

        case TokenBurn:
            hedera_snprintf(
                ctx.full, DISPLAY_SIZE * 3, "%s",
                hedera_format_amount(
                    ctx.transaction.data.tokenBurn.amount,
                    0)); // Must be lowest denomination without decimals

            break;

        case Associate:
        case Dissociate:
            reformat_target_account();
            break;

        case TokenTransfer:
            hedera_snprintf(
                ctx.full, DISPLAY_SIZE * 3, "%s",
                hedera_format_amount(
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ ctx.transfer_to_index ]
                        .amount,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .expected_decimals.value));

            break;

        default:
            break;
    }

    count_screens_of_step();

    switch (ctx.type) {
        case Update:
        case Associate:
        case Dissociate:
            hedera_snprintf(ctx.title, DISPLAY_SIZE, "%s (%u/%u)", "Updating",
                            ctx.current_page, ctx.page_count);
            break;
        case Create:
            hedera_snprintf(ctx.title, DISPLAY_SIZE, "%s (%u/%u)", "Balance",
                            ctx.current_page, ctx.page_count);
            break;
        default:
            hedera_snprintf(ctx.title, DISPLAY_SIZE, "%s (%u/%u)", "Amount",
                            ctx.current_page, ctx.page_count);
            break;
    }

    repaint();
}

void reformat_fee() {
    hedera_snprintf(ctx.full, DISPLAY_SIZE * 3, "%s hbar",
                    hedera_format_tinybar(ctx.transaction.transactionFee));

    count_screens_of_step();

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "Max Fee (%u/%u)",
                    ctx.current_page, ctx.page_count);

    repaint();
}

void reformat_memo() {
    hedera_snprintf(
        ctx.full, MAX_MEMO_SIZE, "%s",
        strlen(ctx.transaction.memo) > 0 ? ctx.transaction.memo : "");

    count_screens_of_step();

    hedera_snprintf(ctx.title, DISPLAY_SIZE, "Memo (%u/%u)", ctx.current_page,
                    ctx.page_count);

    repaint();
}

void handle_transaction_body() {
    explicit_bzero(ctx.summary_line_1, DISPLAY_SIZE + 1);
    explicit_bzero(ctx.summary_line_2, DISPLAY_SIZE + 1);
    explicit_bzero(ctx.full, DISPLAY_SIZE * 3 + 1);
    explicit_bzero(ctx.partial, DISPLAY_SIZE + 1);

    // Step 1, Unknown Type, Screen 1 of 1
    ctx.step = Summary;
    ctx.type = Unknown;
    ctx.current_page = 1;
    ctx.page_count = 1;

    // <Do Action>
    // with Key #X?
    hedera_snprintf(ctx.summary_line_2, DISPLAY_SIZE, "with Key #%u?",
                    ctx.key_index);

    // Handle parsed protobuf message of transaction body
    switch (ctx.transaction.which_data) {
        case Hedera_TransactionBody_cryptoCreateAccount_tag:
            ctx.type = Create;
            hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE, "Create Account");
            break;

        case Hedera_TransactionBody_cryptoUpdateAccount_tag:
            ctx.type = Update;
            hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE, "Update Account");
            break;

        case Hedera_TransactionBody_tokenAssociate_tag:
            ctx.type = Associate;
            hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE,
                            "Associate Token");
            break;

        case Hedera_TransactionBody_tokenDissociate_tag:
            ctx.type = Dissociate;
            hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE,
                            "Dissociate Token");
            break;

        case Hedera_TransactionBody_tokenMint_tag:
            ctx.type = TokenMint;
            hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE, "Mint Token");
            break;

        case Hedera_TransactionBody_tokenBurn_tag:
            ctx.type = TokenBurn;
            hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE, "Burn Token");
            break;

        case Hedera_TransactionBody_cryptoTransfer_tag: {
            if (
                /*
                 * "Verify Account Transaction"
                 *
                 * This is an arbitary transfer transaction that is designed
                 * to fail always. Transfer 0 hbar to no-one with a max fee
                 * of 1 tinybar. If this transaction fails with
                 * "insufficient fee" rather than "invalid signature", then
                 * we know that the signature provided by this key is indeed
                 * associated with the operator account for the transaction.
                 */
                ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts[ 0 ]
                        .amount == 0 &&
                ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts_count == 1 &&
                ctx.transaction.transactionFee == 1) {
                ctx.type = Verify;

                hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE,
                                "Verify Account");
            } else if (ctx.transaction.data.cryptoTransfer.transfers
                           .accountAmounts_count == 2) {
                // Hbar Transfer between two accounts
                ctx.type = Transfer;

                hedera_snprintf(ctx.summary_line_1, DISPLAY_SIZE, "Send Hbar");

                // Determine Sender based on transfers.accountAmounts
                ctx.transfer_to_index = 1;
                ctx.transfer_from_index = 0;
                if (ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts[ 0 ]
                        .amount > 0) {
                    ctx.transfer_to_index = 0;
                    ctx.transfer_from_index = 1;
                }
            } else if (ctx.transaction.data.cryptoTransfer
                           .tokenTransfers_count == 1) {
                // Fungible Token Transfer (two token transfers with one
                // transfer each)
                ctx.type = TokenTransfer;

                validate_token_transfer();

                hedera_snprintf(
                    ctx.summary_line_1, DISPLAY_SIZE, "Send %llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .token.shardNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .token.realmNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .token.tokenNum);

                // Determine Sender based on amount
                ctx.transfer_from_index = 0;
                ctx.transfer_to_index = 1;
                if (ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ 0 ]
                        .amount > 0) {
                    ctx.transfer_from_index = 1;
                    ctx.transfer_to_index = 0;
                }
            } else {
                // Unsupported
                THROW(EXCEPTION_MALFORMED_APDU);
            }
        } break;

        default:
            // Unsupported
            THROW(EXCEPTION_MALFORMED_APDU);
            break;
    }

    UX_DISPLAY(ui_tx_summary_step, NULL);
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
             {"Transaction Summary", ctx.summary_line_1, ctx.summary_line_2});

UX_STEP_NOCB(
    ux_tx_flow_2_step,
    bnnn_paging,
    {
        .title = "Operator",
        .text = (char*) ctx.operator
    }
);

UX_STEP_NOCB(ux_tx_flow_3_step, bnnn_paging,
             {.title = (char*)ctx.senders_title, .text = (char*)ctx.senders});

UX_STEP_NOCB(ux_tx_flow_4_step, bnnn_paging,
             {.title = (char*)ctx.recipients_title,
              .text = (char*)ctx.recipients});

UX_STEP_NOCB(ux_tx_flow_5_step, bnnn_paging,
             {.title = (char*)ctx.amount_title, .text = (char*)ctx.amount});

UX_STEP_NOCB(ux_tx_flow_6_step, bnnn_paging,
             {.title = "Max Fee", .text = (char*)ctx.fee});

UX_STEP_NOCB(ux_tx_flow_7_step, bnnn_paging,
             {.title = "Memo", .text = (char*)ctx.memo});

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

void handle_transaction_body() {
    explicit_bzero(ctx.summary_line_1, DISPLAY_SIZE + 1);
    explicit_bzero(ctx.summary_line_2, DISPLAY_SIZE + 1);
    explicit_bzero(ctx.amount_title, DISPLAY_SIZE + 1);
    explicit_bzero(ctx.senders_title, DISPLAY_SIZE + 1);
    explicit_bzero(ctx.recipients_title, DISPLAY_SIZE + 1);
    explicit_bzero(ctx.operator, DISPLAY_SIZE * 2 + 1);
    explicit_bzero(ctx.senders, DISPLAY_SIZE * 2 + 1);
    explicit_bzero(ctx.recipients, DISPLAY_SIZE * 2 + 1);
    explicit_bzero(ctx.fee, DISPLAY_SIZE * 2 + 1);
    explicit_bzero(ctx.amount, DISPLAY_SIZE * 2 + 1);
    explicit_bzero(ctx.memo, MAX_MEMO_SIZE + 1);

    ctx.type = Unknown;

    // <Do Action>
    // with Key #X?
    hedera_snprintf(ctx.summary_line_2, DISPLAY_SIZE, "with Key #%u?",
                    ctx.key_index);

    hedera_snprintf(ctx.operator, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account);

    hedera_snprintf(ctx.fee, DISPLAY_SIZE * 2, "%s hbar",
                    hedera_format_tinybar(ctx.transaction.transactionFee));

    hedera_snprintf(ctx.memo, MAX_MEMO_SIZE, "%s", ctx.transaction.memo);

    hedera_sprintf(ctx.amount_title, "Amount");
    hedera_sprintf(ctx.senders_title, "Sender");
    hedera_sprintf(ctx.recipients_title, "Recipient");

    // Handle parsed protobuf message of transaction body
    switch (ctx.transaction.which_data) {
        case Hedera_TransactionBody_cryptoCreateAccount_tag: {
            ctx.type = Create;
            hedera_sprintf(ctx.summary_line_1, "Create Account");
            hedera_sprintf(ctx.senders_title, "Stake To");
            hedera_sprintf(ctx.recipients_title, "Collect Rewards?");
            hedera_sprintf(ctx.amount_title, "Balance");

            char stake_target[ ACCOUNT_ID_SIZE ];

            if (ctx.transaction.data.cryptoCreateAccount.which_staked_id ==
                Hedera_CryptoCreateTransactionBody_staked_account_id_tag) {
                hedera_snprintf(stake_target, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                                ctx.transaction.data.cryptoCreateAccount
                                    .staked_id.staked_account_id.shardNum,
                                ctx.transaction.data.cryptoCreateAccount
                                    .staked_id.staked_account_id.realmNum,
                                ctx.transaction.data.cryptoCreateAccount
                                    .staked_id.staked_account_id.account);
            } else if (ctx.transaction.data.cryptoCreateAccount
                           .which_staked_id ==
                       Hedera_CryptoCreateTransactionBody_staked_node_id_tag) {
                hedera_snprintf(stake_target, ACCOUNT_ID_SIZE, "Node %lld",
                                ctx.transaction.data.cryptoCreateAccount
                                    .staked_id.staked_node_id);
            } else {
                hedera_snprintf(stake_target, DISPLAY_SIZE * 2, "None");
            }

            hedera_snprintf(ctx.senders, DISPLAY_SIZE * 2, "%s", stake_target);

            hedera_snprintf(
                ctx.recipients, DISPLAY_SIZE * 2, "%s",
                ctx.transaction.data.cryptoCreateAccount.decline_reward
                    ? "No"
                    : "Yes");

            hedera_snprintf(
                ctx.amount, DISPLAY_SIZE * 2, "%s hbar",
                hedera_format_tinybar(
                    ctx.transaction.data.cryptoCreateAccount.initialBalance));
        } break;

        case Hedera_TransactionBody_cryptoUpdateAccount_tag: {
            ctx.type = Update;
            hedera_sprintf(ctx.summary_line_1, "Update Account");
            hedera_sprintf(ctx.senders_title, "Stake To");
            hedera_sprintf(ctx.recipients_title, "Collect Rewards");
            hedera_sprintf(ctx.amount_title, "Updating");

            const char stake_target[ DISPLAY_SIZE * 2 ];

            if (ctx.transaction.data.cryptoUpdateAccount.which_staked_id ==
                Hedera_CryptoUpdateTransactionBody_staked_account_id_tag) {
                hedera_snprintf(stake_target, DISPLAY_SIZE * 2,
                                "%llu.%llu.%llu",
                                ctx.transaction.data.cryptoUpdateAccount
                                    .staked_id.staked_account_id.shardNum,
                                ctx.transaction.data.cryptoUpdateAccount
                                    .staked_id.staked_account_id.realmNum,
                                ctx.transaction.data.cryptoUpdateAccount
                                    .staked_id.staked_account_id.account);
            } else if (ctx.transaction.data.cryptoUpdateAccount
                           .which_staked_id ==
                       Hedera_CryptoUpdateTransactionBody_staked_node_id_tag) {
                hedera_snprintf(stake_target, ACCOUNT_ID_SIZE, "Node %lld",
                                ctx.transaction.data.cryptoUpdateAccount
                                    .staked_id.staked_node_id);
            } else {
                hedera_snprintf(stake_target, DISPLAY_SIZE * 2, "None");
            }

            hedera_snprintf(ctx.senders, DISPLAY_SIZE * 2, "%s", stake_target);

            if (ctx.transaction.data.cryptoUpdateAccount.has_decline_reward) {
                bool declineRewards = ctx.transaction.data.cryptoUpdateAccount
                                          .decline_reward.value;
                hedera_snprintf(ctx.recipients, DISPLAY_SIZE, "%s",
                                declineRewards ? "No" : "Yes");
            } else {
                hedera_snprintf(ctx.recipients, DISPLAY_SIZE, "%s", "-");
            }

            if (ctx.transaction.data.cryptoUpdateAccount
                    .has_accountIDToUpdate) {
                Hedera_AccountID updatedAccount =
                    ctx.transaction.data.cryptoUpdateAccount.accountIDToUpdate;
                hedera_snprintf(ctx.amount, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                                updatedAccount.shardNum,
                                updatedAccount.realmNum,
                                updatedAccount.account);
            } else {
                hedera_snprintf(
                    ctx.amount, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account);
            }
        } break;

        case Hedera_TransactionBody_tokenAssociate_tag: {
            ctx.type = Associate;

            hedera_sprintf(ctx.summary_line_1, "Associate Token");
            hedera_sprintf(ctx.senders_title, "Token");
            hedera_sprintf(ctx.recipients_title, "Account");

            hedera_snprintf(
                ctx.senders, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                ctx.transaction.data.tokenAssociate.tokens[ 0 ].shardNum,
                ctx.transaction.data.tokenAssociate.tokens[ 0 ].realmNum,
                ctx.transaction.data.tokenAssociate.tokens[ 0 ].tokenNum);

            bool hasAccount = ctx.transaction.data.tokenAssociate.has_account;

            if (hasAccount) {
                hedera_snprintf(
                    ctx.recipients, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.tokenAssociate.account.shardNum,
                    ctx.transaction.data.tokenAssociate.account.realmNum,
                    ctx.transaction.data.tokenAssociate.account.account
                        .accountNum);
            } else {
                hedera_snprintf(
                    ctx.recipients, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account.accountNum);
            }
        } break;

        case Hedera_TransactionBody_tokenDissociate_tag: {
            ctx.type = Dissociate;

            hedera_sprintf(ctx.summary_line_1, "Dissociate Token");
            hedera_sprintf(ctx.senders_title, "Token");
            hedera_sprintf(ctx.recipients_title, "Account");

            hedera_snprintf(
                ctx.senders, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                ctx.transaction.data.tokenDissociate.tokens[ 0 ].shardNum,
                ctx.transaction.data.tokenDissociate.tokens[ 0 ].realmNum,
                ctx.transaction.data.tokenDissociate.tokens[ 0 ].tokenNum);

            bool hasAccount = ctx.transaction.data.tokenAssociate.has_account;

            if (hasAccount) {
                hedera_snprintf(
                    ctx.recipients, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.data.tokenDissociate.account.shardNum,
                    ctx.transaction.data.tokenDissociate.account.realmNum,
                    ctx.transaction.data.tokenDissociate.account.account
                        .accountNum);
            } else {
                hedera_snprintf(
                    ctx.recipients, ACCOUNT_ID_SIZE, "%llu.%llu.%llu",
                    ctx.transaction.transactionID.accountID.shardNum,
                    ctx.transaction.transactionID.accountID.realmNum,
                    ctx.transaction.transactionID.accountID.account.accountNum);
            }
        } break;

        case Hedera_TransactionBody_tokenMint_tag: {
            ctx.type = TokenMint;

            hedera_sprintf(ctx.summary_line_1, "Mint Token");

            hedera_sprintf(ctx.senders_title, "Token");

            hedera_snprintf(ctx.senders, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                            ctx.transaction.data.tokenMint.token.shardNum,
                            ctx.transaction.data.tokenMint.token.realmNum,
                            ctx.transaction.data.tokenMint.token.tokenNum);

            hedera_snprintf(
                ctx.amount, DISPLAY_SIZE * 2, "%s",
                hedera_format_amount(ctx.transaction.data.tokenMint.amount,
                                     0)); // always lowest denomination of token
        } break;

        case Hedera_TransactionBody_tokenBurn_tag: {
            ctx.type = TokenBurn;

            hedera_sprintf(ctx.summary_line_1, "Burn Token");

            hedera_sprintf(ctx.senders_title, "Token");

            hedera_snprintf(ctx.senders, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                            ctx.transaction.data.tokenBurn.token.shardNum,
                            ctx.transaction.data.tokenBurn.token.realmNum,
                            ctx.transaction.data.tokenBurn.token.tokenNum);

            hedera_snprintf(
                ctx.amount, DISPLAY_SIZE * 2, "%s",
                hedera_format_amount(ctx.transaction.data.tokenBurn.amount,
                                     0)); // always lowest denomination of token
        } break;

        case Hedera_TransactionBody_cryptoTransfer_tag: {
            if ( // Only 1 Account (Sender), Fee 1 Tinybar, and Value 0 Tinybar
                ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts[ 0 ]
                        .amount == 0 &&
                ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts_count == 1 &&
                ctx.transaction.transactionFee == 1) {
                // Verify Account Transaction
                ctx.type = Verify;

                hedera_sprintf(ctx.summary_line_1, "Verify Account");

                hedera_sprintf(ctx.senders_title, "Account");

                hedera_snprintf(ctx.senders, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ 0 ]
                                    .accountID.shardNum,
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ 0 ]
                                    .accountID.realmNum,
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ 0 ]
                                    .accountID.account);
            } else if (ctx.transaction.data.cryptoTransfer.transfers
                           .accountAmounts_count == 2) {
                // Number of Accounts == 2
                // Hbar transfer between two accounts
                ctx.type = Transfer;

                hedera_sprintf(ctx.summary_line_1, "Send Hbar");

                // Determine Sender based on transfers.accountAmounts
                ctx.transfer_from_index = 0;
                ctx.transfer_to_index = 1;
                if (ctx.transaction.data.cryptoTransfer.transfers
                        .accountAmounts[ 0 ]
                        .amount > 0) {
                    ctx.transfer_from_index = 1;
                    ctx.transfer_to_index = 0;
                }

                hedera_snprintf(ctx.senders, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ ctx.transfer_from_index ]
                                    .accountID.shardNum,
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ ctx.transfer_from_index ]
                                    .accountID.realmNum,
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ ctx.transfer_from_index ]
                                    .accountID.account);

                hedera_snprintf(ctx.recipients, DISPLAY_SIZE * 2,
                                "%llu.%llu.%llu",
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ ctx.transfer_to_index ]
                                    .accountID.shardNum,
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ ctx.transfer_to_index ]
                                    .accountID.realmNum,
                                ctx.transaction.data.cryptoTransfer.transfers
                                    .accountAmounts[ ctx.transfer_to_index ]
                                    .accountID.account);

                hedera_snprintf(
                    ctx.amount, DISPLAY_SIZE * 2, "%s hbar",
                    hedera_format_tinybar(
                        ctx.transaction.data.cryptoTransfer.transfers
                            .accountAmounts[ ctx.transfer_to_index ]
                            .amount));
            } else if (ctx.transaction.data.cryptoTransfer
                           .tokenTransfers_count == 1) {
                // Fungible Token Transfer
                ctx.type = TokenTransfer;

                validate_token_transfer();

                hedera_snprintf(
                    ctx.summary_line_1, DISPLAY_SIZE * 2, "Send %llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .token.shardNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .token.realmNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .token.tokenNum);

                // Determine Sender based on amount
                ctx.transfer_from_index = 0;
                ctx.transfer_to_index = 1;
                if (ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ 0 ]
                        .amount > 0) {
                    ctx.transfer_from_index = 1;
                    ctx.transfer_to_index = 0;
                }

                hedera_snprintf(
                    ctx.senders, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ ctx.transfer_from_index ]
                        .accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ ctx.transfer_from_index ]
                        .accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ ctx.transfer_from_index ]
                        .accountID.account);

                hedera_snprintf(
                    ctx.recipients, DISPLAY_SIZE * 2, "%llu.%llu.%llu",
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ ctx.transfer_to_index ]
                        .accountID.shardNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ ctx.transfer_to_index ]
                        .accountID.realmNum,
                    ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                        .transfers[ ctx.transfer_to_index ]
                        .accountID.account);

                hedera_snprintf(
                    ctx.amount, DISPLAY_SIZE * 2, "%s",
                    hedera_format_amount(
                        ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                            .transfers[ ctx.transfer_to_index ]
                            .amount,
                        ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                            .expected_decimals.value));
            } else {
                // Unsupported
                THROW(EXCEPTION_MALFORMED_APDU);
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
}
#endif

// Sign Handler
// Decodes and handles transaction message
void handle_sign_transaction(uint8_t p1, uint8_t p2, uint8_t* buffer,
                             uint16_t len,
                             /* out */ volatile unsigned int* flags,
                             /* out */ volatile unsigned int* tx) {
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(tx);

    // Key Index
    ctx.key_index = U4LE(buffer, 0);

    // Raw Tx
    uint8_t raw_transaction[ MAX_TX_SIZE ];
    int raw_transaction_length = len - 4;

    // Oops Oof Owie
    if (raw_transaction_length > MAX_TX_SIZE) {
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    // copy raw transaction
    memmove(raw_transaction, (buffer + 4), raw_transaction_length);

    // Sign Transaction
    // TODO: handle error return here (internal error?!)
    if (!hedera_sign(ctx.key_index, raw_transaction, raw_transaction_length,
                     G_io_apdu_buffer)) {
        THROW(EXCEPTION_INTERNAL);
    }

    // Make in memory buffer into stream
    pb_istream_t stream =
        pb_istream_from_buffer(raw_transaction, raw_transaction_length);

    // Decode the Transaction
    if (!pb_decode(&stream, Hedera_TransactionBody_fields, &ctx.transaction)) {
        // Oh no couldn't ...
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    handle_transaction_body();

    *flags |= IO_ASYNCH_REPLY;
}

void validate_decimals(uint32_t decimals) {
    if (decimals >= 20) {
        // We only support decimal values less than 20
        THROW(EXCEPTION_MALFORMED_APDU);
    }
}

void validate_token_transfer() {
    // One token transfer with two accountAmounts
    if (ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
            .transfers_count != 2) {
        THROW(EXCEPTION_MALFORMED_APDU);
    }

    // Transactions fail if not given in the right denomination
    validate_decimals(ctx.transaction.data.cryptoTransfer.tokenTransfers[ 0 ]
                          .expected_decimals.value);
}
