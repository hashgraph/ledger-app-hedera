#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

enum TransactionStep
{
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

enum TransactionType
{
  Unknown = -1,
  Verify = 0,
  Create = 1,
  Transfer = 2,
  Associate = 3,
  TokenTransfer = 4,
  TokenMint = 5,
  TokenBurn = 6,
};

void handle_transaction_body();
void validate_decimals(uint32_t decimals);

#if defined(TARGET_NANOS)
// Transaction Context
static struct sign_tx_context_t
{
  // ui common
  uint32_t key_index;
  uint8_t transfer_to_index;
  uint8_t transfer_from_index;

  // Transaction Summary
  char summary_line_1[DISPLAY_SIZE + 1];
  char summary_line_2[DISPLAY_SIZE + 1];
  char title[DISPLAY_SIZE + 1];

  // Account ID: uint64_t.uint64_t.uint64_t
  // Most other entities are shorter
  char full[ACCOUNT_ID_SIZE + 1];
  char partial[DISPLAY_SIZE + 1];

  // Steps correspond to parts of the transaction proto
  // type is set based on proto
  enum TransactionStep step;
  enum TransactionType type;

  uint8_t display_index; // 1 -> Number Screens
  uint8_t display_count; // Number Screens

  // Parsed transaction
  HederaTransactionBody transaction;
} ctx;

// Forward declarations for Nano S UI
// Step 1
unsigned int ui_tx_summary_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter);

// Step 2 - 7
void handle_intermediate_left_press();
void handle_intermediate_right_press();
unsigned int ui_tx_intermediate_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter);

// Step 8
unsigned int ui_tx_confirm_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter);

// Step 9
unsigned int ui_tx_deny_step_button(
    unsigned int button_mask,
    unsigned int button_mask_counter);

uint8_t num_screens(size_t length);
void count_screens();
void shift_display();
bool first_screen();
bool last_screen();
void reformat_token();
void reformat_tokens_accounts(char *title_part, uint8_t transfer_index);
void reformat_accounts(char *title_part, uint8_t transfer_index);
void reformat_operator();
void reformat_senders();
void reformat_recipients();
void reformat_amount();
void reformat_fee();
void reformat_memo();

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
// Transaction Context
static struct sign_tx_context_t
{
  // ui common
  uint32_t key_index;
  uint8_t transfer_from_index;
  uint8_t transfer_to_index;

  // Transaction Summary
  char summary_line_1[DISPLAY_SIZE + 1];
  char summary_line_2[DISPLAY_SIZE + 1];
  char senders_title[DISPLAY_SIZE + 1];
  char amount_title[DISPLAY_SIZE + 1];
  char partial[DISPLAY_SIZE + 1];

  enum TransactionType type;

  // Transaction Operator
  char operator[DISPLAY_SIZE * 2 + 1];

  // Transaction Senders
  char senders[DISPLAY_SIZE * 2 + 1];

  // Transaction Recipients
  char recipients[DISPLAY_SIZE * 2 + 1];

  // Transaction Amount
  char amount[DISPLAY_SIZE * 2 + 1];

  // Transaction Fee
  char fee[DISPLAY_SIZE * 2 + 1];

  // Transaction Memo
  char memo[MAX_MEMO_SIZE + 1];

  // Parsed transaction
  HederaTransactionBody transaction;
} ctx;

// Forward declarations for Nano X UI
unsigned int io_seproxyhal_tx_approve(const bagl_element_t *e);
unsigned int io_seproxyhal_tx_reject(const bagl_element_t *e);

#endif // TARGET

#endif // LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
