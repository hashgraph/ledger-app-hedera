#ifndef LEDGER_HEDERA_ERRORS_H
#define LEDGER_HEDERA_ERRORS_H 1

// APDU buffer is malformed
#define EXCEPTION_MALFORMED_APDU 0x6E00

// Instruction request is unknown
#define EXCEPTION_UNKNOWN_INS 0x6D00

// Internal exception
#define EXCEPTION_INTERNAL 0x6980

// User rejected action
#define EXCEPTION_USER_REJECTED 0x6985

// Ok
#define EXCEPTION_OK 0x9000

#endif // LEDGER_HEDERA_ERRORS_H
