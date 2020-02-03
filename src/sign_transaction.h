#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

#if defined(TARGET_NANOS)
// Forward declarations for Nano S UI
// Step 1
static const bagl_element_t* ui_prepro_tx_summary_step(
    const bagl_element_t* element
);
unsigned int ui_tx_summary_step_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

// Step 2 - 4
unsigned int ui_tx_intermediate_step_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

// Step 5
unsigned int ui_tx_confirm_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

// Step 6
unsigned int ui_tx_deny_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

uint8_t num_screens(size_t length);
void reformat_amount();
void reformat_fee();
void reformat_memo();
void setup_nanos_paging();

#elif defined(TARGET_NANOX)
// Forward declarations for Nano X UI
unsigned int io_seproxyhal_tx_approve(const bagl_element_t* e);
unsigned int io_seproxyhal_tx_reject(const bagl_element_t* e);

#endif // TARGET

void handle_transaction_body();

#endif //LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
