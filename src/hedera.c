#include <os.h>
#include <cx.h>
#include "globals.h"
#include "printf.h"
#include "hedera.h"
#include "string.h"

void hedera_derive_keypair(
    uint32_t index,
    /* out */ cx_ecfp_private_key_t* secret, 
    /* out */ cx_ecfp_public_key_t* public
) {
    static uint8_t seed[32];
    static uint32_t path[5];
    static cx_ecfp_private_key_t pk;

    path[0] = 44 | 0x80000000;
    path[1] = 3030 | 0x80000000;
    path[2] = 0x80000000;
    path[3] = 0x80000000;
    path[4] = index | 0x80000000;

    os_perso_derive_node_bip32_seed_key(
        HDW_ED25519_SLIP10, 
        CX_CURVE_Ed25519, 
        path, 
        5, 
        seed, 
        NULL, 
        NULL, 
        0
    );

    cx_ecfp_init_private_key(
        CX_CURVE_Ed25519, 
        seed, 
        sizeof(seed), 
        &pk
    );

    if (public) {
        cx_ecfp_init_public_key(
            CX_CURVE_Ed25519, 
            NULL, 
            0, 
            public
        );
        cx_ecfp_generate_pair(
            CX_CURVE_Ed25519, 
            public, 
            &pk, 
            1
        );
    }

    if (secret) {
        *secret = pk;
    }

    explicit_bzero(seed, sizeof(seed));
    explicit_bzero(&pk, sizeof(pk));
}

void hedera_sign(
    uint32_t index,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result
) {
    static cx_ecfp_private_key_t pk;

    // Get Keys
    hedera_derive_keypair(index, &pk, NULL);

    // Sign Transaction
    // <cx.h> 2283
    // Claims to want Hashes, but other apps use the message itself
    // and complain that the documentation is wrong
    cx_eddsa_sign(
        &pk,                             // private key
        0,                               // mode (UNSUPPORTED)
        CX_SHA512,                       // hashID
        tx,                              // hash (really message)
        tx_len,                          // hash length (really message length)
        NULL,                            // context (UNUSED)
        0,                               // context length (0)
        result,                          // signature
        64,                              // signature length
        NULL                             // info
    );

    // Clear private key
    explicit_bzero(&pk, sizeof(pk));
}

char* hedera_format_tinybar(uint64_t tinybar) {
    return hedera_format_amount(tinybar, 8);
}

char* hedera_format_amount(uint64_t amount, uint8_t decimals) {
    static const int BUF_SIZE = 32;
    static char buf[BUF_SIZE];

    explicit_bzero(buf, BUF_SIZE);

    int i = 0;

    while (i < (BUF_SIZE - 1) && (amount > 0 || i < decimals)) {
        int digit = amount % 10;
        amount /= 10;

        buf[i++] = '0' + digit;

        if (i == decimals) {
            buf[i++] = '.';
        }
    }

    if (buf[i - 1] == '.') {
        buf[i++] = '0';
    }

    int j = 0;
    char tmp;

    while (j < i) {
        j += 1;
        i -= 1;

        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
    }

    return buf;
}


