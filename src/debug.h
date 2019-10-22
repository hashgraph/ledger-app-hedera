#ifndef LEDGER_HEDERA_DEBUG_H
#define LEDGER_HEDERA_DEBUG_H 1

#include <stdint.h>

extern void debug_init_stack_canary();

extern uint32_t debug_get_stack_canary();

extern void debug_check_stack_canary();

#endif // LEDGER_HEDERA_DEBUG_H
