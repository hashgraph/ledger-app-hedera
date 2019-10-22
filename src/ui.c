#include "ui.h"
#include "os.h"

#if defined(TARGET_NANOS)

// This is a forward declaration since [menu_about] needs to know about
// [menu_main] to go back to it.
static const ux_menu_entry_t menu_main[4];

static const ux_menu_entry_t menu_about[3] = {
    {
        .menu = NULL,
        .callback = NULL,
        .userid = 0,
        .icon = NULL,
        .line1 = "Version",
        .line2 = APPVERSION,
        .text_x = 0,
        .icon_x = 0,
    },

    {
        .menu = menu_main,
        .callback = NULL,
        .userid = 0,
        .icon = &C_icon_back,
        .line1 = "Back",
        .line2 = NULL,
        .text_x = 61,
        .icon_x = 40,
    },

    UX_MENU_END
};

static const ux_menu_entry_t menu_main[4] = {
    {
        .menu = NULL,
        .callback = NULL,
        .userid = 0,
        .icon = NULL,
        .line1 = "Awaiting",
        .line2 = "commands...",
        .text_x = 0,
        .icon_x = 0
    },
    {
        .menu = menu_about,
        .callback = NULL,
        .userid = 0,
        .icon = NULL,
        .line1 = "About",
        .line2 = NULL,
        .text_x = 0,
        .icon_x = 0,
    },

    {
        .menu = NULL,
        .callback = &os_sched_exit,
        .userid = 0,
        .icon = &C_icon_dashboard,
        .line1 = "Quit app",
        .line2 = NULL,
        .text_x = 50,
        .icon_x = 29,
    },

    UX_MENU_END
};

#endif // #if TARGET_

// ui_idle displays the main menu
// note that we are not _required_ to use the main menu as the idle screen
void ui_idle(void) {
#if defined(TARGET_NANOS)
    UX_MENU_DISPLAY(0, menu_main, NULL);
#elif defined(TARGET_NANOX)
    // reserve a display stack slot if none yet
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_idle_flow, NULL);
#endif // #if TARGET_
}
