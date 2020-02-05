#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

enum TransactionStep {
    Summary = 1,
    Operator = 2,
    Senders = 3,
    Recipients = 4, 
    Amount = 5,
    Fee = 6,
    Memo =  7,
    Confirm = 8,
    Deny = 9
};

enum TransactionType {
    Unknown = -1,
    Verify = 0,
    Create = 1,
    Transfer = 2
};

#if defined(TARGET_NANOS)
// Forward declarations for Nano S UI
// Step 1
unsigned int ui_tx_summary_step_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

// Step 2 - 7
void handle_intermediate_left_press();
void handle_intermediate_right_press();
unsigned int ui_tx_intermediate_step_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
);

// Step 8
unsigned int ui_tx_confirm_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

// Step 9
unsigned int ui_tx_deny_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
);

uint8_t num_screens(size_t length);
void count_screens();
void shift_display();
bool first_screen();
bool last_screen();
void reformat_operator();
void reformat_senders();
void reformat_recipients();
void reformat_amount();
void reformat_fee();
void reformat_memo();

#elif defined(TARGET_NANOX)
// Forward declarations for Nano X UI
void x_start_tx_loop();
void x_continue_tx_loop();
void x_end_tx_loop();
unsigned int io_seproxyhal_tx_approve(const bagl_element_t* e);
unsigned int io_seproxyhal_tx_reject(const bagl_element_t* e);

#endif // TARGET

void handle_transaction_body();

#endif //LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
