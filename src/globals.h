#ifndef LEDGER_HEDERA_GLOBALS_H
#define LEDGER_HEDERA_GLOBALS_H 1

#include <stdint.h>

#define BIP32_PATH 5
#define DISPLAY_SIZE 18 // characters @ 11pt sys font

#define MAX_TX_SIZE 512
#define FULL_ADDRESS_LENGTH 54
#define ACCOUNT_ID_SIZE 19 * 3 + 2 + 1
#define KEY_SIZE 64
#define MAX_MEMO_SIZE 200
#define SIGNATURE_SIZE 32

#define HBAR 100000000 // tinybar
#define HBAR_BUF_SIZE 26

#define CLA 0xE0

// These are the offsets of various parts of a request APDU packet.
// INS identifies the commands.
// P1 and P2 are parameters to the command.
#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 5

// User IDs for BAGL Elements
static const uint8_t LEFT_ICON_ID = 0x01;
static const uint8_t RIGHT_ICON_ID = 0x02;
static const uint8_t LINE_1_ID = 0x05;
static const uint8_t LINE_2_ID = 0x06;

#endif // LEDGER_HEDERA_GLOBALS_H
