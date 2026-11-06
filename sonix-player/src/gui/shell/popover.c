#include "popover.h"

#include <stdio.h>
#include <string.h>

#include "src/gui/fonts/fonts.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/theme.h"
#include "src/system/core/lang.h"

#define POPOVER_MAX_ITEMS 8
#define POPOVER_WIDTH 284
#define POPOVER_ROW_HEIGHT 62
#define POPOVER_RADIUS 16
#define POPOVER_MARGIN 10 // gap between the anchor and the card, and to screen edges

static lv_obj_t *veil; // full-screen, catches the tap that dismisses
static lv_obj_t *card;

static popover_item_t items_copy[POPOVER_MAX_ITEMS];
static int items_count;

// Whether the menu is currently shown in the shared sheet.
static bool in_sheet;
static void sheet_cleanup(void) { in_sheet = false; }

// The heading the next sheet shows over its rows ("Sleep timer"), or none.
static char next_title[64];

void popover_close(void) {
	if (veil) {
		lv_obj_set_hidden(veil, true);
	}
	if (in_sheet) {
		bottomsheet_close(false);
	}
}

static void veil_clicked_cb(lv_event_t *e) {
	(void)e;
	popover_close();
}

static void item_clicked_cb(lv_event_t *e) {
	int index = (int)(intptr_t)lv_event_get_user_data(e);
	if (index < 0 || index >= items_count) {
		return;
	}

	// Close first: the action may well open another page, and the veil must
	// not linger over it.
	popover_item_t item = items_copy[index];
	popover_close();

	if (item.action) {
		item.action(item.user);
	}
}

void popover_init(void) {
	if (veil) {
		return;
	}

	veil = lv_obj_create(lv_layer_top());
	lv_obj_set_size(veil, lv_pct(100), lv_pct(100));
	lv_obj_set_style_bg_color(veil, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(veil, LV_OPA_60, 0); // heavy dim: the menu owns the screen
	lv_obj_set_style_border_width(veil, 0, 0);
	lv_obj_set_style_radius(veil, 0, 0);
	lv_obj_set_style_pad_all(veil, 0, 0);
	lv_obj_set_scrollable(veil, false);
	lv_obj_set_hidden(veil, true);
	lv_obj_add_event_cb(veil, veil_clicked_cb, LV_EVENT_CLICKED, NULL);

	card = lv_obj_create(veil);
	lv_obj_set_width(card, POPOVER_WIDTH);
	lv_obj_set_height(card, LV_SIZE_CONTENT);
	lv_obj_add_style(card, &theme_style_card, 0);
	lv_obj_set_style_radius(card, POPOVER_RADIUS, 0);
	lv_obj_set_style_border_width(card, 0, 0);
	lv_obj_set_style_pad_all(card, 8, 0);
	lv_obj_set_style_pad_gap(card, 0, 0);
	lv_obj_set_style_shadow_width(card, 0, 0);
	lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_scrollable(card, false);
	// A tap on the card itself must not fall through to the veil.
	lv_obj_set_event_bubble(card, false);
}

// The reference layout shows these menus as Spotify does: in the shared bottom
// sheet, one row per choice, the current one marked with a green check. While
// another sheet is already up the anchored card is used instead, because
// opening a sheet would replace the content the menu was opened from.
#define SHEET_ROW_H 60
#define SHEET_TOP 48	   // with a heading
#define SHEET_TOP_PLAIN 8 // without one, the rows start under the handle

static void sheet_item_cb(lv_event_t *e) {
	int index = (int)(intptr_t)lv_event_get_user_data(e);
	if (index < 0 || index >= items_count) {
		return;
	}
	popover_item_t item = items_copy[index];
	bottomsheet_close(false);
	if (item.action) {
		item.action(item.user);
	}
}

static void sheet_show(int count) {
	int w = lv_display_get_horizontal_resolution(lv_display_get_default());
	int top = next_title[0] ? SHEET_TOP : SHEET_TOP_PLAIN;
	// The sheet's body starts below its handle; leave room for that and a margin.
	lv_obj_t *sheet = bottomsheet_open(top + count * SHEET_ROW_H + 60, sheet_cleanup);
	in_sheet = true;
	if (next_title[0]) {
		lv_obj_t *heading = lv_label_create(sheet);
		lv_label_set_text(heading, tr(next_title));
		lv_obj_set_style_text_font(heading, &font_ui_24_bold, 0);
		lv_obj_set_style_text_color(heading, lv_color_white(), 0);
		lv_obj_set_pos(heading, 22, 0);
	}
	for (int i = 0; i < count; i++) {
		lv_obj_t *row = lv_obj_create(sheet);
		lv_obj_remove_style_all(row);
		lv_obj_set_size(row, w - 24, SHEET_ROW_H);
		lv_obj_set_pos(row, 12, top + i * SHEET_ROW_H);
		lv_obj_set_style_radius(row, 8, 0);
		lv_obj_set_style_bg_color(row, lv_color_make(42, 42, 42), LV_STATE_PRESSED);
		lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
		lv_obj_set_clickable(row, true);
		lv_obj_add_event_cb(row, sheet_item_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);

		lv_obj_t *label = lv_label_create(row);
		lv_label_set_text(label, tr(items_copy[i].label));
		lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
		lv_obj_set_width(label, w - 90);
		lv_obj_add_style(label, &theme_style_text, 0);
		lv_obj_set_style_text_font(label, &font_ui_20, 0);
		lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
		if (items_copy[i].checked) {
			lv_obj_set_style_text_color(label, theme()->accent, 0);
			lv_obj_t *check = lv_image_create(row);
			lv_image_set_src(check, &icon_check);
			lv_obj_set_style_image_recolor(check, theme()->accent, 0);
			lv_obj_set_style_image_recolor_opa(check, LV_OPA_COVER, 0);
			lv_obj_align(check, LV_ALIGN_RIGHT_MID, -10, 0);
		}
	}
}

void popover_show(lv_obj_t *anchor, const popover_item_t *items, int count) {
	if (!veil || !items || count <= 0) {
		return;
	}
	if (count > POPOVER_MAX_ITEMS) {
		count = POPOVER_MAX_ITEMS;
	}

	memcpy(items_copy, items, (size_t)count * sizeof(items[0]));
	items_count = count;
	if (theme_reference() && !bottomsheet_active()) {
		sheet_show(count);
		next_title[0] = '\0';
		return;
	}
	next_title[0] = '\0';

	// Rebuild the rows.
	lv_obj_clean(card);
	for (int i = 0; i < count; i++) {
		lv_obj_t *row = lv_btn_create(card);
		// Tall enough for one line, taller when the text needs two: a
		// translated label such as "Zur Wiedergabeliste hinzufügen" does not
		// fit on one, and a fixed height would clip it at the card's edge.
		lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
		lv_obj_set_style_min_height(row, POPOVER_ROW_HEIGHT, 0);
		lv_obj_set_style_pad_ver(row, 12, 0);
		// With a check mark the entry lays out as a row: text left, mark right.
		// Without one it stays a column, which is what the two-line text of long
		// entries needs.
		lv_obj_set_flex_flow(row, items[i].checked ? LV_FLEX_FLOW_ROW : LV_FLEX_FLOW_COLUMN);
		lv_obj_set_flex_align(row, items[i].checked ? LV_FLEX_ALIGN_START : LV_FLEX_ALIGN_CENTER,
							  LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
		lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
		lv_obj_add_style(row, &theme_style_card_pressed, LV_STATE_PRESSED);
		lv_obj_set_style_radius(row, 10, 0);
		lv_obj_set_style_border_width(row, 0, 0);
		lv_obj_set_style_shadow_width(row, 0, 0);
		lv_obj_set_style_pad_hor(row, 14, 0);
		lv_obj_add_event_cb(row, item_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);

		lv_obj_t *label = lv_label_create(row);
		lv_label_set_text(label, tr(items[i].label));
		lv_obj_add_style(label, &theme_style_text, 0);
		lv_obj_set_style_text_font(label, &font_ui_24, 0);
		// Given a width, a label wraps instead of running past its parent. Left
		// aligned on both lines: a menu is read down its left edge, and centred
		// second lines would make that edge ragged.
		lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
		if (items[i].checked) {
			lv_obj_set_flex_grow(label, 1);

			lv_obj_t *check = lv_image_create(row);
			lv_image_set_src(check, &icon_check);
			lv_obj_set_style_image_recolor(check, theme()->accent, 0);
			lv_obj_set_style_image_recolor_opa(check, LV_OPA_COVER, 0);
		} else {
			lv_obj_set_width(label, lv_pct(100));
		}

		// A hairline between rows, not after the last.
		if (i + 1 < count) {
			lv_obj_t *sep = lv_obj_create(card);
			lv_obj_set_size(sep, lv_pct(94), 1);
			lv_obj_set_style_bg_color(sep, theme()->text_secondary, 0);
			lv_obj_set_style_bg_opa(sep, LV_OPA_20, 0);
			lv_obj_set_style_border_width(sep, 0, 0);
		}
	}

	// Place the card next to the anchor: below it when there is room, above
	// otherwise; slid horizontally to stay on screen.
	lv_area_t a;
	lv_obj_get_coords(anchor, &a);
	lv_obj_update_layout(card);

	int screen_w = lv_obj_get_width(veil);
	int screen_h = lv_obj_get_height(veil);
	int card_h = lv_obj_get_height(card);

	int x = a.x1 + (a.x2 - a.x1) / 2 - POPOVER_WIDTH / 2;
	if (x < POPOVER_MARGIN) {
		x = POPOVER_MARGIN;
	}
	if (x + POPOVER_WIDTH > screen_w - POPOVER_MARGIN) {
		x = screen_w - POPOVER_MARGIN - POPOVER_WIDTH;
	}

	int y = a.y2 + POPOVER_MARGIN; // preferred: right under the control
	if (y + card_h > screen_h - POPOVER_MARGIN) {
		y = a.y1 - POPOVER_MARGIN - card_h; // no room: above it
	}
	if (y < POPOVER_MARGIN) {
		y = POPOVER_MARGIN;
	}

	lv_obj_set_pos(card, x, y);

	lv_obj_set_hidden(veil, false);
	lv_obj_move_foreground(veil);
}

void popover_show_titled(lv_obj_t *anchor, const char *title, const popover_item_t *items, int count) {
	snprintf(next_title, sizeof(next_title), "%s", title ? title : "");
	popover_show(anchor, items, count);
}
