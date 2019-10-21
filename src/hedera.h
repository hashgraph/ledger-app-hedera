#ifndef LEDGER_HEDERA_HEDERA_H
#define LEDGER_HEDERA_HEDERA_H 1

#include <stddef.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>

#include <os.h>
#include <os_io_seproxyhal.h>
#include <cx.h>

#include "errors.h"
#include "io.h"
#include "TransactionBody.pb.h"
#include "utils.h"
#include "ui.h"

#define MAX_TX_SIZE 2048

extern void hedera_derive_keypair(
    uint32_t index,
    /* out */ cx_ecfp_private_key_t* secret, 
    /* out */ cx_ecfp_public_key_t* public
);

extern uint16_t hedera_sign(
    uint32_t index,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result
);

#endif // LEDGER_HEDERA_HEDERA_H
