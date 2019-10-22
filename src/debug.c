#include <stdint.h>
#include "os.h"
#include "debug.h"

// This symbol is defined by the link script to be at the start of the stack
// area.
extern unsigned long _stack;

#define STACK_CANARY (*((volatile uint32_t*) &_stack))

void debug_init_stack_canary() {
    STACK_CANARY = 0xDEADBEEF;
}

void debug_check_stack_canary() {
    if (STACK_CANARY != 0xDEADBEEF) {
        THROW(EXCEPTION_OVERFLOW);
    }
}
