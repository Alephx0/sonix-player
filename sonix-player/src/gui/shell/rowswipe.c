#include "rowswipe.h"

#include <stdlib.h>

// How far a press must travel sideways, and how much straighter than it goes
// down, before it is a swipe rather than a tap or the start of a scroll.
#define TAKE_DISTANCE 8 // under LVGL's 10 px scroll limit and the swipe back's 10 px
#define EDGE 32 // the swipe back's own strip on the left
#define SPRING_MS 170

static lv_obj_t *row_now;	 // the row under the current press
static lv_obj_t *row_swiped; // the row whose last press became a swipe
static lv_point_t origin;
static bool taken, armed, vertical_scroll;
static int direction;
static int32_t offset;

// The strip under the content: the row's own background, recoloured, with an
// icon at the edge the content is moving away from.
static lv_obj_t *icon;
// The row's own local background, per state, so it can be put back exactly
// (a row styled only by its theme gets nothing left behind).
static const lv_style_selector_t STATES[2] = {LV_PART_MAIN | LV_STATE_DEFAULT, LV_PART_MAIN | LV_STATE_PRESSED};
static lv_style_value_t saved_value[2][3];
static bool saved_local[2][3];
static bool saved_chain;

static void place_content(lv_obj_t *row, int32_t x) {
	uint32_t count = lv_obj_get_child_count(row);
	for (uint32_t i = 0; i < count; i++) {
		lv_obj_t *child = lv_obj_get_child(row, (int32_t)i);
		if (child != icon) {
			lv_obj_set_style_translate_x(child, x, 0);
		}
	}
}

static void paint(lv_obj_t *row, const rowswipe_spec_t *spec) {
	uint32_t color = direction > 0 ? spec->right_color : spec->left_color;
	// Both states: a held row is also a pressed one, whose theme colour would
	// otherwise win over the strip.
	for (int i = 0; i < 2; i++) {
		lv_obj_set_style_bg_color(row, armed ? lv_color_hex(color) : lv_color_hex(0x2a2a2a), STATES[i]);
		lv_obj_set_style_bg_opa(row, LV_OPA_COVER, STATES[i]);
		lv_obj_set_style_color_filter_opa(row, LV_OPA_TRANSP, STATES[i]); // no pressed darkening
	}
	if (icon) {
		// Black on Spotify green, white on the red of a removal.
		lv_color_t ink = color == ROWSWIPE_GREEN ? lv_color_black() : lv_color_white();
		lv_obj_set_style_image_recolor(icon, armed ? ink : lv_color_hex(0xb3b3b3), 0);
		lv_obj_set_style_transform_scale(icon, armed ? 300 : 256, 0);
	}
}

static void begin(lv_obj_t *row, const rowswipe_spec_t *spec) {
	taken = true;
	armed = false;
	row_swiped = row;
	static const lv_style_prop_t PROPS[3] = {LV_STYLE_BG_COLOR, LV_STYLE_BG_OPA, LV_STYLE_COLOR_FILTER_OPA};
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			saved_local[i][j] = lv_obj_get_local_style_prop(row, PROPS[j], &saved_value[i][j], STATES[i]) == LV_RESULT_OK;
		}
	}
	// Held still vertically while it is a swipe: a small drift must not hand
	// the press to the list's scroll halfway through.
	saved_chain = lv_obj_has_flag(row, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
	lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
	// Not a pressed row any more but a moving one: the theme's pressed look
	// (a darkening filter) would dull the strip.
	lv_obj_remove_state(row, LV_STATE_PRESSED);
	icon = lv_image_create(row);
	lv_obj_add_flag(icon, LV_OBJ_FLAG_IGNORE_LAYOUT);
	lv_obj_remove_flag(icon, LV_OBJ_FLAG_CLICKABLE);
	lv_image_set_src(icon, direction > 0 ? spec->right_icon : spec->left_icon);
	lv_obj_set_size(icon, 28, 28);
	lv_image_set_inner_align(icon, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_set_style_image_recolor_opa(icon, LV_OPA_COVER, 0);
	lv_obj_set_style_transform_pivot_x(icon, 14, 0);
	lv_obj_set_style_transform_pivot_y(icon, 14, 0);
	lv_obj_align(icon, direction > 0 ? LV_ALIGN_LEFT_MID : LV_ALIGN_RIGHT_MID, direction > 0 ? 24 : -24, 0);
	lv_obj_move_to_index(icon, 0); // drawn first: under the moving content
	paint(row, spec);
}

static void finish(lv_obj_t *row) {
	if (row && lv_obj_is_valid(row)) {
		place_content(row, 0);
		static const lv_style_prop_t PROPS[3] = {LV_STYLE_BG_COLOR, LV_STYLE_BG_OPA, LV_STYLE_COLOR_FILTER_OPA};
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 3; j++) {
				if (saved_local[i][j]) {
					lv_obj_set_local_style_prop(row, PROPS[j], saved_value[i][j], STATES[i]);
				} else {
					lv_obj_remove_local_style_prop(row, PROPS[j], STATES[i]);
				}
			}
		}
		if (saved_chain) {
			lv_obj_add_flag(row, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
		}
		if (icon && lv_obj_is_valid(icon)) {
			lv_obj_delete(icon);
		}
	}
	icon = NULL;
}

static void spring_exec(void *row, int32_t x) { place_content(row, x); }
static void spring_done(lv_anim_t *a) { finish(a->var); }

static void spring_back(lv_obj_t *row) {
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, row);
	lv_anim_set_exec_cb(&a, spring_exec);
	lv_anim_set_values(&a, offset, 0);
	lv_anim_set_duration(&a, SPRING_MS);
	lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
	lv_anim_set_completed_cb(&a, spring_done);
	lv_anim_start(&a);
}

// Past the commit distance the content keeps following, but more slowly, so
// the row never runs off the screen.
static int32_t resisted(int dx) {
	int32_t limit = ROWSWIPE_COMMIT + 40;
	int magnitude = abs(dx);
	if (magnitude > limit) {
		magnitude = limit + (magnitude - limit) / 3;
	}
	return dx < 0 ? -magnitude : magnitude;
}

bool rowswipe_event(lv_event_t *e, lv_obj_t *row, const rowswipe_spec_t *spec, int *committed) {
	if (committed) {
		*committed = 0;
	}
	lv_indev_t *indev = lv_indev_active();
	if (!indev || !row || !spec) {
		return false;
	}
	lv_point_t point;
	lv_indev_get_point(indev, &point);
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_PRESSED) {
		// A spring still running from the last swipe ends now.
		if (icon && row_swiped && lv_obj_is_valid(row_swiped)) {
			lv_anim_delete(row_swiped, spring_exec);
			finish(row_swiped);
		}
		row_now = row;
		row_swiped = NULL;
		// A sideways drag on a row is the row's: it must not chain out to a
		// horizontally scrolling ancestor, which would take the press away
		// before the swipe could claim it. Vertical scrolling still chains.
		lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
		origin = point;
		taken = armed = false;
		vertical_scroll = false;
		direction = 0;
		offset = 0;
		return false;
	}
	if (row != row_now || origin.x <= EDGE) {
		return false;
	}
	int dx = point.x - origin.x, dy = point.y - origin.y;

	if (code == LV_EVENT_PRESSING) {
		if (!taken) {
			// Once a press belongs to vertical scrolling it stays there, even
			// if the finger hooks sideways or returns near its starting height.
			if (lv_indev_get_scroll_obj(indev) || (abs(dy) >= TAKE_DISTANCE && abs(dy) >= abs(dx)))
				vertical_scroll = true;
			if (vertical_scroll)
				return false;
			int want = dx > 0 ? 1 : -1;
			bool offered = want > 0 ? spec->right_icon != NULL : spec->left_icon != NULL;
			if (abs(dx) < TAKE_DISTANCE || abs(dx) < 2 * abs(dy) || !offered) {
				return false;
			}
			direction = want;
			begin(row, spec);
		}
		// Only toward the side chosen: the content does not cross back over.
		if (dx * direction < 0) {
			dx = 0;
		}
		offset = resisted(dx);
		place_content(row, offset);
		bool now_armed = abs(dx) >= ROWSWIPE_COMMIT;
		if (now_armed != armed) {
			armed = now_armed;
			paint(row, spec);
		}
		return true;
	}

	if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
		row_now = NULL;
		if (!taken) {
			return false;
		}
		taken = false;
		if (committed && code == LV_EVENT_RELEASED && dx * direction >= ROWSWIPE_COMMIT) {
			*committed = direction;
		}
		spring_back(row);
		return true;
	}
	return taken && row == row_now;
}

void rowswipe_settle(void) {
	if (icon && row_swiped && lv_obj_is_valid(row_swiped)) {
		lv_anim_delete(row_swiped, spring_exec);
		finish(row_swiped);
	}
}

bool rowswipe_active(void) { return taken && row_now != NULL; }

bool rowswipe_was_swiped(lv_obj_t *row) { return row && row == row_swiped; }
