#include "get_public_key.h"

#if defined(TARGET_NANOS)

void shift_partial_key() {
    os_memmove(
        ctx.partial_key,
        ctx.full_key + ctx.display_index,
        DISPLAY_SIZE
    );
}

static unsigned int ui_get_public_key_compare_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
        case BUTTON_LEFT: // Left
        case BUTTON_EVT_FAST | BUTTON_LEFT:
            if (ctx.display_index > 0) ctx.display_index--;
            shift_partial_key();
            UX_REDISPLAY();
            break;
        case BUTTON_RIGHT: // Right
        case BUTTON_EVT_FAST | BUTTON_RIGHT:
            if (ctx.display_index < KEY_SIZE - DISPLAY_SIZE) ctx.display_index++;
            shift_partial_key();
            UX_REDISPLAY();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // Continue
            ui_idle();
            break;
    }
    return 0;
}

static const bagl_element_t* ui_prepro_get_public_key_compare(
    const bagl_element_t* element
) {
    if (element->component.userid == LEFT_ID 
        && ctx.display_index == 0)
        return NULL; // Hide Left Arrow at Left Edge
    if (element->component.userid == RIGHT_ID 
        && ctx.display_index == KEY_SIZE - DISPLAY_SIZE) 
        return NULL; // Hide Right Arrow at Right Edge
    return element;
}

void send_pk() {
    // Derive Key
    hedera_derive_keypair(ctx.key_index, NULL, &ctx.public);
    
    // Put Key bytes in APDU buffer
    public_key_to_bytes(G_io_apdu_buffer, &ctx.public);
    
    // Flush
    io_exchange_with_code(EXCEPTION_OK, 32);
}

void compare_pk() {
    // init full key str from apdu bytes
    bin2hex(ctx.full_key, G_io_apdu_buffer, KEY_SIZE);
    ctx.full_key[KEY_SIZE] = '\0';

    // init partial key str from full str
    os_memmove(ctx.partial_key, ctx.full_key, DISPLAY_SIZE);
    ctx.partial_key[DISPLAY_SIZE] = '\0';
    
    // init display index
    ctx.display_index = 0;

    // Display compare with button mask
    UX_DISPLAY(
        ui_get_public_key_compare, 
        ui_prepro_get_public_key_compare
    );
}

static unsigned int ui_get_public_key_approve_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: // REJECT
            io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
            ui_idle();
            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // APPROVE
            send_pk();
            compare_pk();
            break;

        default:
            break;
    }

    return 0;
}

void handle_get_public_key_nanos() {
    snprintf(ctx.ui_approve_l2, 40, "Key #%u?", ctx.key_index);

    // Display Approval Screen
    UX_DISPLAY(ui_get_public_key_approve, NULL);
}

#elif defined(TARGET_NANOX)

void handle_get_public_key_nanox() {

}

#endif // TARGET

void handle_get_public_key(
        uint8_t p1,
        uint8_t p2,
        const uint8_t* const buffer,
        uint16_t len,
        /* out */ volatile unsigned int* flags,
        /* out */ volatile const unsigned int* const tx
) {
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(len);
    UNUSED(tx);

    // Read Key Index
    ctx.key_index = U4LE(buffer, 0);

#if defined(TARGET_NANOS)

    handle_get_public_key_nanos();
    *flags |= IO_ASYNCH_REPLY;

#elif defined(TARGET_NANOX)

    handle_get_public_key_nanox();

#endif // TARGET
}