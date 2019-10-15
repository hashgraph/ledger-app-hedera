#include "utils.h"
#include "ui.h"
#include "handlers.h"
#include "errors.h"
#include "pb_custom.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "TransactionBody.pb.h"

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

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

// This is the main loop that reads and writes APDUs. It receives request
// APDUs from the computer, looks up the corresponding command handler, and
// calls it on the APDU payload. Then it loops around and calls io_exchange
// again. The handler may set the 'flags' and 'tx' variables, which affect the
// subsequent io_exchange call. The handler may also throw an exception, which
// will be caught, converted to an error code, appended to the response APDU,
// and sent in the next io_exchange call.

void app_main() {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no APDU received; trigger a reset
                if (rx == 0) {
                    THROW(EXCEPTION_IO_RESET);
                }

                // malformed APDU
                if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                    THROW(EXCEPTION_MALFORMED_APDU);
                }

                // lookup and call the requested instruction
                handler_fn_t* fn = lookup_handler(G_io_apdu_buffer[OFFSET_INS]);
                if (!fn) {
                    THROW(EXCEPTION_UNKNOWN_INS);
                }

                fn(G_io_apdu_buffer[OFFSET_P1], G_io_apdu_buffer[OFFSET_P2],
                   G_io_apdu_buffer + OFFSET_CDATA, G_io_apdu_buffer[OFFSET_LC], 
                   &flags, &tx);
            }
            CATCH(EXCEPTION_IO_RESET) {
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) {
                // Convert the exception to a response code. All error codes
                // start with 6, except for 0x9000, which is a special
                // "success" code. Every APDU payload should end with such a
                // code, even if no other data is sent.

                // If the first byte is not a 6, mask it with 0x6800 to
                // convert it to a proper error code.

                switch (e & 0xF000) {
                    case 0x6000:
                    case 0x9000:
                        sw = e;
                        break;

                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }

                G_io_apdu_buffer[tx++] = sw >> 8;
                G_io_apdu_buffer[tx++] = sw & 0xff;
            }
            FINALLY {
                // do nothing
            }
        }
        END_TRY;
    }
}

static void app_exit(void) {
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {
            // do nothing
        }
    }
    END_TRY_L(exit);
}

__attribute__((section(".boot"))) int main() {
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

    for (;;) {
        UX_INIT();

        BEGIN_TRY {
            TRY {
                // Initialize the hardware abstraction layer (HAL) in 
                // the Ledger SDK
                io_seproxyhal_init();

                USB_power(0);
                USB_power(1);

                // Shows the main menu
                ui_idle();

                // The Ledger NanoX is bluetooth enabled and can communicate 
                // using BLE instead of over USB
#ifdef HAVE_BLE
                BLE_power(0, NULL);
                BLE_power(1, "Nano X");
#endif // HAVE_BLE

                app_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                // reset IO and UX before continuing
                continue;
            }
            CATCH_ALL {
                break;
            }
            FINALLY {
                // do nothing
            }
        }
        END_TRY;
    }

    app_exit();

    return 0;
}
