#ifndef LEDGER_HEDERA_UTILS_H
#define LEDGER_HEDERA_UTILS_H 1

#include <stdint.h>

#include <os.h>

extern void public_key_to_bytes(uint8_t *dst, cx_ecfp_public_key_t *public);

#endif // LEDGER_HEDERA_UTILS_H
