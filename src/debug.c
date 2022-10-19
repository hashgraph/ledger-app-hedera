#include "debug.h"

#include "os.h"

// This symbol is defined by the link script to be at the start of the stack
// area.
extern unsigned long app_stack_canary;

#define STACK_CANARY (*((volatile uint32_t *)&app_stack_canary))

void debug_init_stack_canary() { STACK_CANARY = 0xDEADBEEF; }

void debug_check_stack_canary() {
    if (STACK_CANARY != 0xDEADBEEF) {
        THROW(EXCEPTION_OVERFLOW);
    }
}

uint32_t debug_get_stack_canary() { return STACK_CANARY; }
