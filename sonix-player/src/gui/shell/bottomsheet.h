#ifndef BOTTOMSHEET_H
#define BOTTOMSHEET_H
#include "lvgl/lvgl.h"
/* One modal layer. Replacing content keeps the scrim and panel on screen.
 * Cleanup runs before old content is deleted; it may rescue borrowed widgets. */
lv_obj_t *bottomsheet_open(int height, void (*cleanup)(void));
void bottomsheet_close(bool animate);
bool bottomsheet_active(void);
#endif
