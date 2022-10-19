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
#include "ui.h"
#include "utils.h"

static struct get_public_key_context_t {
    uint32_t key_index;

    // Lines on the UI Screen
    char ui_approve_l2[ DISPLAY_SIZE + 1 ];

    cx_ecfp_public_key_t public;

    // Public Key Compare
    uint8_t display_index;
    uint8_t full_key[ KEY_SIZE + 1 ];
    uint8_t partial_key[ DISPLAY_SIZE + 1 ];
} ctx;

#if defined(TARGET_NANOS)

static const bagl_element_t ui_get_public_key_compare[] = {
    UI_BACKGROUND(), UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),
    // <=                  =>
    //      Public Key
    //      <partial>
    //
    UI_TEXT(LINE_1_ID, 0, 12, 128, "Public Key"),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.partial_key)};

static const bagl_element_t ui_get_public_key_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_CHECK),
    //
    //    Export Public
    //       Key #123?
    //
    UI_TEXT(LINE_1_ID, 0, 12, 128, "Export Public"),
    UI_TEXT(LINE_2_ID, 0, 26, 128, ctx.ui_approve_l2),
};

void shift_partial_key() {
    memmove(ctx.partial_key, ctx.full_key + ctx.display_index, DISPLAY_SIZE);
}

static unsigned int ui_get_public_key_compare_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
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
            if (ctx.display_index < KEY_SIZE - DISPLAY_SIZE)
                ctx.display_index++;
            shift_partial_key();
            UX_REDISPLAY();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // Continue
            ui_idle();
            break;
    }
    return 0;
}

static const bagl_element_t *ui_prepro_get_public_key_compare(
    const bagl_element_t *element) {
    if (element->component.userid == LEFT_ICON_ID && ctx.display_index == 0)
        return NULL; // Hide Left Arrow at Left Edge
    if (element->component.userid == RIGHT_ICON_ID &&
        ctx.display_index == KEY_SIZE - DISPLAY_SIZE)
        return NULL; // Hide Right Arrow at Right Edge
    return element;
}

void compare_pk() {
    // init partial key str from full str
    memmove(ctx.partial_key, ctx.full_key, DISPLAY_SIZE);
    ctx.partial_key[ DISPLAY_SIZE ] = '\0';

    // init display index
    ctx.display_index = 0;

    // Display compare with button mask
    UX_DISPLAY(ui_get_public_key_compare, ui_prepro_get_public_key_compare);
}

static unsigned int ui_get_public_key_approve_button(
    unsigned int button_mask, unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT: // REJECT
            io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
            ui_idle();
            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT: // APPROVE
            io_exchange_with_code(EXCEPTION_OK, 32);
            compare_pk();
            break;

        default:
            break;
    }

    return 0;
}

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
unsigned int io_seproxyhal_touch_pk_ok(const bagl_element_t *e) {
    io_exchange_with_code(EXCEPTION_OK, 32);
    compare_pk();
    return 0;
}

unsigned int io_seproxyhal_touch_pk_cancel(const bagl_element_t *e) {
    io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
    ui_idle();
    return 0;
}

UX_STEP_NOCB(ux_approve_pk_flow_1_step, bn,
             {"Export Public", ctx.ui_approve_l2});

UX_STEP_VALID(ux_approve_pk_flow_2_step, pb, io_seproxyhal_touch_pk_ok(NULL),
              {&C_icon_validate_14, "Approve"});

UX_STEP_VALID(ux_approve_pk_flow_3_step, pb,
              io_seproxyhal_touch_pk_cancel(NULL),
              {&C_icon_crossmark, "Reject"});

UX_STEP_CB(ux_compare_pk_flow_1_step, bnnn_paging, ui_idle(),
           {.title = "Public Key", .text = (char *)ctx.full_key});

UX_DEF(ux_approve_pk_flow, &ux_approve_pk_flow_1_step,
       &ux_approve_pk_flow_2_step, &ux_approve_pk_flow_3_step);

UX_DEF(ux_compare_pk_flow, &ux_compare_pk_flow_1_step);

void compare_pk() { ux_flow_init(0, ux_compare_pk_flow, NULL); }

#endif // TARGET

void get_pk() {
    // Derive Key
    hedera_derive_keypair(ctx.key_index, NULL, &ctx.public);

    // Put Key bytes in APDU buffer
    public_key_to_bytes(G_io_apdu_buffer, &ctx.public);

    // Populate Key Hex String
    bin2hex(ctx.full_key, G_io_apdu_buffer, KEY_SIZE);
    ctx.full_key[ KEY_SIZE ] = '\0';
}

void handle_get_public_key(uint8_t p1, uint8_t p2, uint8_t *buffer,
                           uint16_t len,
                           /* out */ volatile unsigned int *flags,
                           /* out */ volatile unsigned int *tx) {
    UNUSED(p2);
    UNUSED(len);
    UNUSED(tx);

    // Read Key Index
    ctx.key_index = U4LE(buffer, 0);

    // If p1 != 0, silent mode, for use by apps that request the user's public
    // key frequently Only do UI actions for p1 == 0
    if (p1 == 0) {
        // Complete "Export Public | Key #x?"
        hedera_snprintf(ctx.ui_approve_l2, DISPLAY_SIZE, "Key #%u?",
                        ctx.key_index);
    }

    // Populate context with PK
    get_pk();

#if defined(TARGET_NANOS)

    if (p1 == 0) {
        UX_DISPLAY(ui_get_public_key_approve, NULL);
    }

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

    if (p1 == 0) {
        ux_flow_init(0, ux_approve_pk_flow, NULL);
    }

#endif // TARGET

    // Normally happens in approve export public key handler
    if (p1 != 0) {
        io_exchange_with_code(EXCEPTION_OK, 32);
        ui_idle();
    }

    *flags |= IO_ASYNCH_REPLY;
}
