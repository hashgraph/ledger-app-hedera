#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

#if defined(TARGET_NANOS)
// Forward declarations for Nano S UI
// Step 1
unsigned int ui_tx_summary_step_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

// Step 2
unsigned int ui_tx_amount_step_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

// Step 3
unsigned int ui_tx_fee_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

// Step 4
unsigned int ui_tx_confirm_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

// Step 5
unsigned int ui_tx_deny_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

uint8_t num_screens(char* text);
void reformat_amount();
void reformat_fee();
void setup_nanos_paging();

#elif defined(TARGET_NANOX)
// Forward declarations for Nano X UI
unsigned int io_seproxyhal_tx_approve(const bagl_element_t* e);
unsigned int io_seproxyhal_tx_reject(const bagl_element_t* e);

#endif // TARGET

void handle_transaction_body();

#endif //LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
