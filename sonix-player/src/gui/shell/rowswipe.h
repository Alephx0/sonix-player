#ifndef ROWSWIPE_H
#define ROWSWIPE_H

#include <stdbool.h>
#include <stdint.h>

#include "lvgl/lvgl.h"

/* One shared horizontal row swipe, Spotify's swipe-to-queue: the row's content
 * follows the finger over an action strip with its icon; the strip turns its
 * colour once the swipe has gone far enough, and letting go there commits.
 * Short of it, or on a mostly vertical drag, the row springs back and nothing
 * happens. The leftmost strip of the screen is left to the swipe back.
 *
 * The owner forwards its row's PRESSED, PRESSING, RELEASED and PRESS_LOST
 * events; the row's own children are moved, never the row, so list layouts and
 * pooled rows are untouched. */

typedef struct {
	const lv_image_dsc_t *right_icon; // swipe right (toward the right edge); NULL: not offered
	uint32_t right_color;			  // strip colour once armed
	const lv_image_dsc_t *left_icon;  // swipe left; NULL: not offered
	uint32_t left_color;
} rowswipe_spec_t;

#define ROWSWIPE_GREEN 0x1ed760
#define ROWSWIPE_RED 0xe91429

/* Handles one event of `row`. Returns true when the press belongs to a swipe
 * (the caller then stops bubbling and must not treat it as a tap). On a
 * committing release, *committed is +1 (right) or -1 (left), otherwise 0. */
bool rowswipe_event(lv_event_t *e, lv_obj_t *row, const rowswipe_spec_t *spec, int *committed);

/* True when the last press on `row` became a swipe: for click and long-press
 * handlers, which run after the release. Reset by the next press. */
bool rowswipe_was_swiped(lv_obj_t *row);

/* True while a row swipe owns the press: the page's swipe back stands down. */
bool rowswipe_active(void);

/* Puts the swiped row straight back, without the spring: for a commit that
 * rebuilds or reuses the row at once (removing it from a pooled list). */
void rowswipe_settle(void);

/* The distance at which a swipe commits, for tests and callers. */
#define ROWSWIPE_COMMIT 96

#endif
