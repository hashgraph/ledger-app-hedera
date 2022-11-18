#pragma once

#include "ux.h"

typedef struct get_public_key_context_s {
    uint32_t key_index;

    // Lines on the UI Screen
    char ui_approve_l2[ DISPLAY_SIZE + 1 ];

    cx_ecfp_public_key_t public;

    // Public Key Compare
    uint8_t display_index;
    uint8_t full_key[ KEY_SIZE + 1 ];
    uint8_t partial_key[ DISPLAY_SIZE + 1 ];
} get_public_key_context_t;

extern get_public_key_context_t gpk_ctx;
