#include "menu.h"
#include "os.h"

#if defined(TARGET_NANOS)

static const ux_menu_entry_t menu_main[3];

static const ux_menu_entry_t menu_about[3] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END
};

static const ux_menu_entry_t menu_main[3] = {
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
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
