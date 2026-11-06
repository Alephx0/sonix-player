/* Playlist actions use the shared album/collection page, never a second browser. */
#include "playlistview.h"
#include "medialist.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/shell/theme.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static lv_obj_t *sheet, *field, *results;
static keyboard_t *keyboard;
static char name[201];
static int found, search_page;
static library_playlist_row_t hits[12];
static lv_obj_t *box(lv_obj_t *p, int w, int h) {
	lv_obj_t *b = lv_obj_create(p);
	lv_obj_remove_style_all(b);
	lv_obj_set_size(b, w, h);
	lv_obj_set_scrollable(b, false);
	return b;
}
static lv_obj_t *label(lv_obj_t *p, const char *s, int w, int h, const lv_font_t *font, bool dim) {
	lv_obj_t *l = lv_label_create(p);
	lv_label_set_text(l, s);
	lv_obj_set_size(l, w, h);
	lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
	lv_obj_set_style_text_font(l, font, 0);
	lv_obj_set_style_text_color(l, lv_color_hex(dim ? 0xb3b3b3 : 0xffffff), 0);
	return l;
}
static lv_obj_t *button(lv_obj_t *p, const char *s, int w, int h, lv_event_cb_t cb, void *u) {
	lv_obj_t *b = box(p, w, h);
	lv_obj_set_clickable(b, true);
	lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, u);
	if (s) {
		lv_obj_t *l = label(b, s, w - 8, 26, &font_ui_18, false);
		lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_center(l);
	}
	return b;
}
void playlistview_open(const char *value, bool edit) {
	if (!value || !*value)
		return;
	snprintf(name, sizeof(name), "%s", value);
	medialist_open(value, LIBRARY_LIST_PLAYLIST, LIBRARY_FILTER_NONE, value);
	if (edit)
		medialist_edit_playlist();
}
void playlistview_renamed(const char *old, const char *value) {
	if (!strcmp(name, old))
		snprintf(name, sizeof(name), "%s", value);
	medialist_playlist_renamed(old, value);
}
static void find_draw(lv_event_t *e);
static void find_page(lv_event_t *e) {
	search_page += (int)(intptr_t)lv_event_get_user_data(e);
	find_draw(NULL);
}
static void choose(lv_event_t *e) {
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	if (!playlists_has_track(name, hits[i].path) && !playlists_add_track(name, hits[i].path))
		gui_notify_popup("Could not add song");
	find_draw(NULL);
}
static void find_draw(lv_event_t *e) {
	(void)e;
	if (e)
		search_page = 0;
	found = library_song_search_page(lv_textarea_get_text(field), search_page * 12, 12, hits);
	lv_obj_clean(results);
	lv_obj_update_layout(results);
	int w = lv_obj_get_width(results);
	/* Spotify's "Add to this playlist" rows: a sleeve, the title over the
	 * artist, and a circled plus that becomes a green check once added. */
	for (int i = 0; i < found; i++) {
		bool added = playlists_has_track(name, hits[i].path);
		lv_obj_t *r = button(results, NULL, w, 68, choose, (void *)(intptr_t)i);
		lv_obj_set_style_bg_color(r, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
		lv_obj_set_style_bg_opa(r, LV_OPA_COVER, LV_STATE_PRESSED);
		lv_obj_set_style_radius(r, 6, 0);
		lv_obj_t *art = box(r, 52, 52);
		lv_obj_set_pos(art, 0, 8);
		lv_obj_set_style_bg_color(art, lv_color_hex(0x333333), 0);
		lv_obj_set_style_bg_opa(art, LV_OPA_COVER, 0);
		lv_obj_set_style_radius(art, 4, 0);
		lv_obj_t *note = lv_image_create(art);
		lv_image_set_src(note, &icon_music_note);
		lv_obj_set_size(note, 26, 26);
		lv_image_set_inner_align(note, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_style_image_recolor(note, lv_color_hex(0xb3b3b3), 0);
		lv_obj_set_style_image_recolor_opa(note, LV_OPA_COVER, 0);
		lv_obj_center(note);
		lv_obj_t *l = label(r, hits[i].title, w - 116, 28, &font_ui_20, false);
		lv_obj_set_pos(l, 66, hits[i].artist[0] ? 8 : 20);
		if (hits[i].artist[0]) {
			l = label(r, hits[i].artist, w - 116, 24, &font_ui_16, true);
			lv_obj_set_pos(l, 66, 38);
		}
		lv_obj_t *mark = lv_image_create(r);
		lv_image_set_src(mark, added ? &icon_circle_check : &icon_plus);
		lv_obj_set_size(mark, 30, 30);
		lv_image_set_inner_align(mark, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_style_image_recolor(mark, lv_color_hex(added ? 0x1ed760 : 0xffffff), 0);
		lv_obj_set_style_image_recolor_opa(mark, LV_OPA_COVER, 0);
		if (!added) {
			lv_obj_set_style_border_width(mark, 2, 0);
			lv_obj_set_style_border_color(mark, lv_color_hex(0xb3b3b3), 0);
			lv_obj_set_style_radius(mark, LV_RADIUS_CIRCLE, 0);
			lv_obj_set_style_pad_all(mark, 5, 0);
		}
		lv_obj_align(mark, LV_ALIGN_RIGHT_MID, -6, 0);
	}
	if (search_page)
		button(results, "Previous songs", w, 48, find_page, (void *)-1);
	if (found == 12)
		button(results, "More songs", w, 48, find_page, (void *)1);
	if (!found)
		label(results, "No matching songs", w, 32, &font_ui_18, true);
}
static void find_done(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(keyboard, false);
	lv_obj_set_height(results, lv_obj_get_height(sheet) - 116);
}
static void find_focus(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(keyboard, true);
	lv_obj_set_height(results, lv_obj_get_height(sheet) - 386);
}
static void find_cleanup(void) {
	sheet = field = results = NULL;
	keyboard = NULL;
	medialist_refresh_visible();
}
void playlistview_add_songs(void) {
	search_page = 0;
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
	sheet = bottomsheet_open(h - 60, find_cleanup);
	lv_obj_t *l = label(sheet, "Add to this playlist", w - 100, 34, &font_ui_24_bold, false);
	lv_obj_set_pos(l, 20, 4);
	field = lv_textarea_create(sheet);
	lv_obj_remove_style_all(field);
	lv_obj_set_size(field, w - 40, 48);
	lv_obj_set_pos(field, 20, 50);
	lv_textarea_set_one_line(field, true);
	lv_textarea_set_max_length(field, 200);
	lv_textarea_set_placeholder_text(field, "Search");
	lv_obj_set_style_text_font(field, &font_ui_20, 0);
	lv_obj_set_style_text_color(field, lv_color_white(), 0);
	lv_obj_set_style_text_color(field, lv_color_hex(0x8a8a8a), LV_PART_TEXTAREA_PLACEHOLDER);
	lv_obj_set_style_bg_color(field, lv_color_hex(0x333333), 0);
	lv_obj_set_style_bg_opa(field, LV_OPA_COVER, 0);
	lv_obj_set_style_radius(field, 6, 0);
	lv_obj_set_style_pad_hor(field, 14, 0);
	lv_obj_set_style_pad_ver(field, 11, 0);
	keyboard_style_caret(field);
	results = box(sheet, w - 40, lv_obj_get_height(sheet) - 116);
	lv_obj_set_pos(results, 20, 108);
	lv_obj_set_scrollable(results, true);
	lv_obj_set_scroll_dir(results, LV_DIR_VER);
	lv_obj_set_flex_flow(results, LV_FLEX_FLOW_COLUMN);
	keyboard = keyboard_create(sheet, w, 270, field, NULL, "Done", find_done, NULL);
	keyboard_set_visible(keyboard, false); // the list first; the keyboard on a tap
	lv_obj_add_event_cb(field, find_draw, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(field, find_focus, LV_EVENT_CLICKED, NULL);
	find_draw(NULL);
}
