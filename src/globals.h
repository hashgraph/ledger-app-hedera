#ifndef LEDGER_HEDERA_GLOBALS_H
#define LEDGER_HEDERA_GLOBALS_H 1

#if defined(TARGET_NANOS)

#include "os.h"
#include "os_io_seproxyhal.h"

#define FULL_ADDRESS_LENGTH 54
#define BIP32_PATH 5

#elif defined(TARGET_NANOX)

#endif // TARGET
#endif // LEDGER_HEDERA_GLOBALS_H
