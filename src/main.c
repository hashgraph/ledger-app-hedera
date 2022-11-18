#include "debug.h"
#include "errors.h"
#include "globals.h"
#include "glyphs.h"
#include "handlers.h"
#include "io.h"
#include "ui_flows.h"
#include "utils.h"
#include "ux.h"

// This is the main loop that reads and writes APDUs. It receives request
// APDUs from the computer, looks up the corresponding command handler, and
// calls it on the APDU payload. Then it loops around and calls io_exchange
// again. The handler may set the 'flags' and 'tx' variables, which affect the
// subsequent io_exchange call. The handler may also throw an exception, which
// will be caught, converted to an error code, appended to the response APDU,
// and sent in the next io_exchange call.

// Things are marked volatile throughout the app to prevent unintended compiler
// reording of instructions (since the try-catch system is a macro)

void app_main() {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws
                        // an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                // no APDU received; trigger a reset
                if (rx == 0) {
                    THROW(EXCEPTION_IO_RESET);
                }

                // malformed APDU
                if (G_io_apdu_buffer[ OFFSET_CLA ] != CLA) {
                    THROW(EXCEPTION_MALFORMED_APDU);
                }

                // APDU handler functions defined in handlers
                switch (G_io_apdu_buffer[ OFFSET_INS ]) {
                    case INS_GET_APP_CONFIGURATION:
                        // handlers -> get_app_configuration
                        handle_get_app_configuration(
                            G_io_apdu_buffer[ OFFSET_P1 ],
                            G_io_apdu_buffer[ OFFSET_P2 ],
                            G_io_apdu_buffer + OFFSET_CDATA,
                            G_io_apdu_buffer[ OFFSET_LC ], &flags, &tx);
                        break;

                    case INS_GET_PUBLIC_KEY:
                        // handlers -> get_public_key
                        handle_get_public_key(G_io_apdu_buffer[ OFFSET_P1 ],
                                              G_io_apdu_buffer[ OFFSET_P2 ],
                                              G_io_apdu_buffer + OFFSET_CDATA,
                                              G_io_apdu_buffer[ OFFSET_LC ],
                                              &flags, &tx);
                        break;

                    case INS_SIGN_TRANSACTION:
                        // handlers -> sign_transaction
                        handle_sign_transaction(G_io_apdu_buffer[ OFFSET_P1 ],
                                                G_io_apdu_buffer[ OFFSET_P2 ],
                                                G_io_apdu_buffer + OFFSET_CDATA,
                                                G_io_apdu_buffer[ OFFSET_LC ],
                                                &flags, &tx);
                        break;

                    default:
                        THROW(EXCEPTION_UNKNOWN_INS);
                }
            }
            CATCH(EXCEPTION_IO_RESET) { THROW(EXCEPTION_IO_RESET); }
            CATCH_OTHER(e) {
                // Convert exception to response code and add to APDU return
                switch (e & 0xF000) {
                    case 0x6000:
                    case 0x9000:
                        sw = e;
                        break;

                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }

                G_io_apdu_buffer[ tx++ ] = sw >> 8;
                G_io_apdu_buffer[ tx++ ] = sw & 0xff;
            }
            FINALLY {
                // explicitly do nothing
            }
        }
        END_TRY;
    }
}

void app_exit(void) {
    // All os calls must be wrapped in a try catch context
    BEGIN_TRY_L(exit) {
        TRY_L(exit) { os_sched_exit(-1); }
        FINALLY_L(exit) {
            // explicitly do nothing
        }
    }
    END_TRY_L(exit);
}

__attribute__((section(".boot"))) int main() {
    // exit critical section (ledger magic)
    __asm volatile("cpsie i");

    // go with the overflow
    debug_init_stack_canary();

    os_boot();

    for (;;) {
        // Initialize the UX system
        UX_INIT();

        BEGIN_TRY {
            TRY {
                // Initialize the hardware abstraction layer (HAL) in
                // the Ledger SDK
                io_seproxyhal_init();

#ifdef TARGET_NANOX
                // grab the current plane mode setting
                G_io_app.plane_mode =
                    os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif // TARGET_NANOX

#ifdef HAVE_BLE
                BLE_power(0, NULL);
                BLE_power(1, "Nano X");
#endif // HAVE_BLE

                USB_power(0);
                USB_power(1);

                // Shows the main menu
                ui_idle();

                // Actual Main Loop
                app_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                // reset IO and UX before continuing
                continue;
            }
            CATCH_ALL { break; }
            FINALLY {
                // explicitly do nothing
            }
        }
        END_TRY;
    }

    app_exit();

    return 0;
}
