#ifndef SETTINGS_H
#define SETTINGS_H

#include "src/gui/shell/gui.h"

#include "lvgl/lvgl.h"

extern lv_obj_t *settings_screen;

void settings_init(gui_config_t *cfg);
// The reference layout's Spotify categories, once every page they open is built.
void settings_build_categories(gui_config_t *cfg);
// The Search settings page once it has been opened (NULL before).
lv_obj_t *settings_search_screen(void);

// Hands power.c the screen timeout as it was last left ([power]
// screen_off_seconds). Call once at startup, after power_init(); the Screen
// page calls it again whenever the slider moves.
void settings_apply_screen_off(void);

// Shows or hides the developer options entry according to whether it has been
// unlocked. The page is built once at startup, so it has to be told when the
// five taps on the build number unlock it.
void settings_refresh_devoptions(void);

#endif // SETTINGS_H
