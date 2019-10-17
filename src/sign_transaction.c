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
    char line_2[40];
    uint8_t transaction;
} ctx;

// Define Bagel for Sign Transaction Confirmation
static const bagl_element_t ui_sign_tx_approve[] = {
        UI_BACKGROUND(),
        UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
        UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

        // X                            O
        //      Sign Transaction
        //      with Key #0?

        UI_TEXT(0x00, 0, 12, 128, "Sign Transaction"),
        UI_TEXT(0x00, 0, 26, 128, ctx.line_2)
};

// Button handler for ui_sign_tx_approve
static unsigned int ui_sign_tx_approve_button(unsigned int button_mask, unsigned int button_mask_counter) {
    uint16_t tx = 0;

    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:  // Reject
            io_exchange_with_code(EXCEPTION_USER_REJECTED, tx);
            ui_idle();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:  // Approve
            tx += hedera_sign(
                ctx.key_index, 
                ctx.transaction, 
                sizeof(ctx.transaction) / sizeof(uint8_t), 
                G_io_apdu_buffer
            );
            io_exchange_with_code(EXCEPTION_OK, tx);  // flush
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
    snprintf(ctx.line_2, 40, "with Key #%d?", ctx.key_index);

    // TODO: Use P1_MORE to accept a streaming body (for > max(APDU))
    // Extract Transaction Message
    os_memset(ctx.transaction, 0, sizeof(ctx.transaction));
    os_memcpy(ctx.transaction, buffer + 4, (len - 4) * sizeof(uint8_t));

    // Display Confirmation Screen
    UX_DISPLAY(ui_sign_tx_approve, NULL);

    *flags |= IO_ASYNCH_REPLY;
}
