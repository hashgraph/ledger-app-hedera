#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pb.h>
#include <pb_decode.h>

#include "printf.h"
#include "globals.h"
#include "glyphs.h"
#include "ux.h"
#include "debug.h"
#include "errors.h"
#include "handlers.h"
#include "hedera.h"
#include "io.h"
#include "TransactionBody.pb.h"
#include "utils.h"
#include "ui_flows.h"
#include "ui_common.h"
#include "sign_transaction.h"
#include "hedera_format.h"

#define BUF_SIZE 32

static char *hedera_format_amount(uint64_t amount, uint8_t decimals) {
    static char buf[BUF_SIZE];

    // NOTE: format of amounts are not sensitive
    memset(buf, 0, BUF_SIZE);

    // Quick shortcut if the amount is zero
    // Regardless of decimals, the output is always "0"
    if (amount == 0) {
        buf[0] = '0';
        buf[1] = '\0';

        return buf;
    }

    // NOTE: we silently fail with a decimal value > 20
    //  this function shuold only be called on decimal values smaller than 20
    if (decimals >= 20) return buf;

    int i = 0;

    while (i < (BUF_SIZE - 1) && (amount > 0 || i < decimals)) {
        int digit = amount % 10;
        amount /= 10;

        buf[i++] = '0' + digit;

        if (i == decimals) {
            buf[i++] = '.';
        }
    }

    if (buf[i - 1] == '.') {
        buf[i++] = '0';
    }

    int size = i;
    int j = 0;
    char tmp;

    while (j < i) {
        i -= 1;

        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;

        j += 1;
    }

    for (j = size - 1; j > 0; j--) {
        if (buf[j] == '0') {
            continue;
        } else if (buf[j] == '.') {
            break;
        } else {
            j += 1;
            break;
        }
    }

    if (j < size - 1) {
        buf[j] = '\0';
    }

    return buf;
}

static char *hedera_format_tinybar(uint64_t tinybar) {
    return hedera_format_amount(tinybar, 8);
}

static void validate_decimals(uint32_t decimals) {
    if (decimals >= 20) {
        // We only support decimal values less than 20
        THROW(EXCEPTION_MALFORMED_APDU);
    }
}

#define hedera_safe_printf(element, ...) hedera_snprintf(element, sizeof(element) - 1, __VA_ARGS__)

#if defined(TARGET_NANOS)

static void set_title(const char *title) {
    hedera_safe_printf(
        st_ctx.title,
        "%s (%u/%u)",
        title,
        st_ctx.display_index,
        st_ctx.display_count
    );
}

#endif

static void set_senders_title(const char *title) {
    hedera_safe_printf(
        st_ctx.senders_title,
#if defined(TARGET_NANOS)
        "%s (%u/%u)",
        title,
        st_ctx.display_index,
        st_ctx.display_count
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
        "%s",
        title
#endif
    );
}

static void set_amount_title(const char *title) {
    hedera_safe_printf(
        st_ctx.amount_title,
#if defined(TARGET_NANOS)
        "%s (%u/%u)",
        title,
        st_ctx.display_index,
        st_ctx.display_count
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
        "%s",
        title
#endif
    );
}

void reformat_key(void) {
    hedera_safe_printf(
        st_ctx.summary_line_2,
        "with Key #%u?",
        st_ctx.key_index
    );
}

void reformat_summary(const char *summary) {
    hedera_safe_printf(st_ctx.summary_line_1, summary);
}

void reformat_summary_send_token(void) {
    hedera_safe_printf(
        st_ctx.summary_line_1,
        "Send %llu.%llu.%llu",
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].token.shardNum,
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].token.realmNum,
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].token.tokenNum
    );
}

void reformat_operator(void) {
    hedera_safe_printf(
        st_ctx.operator,
        "%llu.%llu.%llu",
        st_ctx.transaction.transactionID.accountID.shardNum,
        st_ctx.transaction.transactionID.accountID.realmNum,
        st_ctx.transaction.transactionID.accountID.accountNum
    );

#if defined(TARGET_NANOS)
    set_title("Operator");
#endif
}

void reformat_fee(void) {
    hedera_safe_printf(
        st_ctx.fee,
        "%s hbar",
        hedera_format_tinybar(st_ctx.transaction.transactionFee)
    );

#if defined(TARGET_NANOS)
    set_title("Max Fee");
#endif
}

void reformat_memo(void) {
    hedera_safe_printf(
        st_ctx.memo,
        "%s",
        (st_ctx.transaction.memo[0] != '\0') ? st_ctx.transaction.memo : ""
    );

    if (strlen(st_ctx.memo) > MAX_MEMO_SIZE) {
        // :grimacing:
        THROW(EXCEPTION_MALFORMED_APDU);
    }

#if defined(TARGET_NANOS)
    set_title("Memo");
#endif
}

void reformat_amount_balance(void) {
    hedera_safe_printf(
        st_ctx.amount,
        "%s hbar",
        hedera_format_tinybar(st_ctx.transaction.data.cryptoCreateAccount.initialBalance)
    );

    set_amount_title("Balance");
}

void reformat_amount_transfer(void) {
    hedera_safe_printf(
        st_ctx.amount,
        "%s hbar",
        hedera_format_tinybar(st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[st_ctx.transfer_to_index].amount)
    );

    set_amount_title("Amount");
}

void reformat_amount_burn(void) {
    validate_decimals(st_ctx.transaction.data.tokenBurn.expected_decimals.value);
    hedera_safe_printf(
        st_ctx.amount,
        "%s",
        hedera_format_amount(
            st_ctx.transaction.data.tokenBurn.amount,
            st_ctx.transaction.data.tokenBurn.expected_decimals.value
        )
    );

    set_amount_title("Amount");
}

void reformat_amount_mint(void) {
    validate_decimals(st_ctx.transaction.data.tokenMint.expected_decimals.value);
    hedera_safe_printf(
        st_ctx.amount,
        "%s",
        hedera_format_amount(
            st_ctx.transaction.data.tokenMint.amount,
            st_ctx.transaction.data.tokenMint.expected_decimals.value
        )
    );

    set_amount_title("Amount");
}

void reformat_token_tranfer(void) {
    validate_decimals(st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].expected_decimals.value);
    hedera_safe_printf(
        st_ctx.amount,
        "%s",
        hedera_format_amount(
            st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].transfers[st_ctx.transfer_to_index].amount,
            st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].expected_decimals.value
        )
    );

    set_amount_title("Amount");
}

void reformat_token_associate(void) {
    hedera_safe_printf(
        st_ctx.senders,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.tokenAssociate.tokens[0].shardNum,
        st_ctx.transaction.data.tokenAssociate.tokens[0].realmNum,
        st_ctx.transaction.data.tokenAssociate.tokens[0].tokenNum
    );

    set_senders_title("Token");
}

void reformat_token_mint(void) {
    hedera_safe_printf(
        st_ctx.senders,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.tokenMint.token.shardNum,
        st_ctx.transaction.data.tokenMint.token.realmNum,
        st_ctx.transaction.data.tokenMint.token.tokenNum
    );

    set_senders_title("Token");
}

void reformat_token_burn(void) {
    hedera_safe_printf(
        st_ctx.senders,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.tokenBurn.token.shardNum,
        st_ctx.transaction.data.tokenBurn.token.realmNum,
        st_ctx.transaction.data.tokenBurn.token.tokenNum
    );

    set_senders_title("Token");
}

void reformat_verify_account() {
    hedera_safe_printf(
        st_ctx.senders,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.shardNum,
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.realmNum,
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.accountNum
    );

    set_senders_title("Account");
}

void reformat_sender_account(void) {
    hedera_safe_printf(
        st_ctx.senders,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[st_ctx.transfer_from_index].accountID.shardNum,
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[st_ctx.transfer_from_index].accountID.realmNum,
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[st_ctx.transfer_from_index].accountID.accountNum
    );

    set_senders_title("Sender");
}

void reformat_recipient_account(void) {
    hedera_safe_printf(
        st_ctx.recipients,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[st_ctx.transfer_to_index].accountID.shardNum,
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[st_ctx.transfer_to_index].accountID.realmNum,
        st_ctx.transaction.data.cryptoTransfer.transfers.accountAmounts[st_ctx.transfer_to_index].accountID.accountNum
    );

#if defined(TARGET_NANOS)
    set_title("Recipient");
#endif
}

void reformat_tokens_account_sender(void) {
    hedera_safe_printf(
        st_ctx.senders,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].transfers[st_ctx.transfer_from_index].accountID.shardNum,
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].transfers[st_ctx.transfer_from_index].accountID.realmNum,
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].transfers[st_ctx.transfer_from_index].accountID.accountNum
    );

    set_senders_title("Sender");
}

void reformat_tokens_account_recipient(void) {
    hedera_safe_printf(
        st_ctx.recipients,
        "%llu.%llu.%llu",
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].transfers[st_ctx.transfer_to_index].accountID.shardNum,
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].transfers[st_ctx.transfer_to_index].accountID.realmNum,
        st_ctx.transaction.data.cryptoTransfer.tokenTransfers[0].transfers[st_ctx.transfer_to_index].accountID.accountNum
    );

#if defined(TARGET_NANOS)
    set_title("Recipient");
#endif
}
