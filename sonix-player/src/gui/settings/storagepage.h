#ifndef STORAGEPAGE_H
#define STORAGEPAGE_H

#include "lvgl/lvgl.h"
#include "src/gui/shell/gui.h"

// Settings > Device > Storage: the card split by what fills it (music,
// audiobooks, photos, books, the player's own data, other, free) in one bar,
// and the R1's internal memory (the OS) apart.
extern lv_obj_t *storagepage_screen;
void storagepage_init(gui_config_t *cfg);

#endif /* STORAGEPAGE_H */
