#pragma once

#include <stdint.h>

// Forward declare to avoid including os.h in a header file
struct cx_ecfp_256_public_key_s;
struct cx_ecfp_256_private_key_s;

bool hedera_derive_keypair(uint32_t index,
                           /* out */ struct cx_ecfp_256_private_key_s* secret,
                           /* out */ struct cx_ecfp_256_public_key_s* public);

bool hedera_sign(uint32_t index, const uint8_t* tx, uint8_t tx_len,
                 /* out */ uint8_t* result);
