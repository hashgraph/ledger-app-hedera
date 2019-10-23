#ifndef LEDGER_HEDERA_HEDERA_H
#define LEDGER_HEDERA_HEDERA_H 1

#include <stdint.h>

#define MAX_TX_SIZE 512

// Forward declare to avoid including os.h in a header file

struct cx_ecfp_256_public_key_s;

struct cx_ecfp_256_private_key_s;

extern void hedera_derive_keypair(
    uint32_t index,
    /* out */ struct cx_ecfp_256_private_key_s* secret, 
    /* out */ struct cx_ecfp_256_public_key_s* public
);

extern uint16_t hedera_sign(
    uint32_t index,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result
);

extern char* hedera_format_tinybar(uint64_t tinybar);

#endif // LEDGER_HEDERA_HEDERA_H
