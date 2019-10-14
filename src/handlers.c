#include "handlers.h"

handler_fn_t* lookup_handler(uint8_t ins) {
    switch (ins) {
        case INS_GET_APP_CONFIGURATION: return handle_get_app_configuration;
        case INS_GET_PUBLIC_KEY:        return handle_get_public_key;
        case INS_SIGN_TRANSACTION:      return handle_sign_transaction;

        default:                        return NULL;
    }
}
