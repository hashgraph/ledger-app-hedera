#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

#if defined(TARGET_NANOS)

unsigned int ui_tx_approve_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

void handle_sign_transaction_nanos();

#elif defined(TARGET_NANOX)

void handle_sign_transaction_nanox();

#endif // TARGET

#endif //LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
