#include <string.h>

#include "hedera_format.h"

// if explicit_bzero is not defined, provide with memset
// enables unit testing
#ifndef explicit_bzero
#define explicit_bzero(buf, size) memset(buf, 0, size)
#endif

char* hedera_format_tinybar(uint64_t tinybar) {
    return hedera_format_amount(tinybar, 8);
}

char* hedera_format_amount(uint64_t amount, uint8_t decimals) {
    static const int BUF_SIZE = 32;
    static char buf[BUF_SIZE];

    explicit_bzero(buf, BUF_SIZE);

    int i = 0;

    while (i < (BUF_SIZE - 1) && (amount > 0 || i < decimals)) {
        int digit = amount % 10;
        amount /= 10;

        buf[i++] = '0' + digit;

        if (i == decimals) {
            buf[i++] = '.';
        }
    }

    if (buf[i - 1] == '.') {
        buf[i++] = '0';
    }

    int j = 0;
    char tmp;

    while (j < i) {
        i -= 1;

        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
        
        j += 1;
    }

    return buf;
}


