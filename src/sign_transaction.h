#pragma once

#include "proto/TransactionBody.pb.h"

enum TransactionStep {
    Summary = 1,
    Operator = 2,
    Senders = 3,
    Recipients = 4,
    Amount = 5,
    Fee = 6,
    Memo = 7,
    Confirm = 8,
    Deny = 9
};

enum TransactionType {
    Unknown = -1,
    Verify = 0,
    Create = 1,
    Transfer = 2,
    Associate = 3,
    TokenTransfer = 4,
    TokenMint = 5,
    TokenBurn = 6,
};

typedef struct sign_tx_context_s {
    // ui common
    uint32_t key_index;
    uint8_t transfer_to_index;
    uint8_t transfer_from_index;

    // Transaction Summary
    char summary_line_1[ DISPLAY_SIZE + 1 ];
    char summary_line_2[ DISPLAY_SIZE + 1 ];

#if defined(TARGET_NANOS)
    union {
#define TITLE_SIZE (DISPLAY_SIZE + 1)
        char title[ TITLE_SIZE ];
        char senders_title[ TITLE_SIZE ]; // alias for title
        char amount_title[ TITLE_SIZE ];  // alias for title
    };
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
    char senders_title[ DISPLAY_SIZE + 1 ];
    char amount_title[ DISPLAY_SIZE + 1 ];
#endif

    // Account ID: uint64_t.uint64_t.uint64_t
    // Most other entities are shorter
#if defined(TARGET_NANOS)
    union {
#define FULL_SIZE (ACCOUNT_ID_SIZE + 1)
        char full[ FULL_SIZE ];
        char operator[ FULL_SIZE ];   // alias for full
        char senders[ FULL_SIZE ];    // alias for full
        char recipients[ FULL_SIZE ]; // alias for full
        char amount[ FULL_SIZE ];     // alias for full
        char fee[ FULL_SIZE ];        // alias for full
        char memo[ FULL_SIZE ];       // alias for full
    };
    char partial[ DISPLAY_SIZE + 1 ];
#endif

    // Steps correspond to parts of the transaction proto
    // type is set based on proto
#if defined(TARGET_NANOS)
    enum TransactionStep step;
#endif
    enum TransactionType type;

#if defined(TARGET_NANOS)
    uint8_t display_index; // 1 -> Number Screens
    uint8_t display_count; // Number Screens
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
    // Transaction Operator
    char operator[ DISPLAY_SIZE * 2 + 1 ];

    // Transaction Senders
    char senders[ DISPLAY_SIZE * 2 + 1 ];

    // Transaction Recipients
    char recipients[ DISPLAY_SIZE * 2 + 1 ];

    // Transaction Amount
    char amount[ DISPLAY_SIZE * 2 + 1 ];

    // Transaction Fee
    char fee[ DISPLAY_SIZE * 2 + 1 ];

    // Transaction Memo
    char memo[ MAX_MEMO_SIZE + 1 ];
#endif

    // Parsed transaction
    HederaTransactionBody transaction;
} sign_tx_context_t;

extern sign_tx_context_t st_ctx;
