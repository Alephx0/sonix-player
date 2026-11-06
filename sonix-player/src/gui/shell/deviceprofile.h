#ifndef DEVICEPROFILE_H
#define DEVICEPROFILE_H

#include <stdbool.h>

#include "lvgl/lvgl.h"

// The device's own name and picture, as Spotify shows an account's: the round
// avatar beside every tab title, the profile card at the top of the drawer and
// of Settings, and the head of About, where both are changed.
//
// The name is [device] name in the config ("Sonix R1" when unset). The picture
// is [device] image, a path on the card; with no entry at all, .local/profile.jpg
// on the card is used when it is there, and "" means no picture (the name's
// initial on a coloured disc, Spotify's default).

void deviceprofile_init(const char *sd_root);

const char *deviceprofile_name(void);
void deviceprofile_set_name(const char *name);

// The picture's path, or "" for none.
const char *deviceprofile_image(void);
void deviceprofile_set_image(const char *path);

// Paints `obj` as the avatar, `size` px across: the picture cut round, or the
// initial in `font`. Painted again by itself after a change, until deleted.
void deviceprofile_avatar(lv_obj_t *obj, int size, const lv_font_t *font);

// A label that shows the name, kept up to date after a change.
void deviceprofile_name_label(lv_obj_t *label);

// Called after the name or the picture changes (up to four listeners).
void deviceprofile_on_change(void (*cb)(void));

// The two editors, over the page: the name with the keyboard, the picture from
// the images on the card.
void deviceprofile_edit_name(void);
void deviceprofile_pick_image(void);

#endif /* DEVICEPROFILE_H */
