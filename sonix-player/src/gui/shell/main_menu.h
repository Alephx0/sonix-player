#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "src/gui/shell/gui.h"

#include "lvgl/lvgl.h"

extern lv_obj_t *main_menu_screen;

void main_menu_init(gui_config_t *cfg);

// Reuse the same mini-player and navigation on a native album screen.
int main_menu_mount_footer(lv_obj_t *screen);
void main_menu_restore_footer(lv_obj_t *screen);
void main_menu_open_search(void);
void main_menu_open_library(int filter);
// True while the profile drawer is open (the probe reports it as a modal).
bool main_menu_drawer_open(void);

#endif
