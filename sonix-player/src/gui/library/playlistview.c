/* Collection-oriented playlist page. No file browser or file mutations. */
#include "playlistview.h"
#include "collectionview.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/nowplaying/player.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/shell/main_menu.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static lv_obj_t *screen, *heading, *list, *sheet, *field, *results, *edit_button, *images[12];
static keyboard_t *keyboard;
static char name[201];
static bool editing;
static int page, count, found, search_page;
static unsigned revision;
static library_playlist_row_t tracks[12], hits[12];
static cover_image_t covers[4];
static int art_rows[4] = {-1, -1, -1, -1};
static void draw(void);
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
static void release_art(void) {
	for (int i = 0; i < 4; i++) {
		coverloader_release(190 + i);
		if (art_rows[i] >= 0 && images[art_rows[i]])
			lv_image_set_src(images[art_rows[i]], &icon_album);
		cover_free(&covers[i]);
		art_rows[i] = -1;
	}
	memset(images, 0, sizeof(images));
}
static void play_at(int offset, bool shuffle) {
	library_index_t *ix = library_index_open(LIBRARY_LIST_PLAYLIST, LIBRARY_FILTER_NONE, name, LIBRARY_ORDER_DEFAULT, false);
	if (!ix)
		return;
	collections_touch(COLLECTION_PLAYLIST, name);
	if (device_state_play_index(ix, offset)) {
		playlist_set_shuffle(shuffle);
		device_state_queue_changed();
		player_refresh_now_playing();
		player_sheet_open(true);
	}
}
static void play_cb(lv_event_t *e) { play_at(0, (intptr_t)lv_event_get_user_data(e) != 0); }
static void track_cb(lv_event_t *e) {
	if (!editing)
		play_at(page * 12 + (int)(intptr_t)lv_event_get_user_data(e), playlist_shuffle_enabled());
}
static void change(lv_event_t *e) {
	int code = (int)(intptr_t)lv_event_get_user_data(e), row = code / 3, action = code % 3, pos = page * 12 + row;
	if (action == 2)
		playlists_remove_positions(name, &pos, 1);
	else {
		int to = pos + (action ? 1 : -1);
		if (to >= 0 && to < playlists_count_tracks(name))
			library_playlist_move(name, pos, to);
	}
	draw();
}
static void page_cb(lv_event_t *e) {
	page += (int)(intptr_t)lv_event_get_user_data(e);
	if (page < 0)
		page = 0;
	draw();
}
static void edit_cb(lv_event_t *e) {
	(void)e;
	editing = !editing;
	draw();
}
static void options(lv_event_t *e) {
	(void)e;
	collectionview_collection_options(COLLECTION_PLAYLIST, name, name, "You");
}
static void add_cb(lv_event_t *e) {
	(void)e;
	playlistview_add_songs();
}
static void draw(void) {
	if (!screen)
		return;
	release_art();
	lv_obj_clean(list);
	lv_obj_update_layout(list);
	revision = library_revision(LIBRARY_LIST_PLAYLIST);
	if (page && page * 12 >= playlists_count_tracks(name))
		page--;
	count = library_playlist_page(name, page * 12, 12, tracks);
	int w = lv_obj_get_width(list);
	lv_label_set_text(heading, name);
	lv_label_set_text(lv_obj_get_child(edit_button, 0), editing ? "Done" : "Edit");
	for (int i = 0; i < count; i++) {
		lv_obj_t *r = button(list, NULL, w, editing ? 128 : 84, track_cb, (void *)(intptr_t)i);
		images[i] = lv_image_create(r);
		lv_image_set_src(images[i], &icon_album);
		lv_obj_set_size(images[i], 56, 56);
		lv_image_set_inner_align(images[i], LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_pos(images[i], 0, 6);
		lv_obj_t *l = label(r, tracks[i].title, w - 72, 30, &font_ui_20, false);
		lv_obj_set_pos(l, 70, 5);
		l = label(r, tracks[i].artist, w - 72, 24, &font_ui_16, true);
		lv_obj_set_pos(l, 70, 38);
		if (editing) {
			const char *names[] = {"Up", "Down", "Remove"};
			for (int a = 0; a < 3; a++) {
				lv_obj_t *b = button(r, names[a], (w - 16) / 3, 48, change, (void *)(intptr_t)(i * 3 + a));
				lv_obj_set_pos(b, a * ((w - 16) / 3 + 8), 76);
				lv_obj_set_style_bg_opa(b, 255, 0);
				lv_obj_set_style_bg_color(b, lv_color_hex(0x282828), 0);
				lv_obj_set_style_radius(b, 8, 0);
			}
		}
	}
	if (!count)
		label(list, "Add songs to start this playlist", w, 40, &font_ui_20, true);
	if (page)
		button(list, "Previous songs", w, 48, page_cb, (void *)-1);
	if (count == 12)
		button(list, "More songs", w, 48, page_cb, (void *)1);
}
static void tick(lv_timer_t *t) {
	(void)t;
	if (!screen || lv_screen_active() != screen)
		return;
	if (!playlists_exists(name)) {
		switch_screen(main_menu_screen);
		return;
	}
	if (revision != library_revision(LIBRARY_LIST_PLAYLIST) && !bottomsheet_active())
		draw();
	lv_area_t area;
	lv_obj_get_coords(list, &area);
	for (int i = 0; i < 4; i++)
		if (art_rows[i] >= 0) {
			lv_area_t r;
			lv_obj_get_coords(images[art_rows[i]], &r);
			if (r.y2 < area.y1 || r.y1 > area.y2) {
				lv_image_set_src(images[art_rows[i]], &icon_album);
				coverloader_release(190 + i);
				cover_free(&covers[i]);
				art_rows[i] = -1;
			} else if (!covers[i].pixels) {
				bool done = false;
				if (coverloader_take(190 + i, &covers[i], &done))
					lv_image_set_src(images[art_rows[i]], &covers[i].dsc);
			}
		}
	for (int j = 0; j < count; j++) {
		lv_area_t r;
		lv_obj_get_coords(images[j], &r);
		if (r.y2 < area.y1 || r.y1 > area.y2)
			continue;
		bool held = false;
		int free = -1;
		for (int i = 0; i < 4; i++) {
			if (art_rows[i] == j)
				held = true;
			if (art_rows[i] < 0)
				free = i;
		}
		if (!held && free >= 0) {
			art_rows[free] = j;
			coverloader_request(190 + free, tracks[j].path, 56);
		}
	}
}
static void loaded(lv_event_t *e) {
	(void)e;
	main_menu_mount_footer(screen);
	draw();
}
static void unloaded(lv_event_t *e) {
	(void)e;
	main_menu_restore_footer(screen);
	release_art();
}
void playlistview_open(const char *value, bool edit) {
	if (!value || !*value)
		return;
	snprintf(name, sizeof(name), "%s", value);
	editing = edit;
	page = 0;
	if (!screen) {
		int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
		screen = box(NULL, w, h);
		lv_obj_set_style_bg_color(screen, lv_color_hex(0x121212), 0);
		lv_obj_set_style_bg_opa(screen, 255, 0);
		heading = label(screen, "", w - 100, 92, &font_ui_24_bold, false);
		lv_obj_set_pos(heading, 76, 68);
		lv_obj_t *more = button(screen, "...", 44, 44, options, NULL);
		lv_obj_set_pos(more, w - 54, 160);
		lv_obj_t *b = button(screen, "Play", 98, 48, play_cb, NULL);
		lv_obj_set_pos(b, 16, 164);
		lv_obj_set_style_bg_color(b, lv_color_hex(0x167c40), 0);
		lv_obj_set_style_bg_opa(b, 255, 0);
		lv_obj_set_style_radius(b, 24, 0);
		b = button(screen, "Shuffle", 104, 48, play_cb, (void *)1);
		lv_obj_set_pos(b, 122, 164);
		b = button(screen, "Add songs", 126, 48, add_cb, NULL);
		lv_obj_set_pos(b, 16, 220);
		edit_button = button(screen, "Edit", 76, 48, edit_cb, NULL);
		lv_obj_set_pos(edit_button, w - 92, 220);
		list = box(screen, w - 32, h - 426);
		lv_obj_set_pos(list, 16, 280);
		lv_obj_set_scrollable(list, true);
		lv_obj_set_scroll_dir(list, LV_DIR_VER);
		lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_style_pad_row(list, 6, 0);
		lv_obj_add_event_cb(screen, loaded, LV_EVENT_SCREEN_LOADED, NULL);
		lv_obj_add_event_cb(screen, unloaded, LV_EVENT_SCREEN_UNLOADED, NULL);
		lv_timer_create(tick, 140, NULL);
	}
	if (lv_screen_active() == screen)
		draw();
	else
		switch_screen(screen);
}
void playlistview_renamed(const char *old, const char *value) {
	if (!strcmp(name, old)) {
		snprintf(name, sizeof(name), "%s", value);
		if (screen)
			lv_label_set_text(heading, name);
	}
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
	for (int i = 0; i < found; i++) {
		lv_obj_t *r = button(results, NULL, w, 64, choose, (void *)(intptr_t)i);
		lv_obj_t *l = label(r, hits[i].title, w - 48, 48, &font_ui_18, false);
		lv_obj_set_pos(l, 0, 8);
		l = label(r, playlists_has_track(name, hits[i].path) ? "Added" : "+", 44, 24, &font_ui_14, true);
		lv_obj_align(l, LV_ALIGN_RIGHT_MID, 0, 0);
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
	lv_obj_set_height(results, lv_obj_get_height(sheet) - 132);
}
static void find_focus(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(keyboard, true);
	lv_obj_set_height(results, lv_obj_get_height(sheet) - 402);
}
static void find_cleanup(void) {
	sheet = field = results = NULL;
	keyboard = NULL;
	draw();
}
void playlistview_add_songs(void) {
	search_page = 0;
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
	sheet = bottomsheet_open(h - 60, find_cleanup);
	lv_obj_t *l = label(sheet, "Add songs", w - 40, 38, &font_ui_24_bold, false);
	lv_obj_set_pos(l, 20, 10);
	field = lv_textarea_create(sheet);
	lv_obj_add_style(field, &theme_style_card, 0);
	lv_obj_add_style(field, &theme_style_text, 0);
	lv_obj_set_size(field, w - 40, 56);
	lv_obj_set_pos(field, 20, 58);
	lv_textarea_set_one_line(field, true);
	lv_textarea_set_max_length(field, 200);
	lv_textarea_set_placeholder_text(field, "Search local songs");
	lv_obj_set_style_text_font(field, &font_ui_20, 0);
	results = box(sheet, w - 40, lv_obj_get_height(sheet) - 402);
	lv_obj_set_pos(results, 20, 124);
	lv_obj_set_scrollable(results, true);
	lv_obj_set_scroll_dir(results, LV_DIR_VER);
	lv_obj_set_flex_flow(results, LV_FLEX_FLOW_COLUMN);
	keyboard = keyboard_create(sheet, w, 270, field, NULL, "Done", find_done, NULL);
	lv_obj_add_event_cb(field, find_draw, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(field, find_focus, LV_EVENT_CLICKED, NULL);
	find_draw(NULL);
}
