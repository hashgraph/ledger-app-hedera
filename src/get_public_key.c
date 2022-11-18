#include "get_public_key.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug.h"
#include "errors.h"
#include "globals.h"
#include "handlers.h"
#include "hedera.h"
#include "io.h"
#include "printf.h"
#include "ui_flows.h"
#include "utils.h"

get_public_key_context_t gpk_ctx;

static void get_pk() {
    // Derive Key
    hedera_derive_keypair(gpk_ctx.key_index, NULL, &gpk_ctx.public);

    // Put Key bytes in APDU buffer
    public_key_to_bytes(G_io_apdu_buffer, &gpk_ctx.public);

    // Populate Key Hex String
    bin2hex(gpk_ctx.full_key, G_io_apdu_buffer, KEY_SIZE);
    gpk_ctx.full_key[ KEY_SIZE ] = '\0';
}

void handle_get_public_key(uint8_t p1, uint8_t p2, uint8_t* buffer,
                           uint16_t len,
                           /* out */ volatile unsigned int* flags,
                           /* out */ volatile unsigned int* tx) {
    UNUSED(p2);
    UNUSED(len);
    UNUSED(tx);

    // Read Key Index
    gpk_ctx.key_index = U4LE(buffer, 0);

    // If p1 != 0, silent mode, for use by apps that request the user's public
    // key frequently Only do UI actions for p1 == 0
    if (p1 == 0) {
        // Complete "Export Public | Key #x?"
        hedera_snprintf(gpk_ctx.ui_approve_l2, DISPLAY_SIZE, "Key #%u?",
                        gpk_ctx.key_index);
    }

    // Populate context with PK
    get_pk();

    if (p1 == 0) {
        ui_get_public_key();
    }

    // Normally happens in approve export public key handler
    if (p1 != 0) {
        io_exchange_with_code(EXCEPTION_OK, 32);
        ui_idle();
    }

    *flags |= IO_ASYNCH_REPLY;
}
