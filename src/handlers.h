#ifndef LEDGER_HEDERA_HANDLERS_H
#define LEDGER_HEDERA_HANDLERS_H 1

#include <stddef.h>
#include <stdint.h>

// CLA <INS> <-- Command Line Argument <Instruction>
#define INS_GET_APP_CONFIGURATION 0x01
#define INS_GET_PUBLIC_KEY 0x02
#define INS_SIGN_TRANSACTION 0x04

typedef void handler_fn_t(uint8_t p1, uint8_t p2, uint8_t* buffer, uint16_t len,
                          /* out */ volatile unsigned int* flags,
                          /* out */ volatile unsigned int* tx);

extern handler_fn_t handle_get_app_configuration;
extern handler_fn_t handle_get_public_key;
extern handler_fn_t handle_sign_transaction;

#endif // LEDGER_HEDERA_HANDLERS_H
