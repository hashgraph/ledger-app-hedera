#include "hedera.h"

void hedera_derive_keypair(
    uint32_t index,
    /* out */ cx_ecfp_private_key_t* secret, 
    /* out */ cx_ecfp_public_key_t* public
) {
    uint8_t seed[32];
    cx_ecfp_private_key_t pk;

    // bip32 path for 44'/3030'/n'/0'/0'
    uint32_t path[] = {44 | 0x80000000, 3030 | 0x80000000, index | 0x80000000, 0x80000000, 0x80000000};
    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, path, 5, seed, NULL, (unsigned char*) "ed25519 seed", 12);

    cx_ecfp_init_private_key(CX_CURVE_Ed25519, seed, sizeof(seed), &pk);

    if (public) {
        cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, public);
        cx_ecfp_generate_pair(CX_CURVE_Ed25519, public, &pk, 1);
    }

    if (secret) {
        *secret = pk;
    }

    os_memset(seed, 0, sizeof(seed));
    os_memset(&pk, 0, sizeof(pk));
}

void derive_and_sign(
        uint32_t index,
        const uint8_t* buffer,
        /* out */ uint8_t* result
        ) {

    // Get Keys
    cx_ecfp_private_key_t private_key;
    hedera_derive_keypair(index, &private_key, NULL);

    // Sign Transaction
    // <cx.h> 2283
    cx_eddsa_sign(
        &private_key,                    // private key
        0,                               // mode (UNSUPPORTED)
        CX_SHA512,                       // hashID (???)
        buffer,                          // hash
        sizeof(buffer)/sizeof(uint8_t),  // hash length
        NULL,                            // context (UNUSED)
        0,                               // context length (0)
        result,                          // signature
        64,                              // signature length
        0                                // info (0)
    );

    // Overwrite Private Key
    os_memset(&private_key, 1, sizeof(private_key));
    os_memset(&private_key, 0, sizeof(private_key));
}