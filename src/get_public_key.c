#include <stddef.h>
#include <stdint.h>
#include "os.h"
#include "os_io_seproxyhal.h"
#include "errors.h"
#include "io.h"
#include "ui.h"
#include "hedera.h"
#include "utils.h"

static struct get_public_key_context_t {
    uint32_t key_index;
    char key_str[40]; // variable-length
} ctx;

// Define the approval screen. This is where the user will approve the
// generation of the public key.
static const bagl_element_t ui_get_public_key_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

    // These two lines form a complete sentence:
    //
    //    Generate Public
    //       Key #123?
    //

    UI_TEXT(0x00, 0, 12, 128, "Export Public"),
    UI_TEXT(0x00, 0, 26, 128, ctx.key_str),
};

// This is the button handler for the approval screen. If the user approves,
// it generates and sends the public key.
static unsigned int ui_get_public_key_approve_button(unsigned int button_mask, unsigned int button_mask_counter) {
    cx_ecfp_public_key_t public;
    uint16_t tx = 0;

    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: // REJECT
            io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
            ui_idle();
            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // APPROVE
            // Derive the public key and store them in the APDU buffer.
            hedera_derive_keypair(ctx.key_index, NULL, &public);
            public_key_to_bytes(G_io_apdu_buffer, &public);
            tx += 32;

            // Flush the APDU buffer, sending the response.
            io_exchange_with_code(EXCEPTION_OK, tx);

            // The user now has the public key, return to the idle screen.
            ui_idle();

            break;
    }

    return 0;
}

// A high-level description of getPublicKey is as follows. The user initiates
// the command on their computer by requesting the generation of a specific
// public key. The command handler then displays a screen asking the user to
// confirm the action. If the user presses the 'approve' button, the requested
// key is generated, sent to the computer, and displayed on the device. The
// user may then visually compare the key shown on the device to the key
// received by the computer.

void handle_get_public_key(
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

    // Read the key index from the buffer
    ctx.key_index = U4LE(buffer, 0);

    // Prepare the approval screen
    os_memmove(ctx.key_str, "Key #", 5);
    int n = bin2dec(ctx.key_str+5, ctx.key_index);
    os_memmove(ctx.key_str+5+n, "?", 2);

    UX_DISPLAY(ui_get_public_key_approve, NULL);

    *flags |= IO_ASYNCH_REPLY;
}
