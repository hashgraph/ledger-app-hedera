#ifndef LEDGER_HEDERA_GET_PUBLIC_KEY_H
#define LEDGER_HEDERA_GET_PUBLIC_KEY_H 1

void get_pk();
void compare_pk();

#if defined(TARGET_NANOS)

void shift_partial_key();

static unsigned int ui_get_public_key_compare_button(
    unsigned int button_mask, unsigned int button_mask_counter);

static const bagl_element_t *ui_prepro_get_public_key_compare(
    const bagl_element_t *element);

void send_pk();

static unsigned int ui_get_public_key_approve_button(
    unsigned int button_mask, unsigned int button_mask_counter);

#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

unsigned int io_seproxyhal_touch_pk_ok(const bagl_element_t *e);
unsigned int io_seproxyhal_touch_pk_cancel(const bagl_element_t *e);

#endif // TARGET

#endif // LEDGER_HEDERA_GET_PUBLIC_KEY_H
