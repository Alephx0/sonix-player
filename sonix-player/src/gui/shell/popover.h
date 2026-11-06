#ifndef POPOVER_H
#define POPOVER_H

#include <stdbool.h>

#include "lvgl/lvgl.h"

// A small modal menu that opens next to the control that asked for it --
// iOS-style: the page stays visible, a dim veil catches taps, and a compact
// card with a few actions appears by the anchor. One popover exists at a
// time; showing a new one replaces the old.

typedef struct {
	const char *label;
	void (*action)(void *user); // called after the popover has closed
	void *user;
	bool checked;
} popover_item_t;

// Builds the (hidden) veil and card once. Call from gui_init.
void popover_init(void);

// Opens the menu near `anchor` with `count` items (at most 7 are shown).
void popover_show(lv_obj_t *anchor, const popover_item_t *items, int count);

// Closes it (the veil tap and every item do this on their own).
void popover_close(void);

// The same, headed by a title in the reference layout's sheet ("Sleep timer").
void popover_show_titled(lv_obj_t *anchor, const char *title, const popover_item_t *items, int count);

#endif /* POPOVER_H */
