#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

#if defined(TARGET_NANOS)

unsigned int ui_tx_approve_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

#elif defined(TARGET_NANOX)

unsigned int io_seproxyhal_confirm_tx_approve(const bagl_element_t *e);
unsigned int io_seproxyhal_confirm_tx_reject(const bagl_element_t *e);

#endif // TARGET

void handle_transaction_body();

#endif //LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
