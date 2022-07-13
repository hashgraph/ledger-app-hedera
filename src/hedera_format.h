#ifndef LEDGER_HEDERA_HEDERA_FORMAT_H
#define LEDGER_HEDERA_HEDERA_FORMAT_H 1

#include <stdint.h>

extern char *hedera_format_tinybar(uint64_t tinybar);
extern char *hedera_format_amount(uint64_t amount, uint8_t decimals);

#endif // LEDGER_HEDERA_HEDERA_FORMAT_H
