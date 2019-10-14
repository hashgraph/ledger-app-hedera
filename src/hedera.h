#pragma once

#include <stdint.h>
#include <os.h>
#include <cx.h>

extern void hedera_derive_keypair(
    uint32_t index,
    /* out */ cx_ecfp_private_key_t* private_key, 
    /* out */ cx_ecfp_public_key_t* public_key
);
