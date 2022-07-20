#include "ui.h"

/*
 * Defines the main menu and idle actions for the app
 */

#if defined(TARGET_NANOS)
ux_state_t ux;
unsigned int ux_step;
unsigned int ux_step_count;

static const ux_menu_entry_t menu_main[ 4 ];

static const ux_menu_entry_t menu_about[ 3 ] = {{
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

                                                UX_MENU_END};

static const ux_menu_entry_t menu_main[ 4 ] = {{.menu = NULL,
                                                .callback = NULL,
                                                .userid = 0,
                                                .icon = NULL,
                                                .line1 = "Awaiting",
                                                .line2 = "Commands",
                                                .text_x = 0,
                                                .icon_x = 0},
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

                                               UX_MENU_END};
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

UX_STEP_NOCB(ux_idle_flow_1_step, nn, {"Awaiting", "Commands"});

UX_STEP_NOCB(ux_idle_flow_2_step, bn,
             {
                 "Version",
                 APPVERSION,
             });

UX_STEP_VALID(ux_idle_flow_3_step, pb, os_sched_exit(-1),
              {&C_icon_dashboard_x, "Exit"});

UX_DEF(ux_idle_flow, &ux_idle_flow_1_step, &ux_idle_flow_2_step,
       &ux_idle_flow_3_step);

#endif // TARGET

void ui_idle(void) {
#if defined(TARGET_NANOS)
    UX_MENU_DISPLAY(0, menu_main, NULL);
#elif defined(TARGET_NANOX) || defined(TARGET_NANOS2)
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_idle_flow, NULL);
#endif // #if TARGET_
}
