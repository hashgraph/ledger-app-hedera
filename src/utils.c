#include "utils.h"

int bin2dec(char *dst, uint64_t n) {
    if (n == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return 1;
    }
    // determine final length
    int len = 0;
    for (uint64_t nn = n; nn != 0; nn /= 10) {
        len++;
    }
    // write digits in big-endian order
    for (int i = len-1; i >= 0; i--) {
        dst[i] = (n % 10) + '0';
        n /= 10;
    }
    dst[len] = '\0';
    return len;
}

void public_key_to_bytes(unsigned char *dst, cx_ecfp_public_key_t *public) {
    for (int i = 0; i < 32; i++) {
        dst[i] = public->W[64 - i];
    }

    if (public->W[32] & 1) {
        dst[31] |= 0x80;
    }
}
