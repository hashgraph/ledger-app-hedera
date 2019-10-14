#pragma once

#include "os.h"
#include "os_io_seproxyhal.h"

#define P1_CONFIRM 0x01
#define P1_NON_CONFIRM 0x00
#define P1_FIRST 0x00
#define P1_MORE 0x80

#define FULL_ADDRESS_LENGTH 54
#define BIP32_PATH 5

// ux is a magic global variable implicitly referenced by the UX_ macros. Apps
// should never need to reference it directly.
extern ux_state_t ux;

// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;
