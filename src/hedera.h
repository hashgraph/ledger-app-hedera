#pragma once

#include <stdint.h>
#include <os.h>
#include <cx.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "os.h"
#include "os_io_seproxyhal.h"
#include "errors.h"
#include "io.h"
#include "ui.h"
#include "utils.h"
#include "TransactionBody.pb.h"

#define MAX_TX_SIZE 1024

extern void hedera_derive_keypair(
    uint32_t index,
    /* out */ cx_ecfp_private_key_t* private_key, 
    /* out */ cx_ecfp_public_key_t* public_key
);

extern uint16_t hedera_sign(
    uint32_t index,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result
);
