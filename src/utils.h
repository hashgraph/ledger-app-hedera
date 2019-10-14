#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <os.h>

// bin2dec converts an unsigned integer to a decimal string and appends a
// final NUL byte. It returns the length of the string.
extern int bin2dec(char *dst, uint64_t n);

extern void public_key_to_bytes(unsigned char *dst, cx_ecfp_public_key_t *public);
