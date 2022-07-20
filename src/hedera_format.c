#include "hedera_format.h"

#include <string.h>

char* hedera_format_tinybar(uint64_t tinybar) {
    return hedera_format_amount(tinybar, 8);
}

#define BUF_SIZE 32

char* hedera_format_amount(uint64_t amount, uint8_t decimals) {
    static char buf[ BUF_SIZE ];

    // NOTE: format of amounts are not sensitive
    memset(buf, 0, BUF_SIZE);

    // Quick shortcut if the amount is zero
    // Regardless of decimals, the output is always "0"
    if (amount == 0) {
        buf[ 0 ] = '0';
        buf[ 1 ] = '\0';

        return buf;
    }

    // NOTE: we silently fail with a decimal value > 20
    //  this function shuold only be called on decimal values smaller than 20
    if (decimals >= 20) return buf;

    int i = 0;

    while (i < (BUF_SIZE - 1) && (amount > 0 || i < decimals)) {
        int digit = amount % 10;
        amount /= 10;

        buf[ i++ ] = '0' + digit;

        if (i == decimals) {
            buf[ i++ ] = '.';
        }
    }

    if (buf[ i - 1 ] == '.') {
        buf[ i++ ] = '0';
    }

    int size = i;
    int j = 0;
    char tmp;

    while (j < i) {
        i -= 1;

        tmp = buf[ j ];
        buf[ j ] = buf[ i ];
        buf[ i ] = tmp;

        j += 1;
    }

    for (j = size - 1; j > 0; j--) {
        if (buf[ j ] == '0') {
            continue;
        } else if (buf[ j ] == '.') {
            break;
        } else {
            j += 1;
            break;
        }
    }

    if (j < size - 1) {
        buf[ j ] = '\0';
    }

    return buf;
}
