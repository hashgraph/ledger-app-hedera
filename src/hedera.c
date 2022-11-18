#include "hedera.h"

#include <cx.h>
#include <os.h>
#include <string.h>

#include "globals.h"
#include "utils.h"

bool hedera_derive_keypair(uint32_t index,
                           /* out */ cx_ecfp_private_key_t* secret,
                           /* out */ cx_ecfp_public_key_t* public) {
    static uint8_t seed[ 32 ];
    static uint32_t path[ 5 ];
    static cx_ecfp_private_key_t pk;

    path[ 0 ] = 44 | 0x80000000;
    path[ 1 ] = 3030 | 0x80000000;
    path[ 2 ] = 0x80000000;
    path[ 3 ] = 0x80000000;
    path[ 4 ] = index | 0x80000000;

    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519,
                                        path, 5, seed, NULL, NULL, 0);

    if (CX_OK != cx_ecfp_init_private_key_no_throw(CX_CURVE_Ed25519, seed,
                                                   sizeof(seed), &pk)) {
        MEMCLEAR(seed);
        return false;
    }

    if (public) {
        if (CX_OK != cx_ecfp_init_public_key_no_throw(CX_CURVE_Ed25519, NULL, 0,
                                                      public)) {
            MEMCLEAR(seed);
            MEMCLEAR(pk);
            return false;
        }

        if (CX_OK !=
            cx_ecfp_generate_pair_no_throw(CX_CURVE_Ed25519, public, &pk, 1)) {
            MEMCLEAR(seed);
            MEMCLEAR(pk);
            return false;
        }
    }

    if (secret) {
        *secret = pk;
    }

    MEMCLEAR(seed);
    MEMCLEAR(pk);

    return true;
}

bool hedera_sign(uint32_t index, const uint8_t* tx, uint8_t tx_len,
                 /* out */ uint8_t* result) {
    static cx_ecfp_private_key_t pk;

    // Get Keys
    if (!hedera_derive_keypair(index, &pk, NULL)) {
        return false;
    }

    // Sign Transaction
    // <cx.h> 2283
    // Claims to want Hashes, but other apps use the message itself
    // and complain that the documentation is wrong
    if (CX_OK != cx_eddsa_sign_no_throw(
                     &pk,       // private key
                     CX_SHA512, // hashID
                     tx,        // hash (really message)
                     tx_len,    // hash length (really message length)
                     result,    // signature
                     64         // signature length
                     )) {
        MEMCLEAR(pk);
        return false;
    }

    // Clear private key
    MEMCLEAR(pk);

    return true;
}
