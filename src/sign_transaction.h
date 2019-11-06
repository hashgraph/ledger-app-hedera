#ifndef LEDGER_APP_HEDERA_SIGN_TRANSACTION_H
#define LEDGER_APP_HEDERA_SIGN_TRANSACTION_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <printf.h>
#include <pb.h>
#include <pb_decode.h>

#include "globals.h"
#include "debug.h"
#include "errors.h"
#include "handlers.h"
#include "hedera.h"
#include "io.h"
#include "TransactionBody.pb.h"
#include "utils.h"
#include "ui.h"

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
