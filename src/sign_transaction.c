#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "os.h"
#include "os_io_seproxyhal.h"
#include "errors.h"
#include "io.h"
#include "ui.h"
#include "hedera.h"
#include "utils.h"

// Define context for UI interaction
static struct sign_tx_context_t {
    uint32_t key_index;
    uint8_t buffer[32];
    char* sign_str = "Sign Transaction With";
    char key_str[40];
} ctx;

// Define Bagel for Sign Transaction Confirmation
static const bagl_element_t ui_sign_tx_approve[] = {
        UI_BACKGROUND(),
        UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
        UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

        // X                            O
        //      Sign Transaction With
        //      Key #123456?

        UI_TEXT(0x00, 12, 128, ctx.sign_str);
        UI_TEXT(0x00, 26, 128, ctx.key_str);
};

// Button handler for ui_sign_tx_approve
static unsigned int ui_sign_tx_approve(unsigned int button_mask, unsigned int button_mask_counter) {
    uint16_t = 0;

    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:  // Reject
            io_exchange_with_code(SW_USER_REJECTED, tx);
            ui_idle();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:  // Approve
            derive_and_sign(ctx.key_index, ctx.buffer, G_io_apdu_buffer)  // Derive Secrets, Sign
            tx += 64;
            io_exchange_with_code(SW_OK, tx);  // flush
            ui_idle();
            break;
    }
    return 0;
}

// Handle parsing APDU and displaying UI element
void handle_sign_transaction(
    uint8_t p1,
    uint8_t p2,
    uint8_t* buffer,
    uint16_t len,
    /* out */ volatile unsigned int* flags,
    /* out */ volatile unsigned int* tx
) {
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(len);
    UNUSED(tx);

    // Get Key Index and Prepare Message
    ctx.key_index = U4LE(buffer, 0);
    snprintf(ctx.key_str, 40, "Key #%d?", ctx.key_index);

    // Get transaction data
    os_memmove(ctx.buffer, buffer + 4, sizeof(ctx.buffer));

    // Display Confirmation Screen
    UX_DISPLAY(ui_sign_tx_approve, NULL);

    *flags |= IO_ASYNC_REPLY;
}
