#include "utils.h"

void public_key_to_bytes(unsigned char *dst, cx_ecfp_public_key_t *public) {
    for (int i = 0; i < 32; i++) {
        dst[i] = public->W[64 - i];
    }

    if (public->W[32] & 1) {
        dst[31] |= 0x80;
    }
}
