#pragma once

#include <os.h>
#include <stdint.h>

#define MEMCLEAR(element) explicit_bzero(&element, sizeof(element))

void public_key_to_bytes(uint8_t *dst, cx_ecfp_public_key_t *public);

void bin2hex(uint8_t *dst, uint8_t *data, uint64_t inlen);
