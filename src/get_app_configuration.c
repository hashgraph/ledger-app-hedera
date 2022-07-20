#include <os.h>
#include <os_io_seproxyhal.h>
#include <stdint.h>

#include "errors.h"
#include "io.h"

void handle_get_app_configuration(
    uint8_t p1, uint8_t p2, const uint8_t* const buffer, uint16_t len,
    /* out */ volatile const unsigned int* const flags,
    /* out */ volatile const unsigned int* const tx) {
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(buffer);
    UNUSED(len);
    UNUSED(flags);
    UNUSED(tx);

    // storage allowed?
    G_io_apdu_buffer[ 0 ] = 0;

    // version
    G_io_apdu_buffer[ 1 ] = APPVERSION_M;
    G_io_apdu_buffer[ 2 ] = APPVERSION_N;
    G_io_apdu_buffer[ 3 ] = APPVERSION_P;

    io_exchange_with_code(EXCEPTION_OK, 4);
}
