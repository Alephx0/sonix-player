#include "collectionview.h"
#include "playlistview.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/library/medialist.h"
#include "src/gui/library/playlistpage.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/nowplaying/player.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/shell/main_menu.h"
#include "src/gui/shell/popover.h"
#include "src/gui/shell/rowswipe.h"
#include "src/gui/shell/confirm.h"
#include "src/gui/shell/toast.h"
#include "src/gui/shell/reference_icons.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/system/core/config.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include <pthread.h>
#include "src/system/core/utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static gui_config_t *cfg;
/* Your Library scrolls continuously: rows are read a page at a time as the
 * list nears its end, and kept in a window of LIB_ROWS -- a ring, so a row's
 * item stays where its callbacks point while the window moves. Scrolled far
 * enough down, the first page goes; scrolled back up, it comes back. A
 * library of a thousand albums is never a thousand rows of widgets. */
#define LIB_PAGE 24
#define LIB_ROWS (4 * LIB_PAGE)
// The ring has a page more than the window: a page is added before the one at
// the other end goes, so the list never shortens under the reader.
#define LIB_SLOTS (LIB_ROWS + LIB_PAGE)
static lv_obj_t *row_images[LIB_SLOTS], *row_boxes[LIB_SLOTS];
static struct {
	int row;
	unsigned candidate; /* At most eight local artist recordings, one decode at a time. */
	cover_image_t image;
	bool loaded;
} art[8];
static void art_drop(void) {
	for (int i = 0; i < 8; i++) {
		coverloader_release(COVERLOADER_COLLECTION_BASE + i);
		if (art[i].loaded) {
			if (art[i].row >= 0 && art[i].row < LIB_SLOTS && row_images[art[i].row])
				lv_image_set_src(row_images[art[i].row], &icon_album);
			cover_free(&art[i].image);
		}
		art[i].loaded = false;
		art[i].row = -1;
	}
}
static bool sample_track(const char *name, const char *path, const char *artist, void *user) {
	(void)name;
	(void)artist;
	if (path && !access(path, R_OK)) {
		snprintf(user, 512, "%s", path);
		return false;
	}
	return true;
}
static void art_update(void);

static lv_obj_t *root, *list, *sheet, *sheet_body, *name_layer, *name_field;
static int filter, sort_mode, page;
static char folder[512], library_query[256];
static unsigned seen_music;
static bool suppress_open;
static lv_point_t row_press;
static int name_operation;
static void name_begin(int operation);
static void playlist_action(lv_event_t *e);
static void folder_back(lv_event_t *e) {
	(void)e;
	folder[0] = 0;
	page = 0;
	collectionview_refresh();
}
static bool grid;
static collection_item_t rows[LIB_SLOTS], menu_item;
// The window: items [lib_first, lib_first + lib_loaded) of the list, item a
// in slot a % LIB_SLOTS. lib_head is how many children of the list come before
// the rows (shortcuts, the folder's header).
static int lib_first, lib_loaded, lib_head;
static int lib_width; // the list's width, from the refresh: a list just made has none yet
static bool lib_end, lib_busy;
static lv_obj_t *lib_footer;
static char track[512], track_title[256], track_artist[256];
static int member_page;
static lv_obj_t *member_list, *member_search;
static keyboard_t *member_keyboard;
static char member_query[201];
static bool name_returns;
static lv_obj_t *member_art[8];
static cover_image_t member_covers[8];
static int member_art_count;
static lv_timer_t *member_timer;
static void member_art_drop(void) {
	for (int i = 0; i < 8; i++) {
		coverloader_release(COVERLOADER_MEMBERS_BASE + i);
		if (member_art[i])
			lv_image_set_src(member_art[i], &icon_files_playlist);
		cover_free(&member_covers[i]);
		member_art[i] = NULL;
	}
	member_art_count = 0;
}
static void member_art_tick(lv_timer_t *t) {
	(void)t;
	for (int i = 0; i < member_art_count; i++)
		if (member_art[i] && !member_covers[i].pixels) {
			bool done = false;
			if (coverloader_take(COVERLOADER_MEMBERS_BASE + i, &member_covers[i], &done))
				lv_image_set_src(member_art[i], &member_covers[i].dsc);
		}
}
static char member_names[24][201];
static int member_count, member_seen;
static unsigned seen_revision, seen_playlists;
static lv_timer_t *refresh_timer;
static lv_obj_t *text(lv_obj_t *p, const char *value, int width, const lv_font_t *font, bool dim) {
	lv_obj_t *o = lv_label_create(p);
	lv_obj_set_width(o, width);
	lv_label_set_text(o, value);
	lv_label_set_long_mode(o, LV_LABEL_LONG_DOT);
	lv_obj_set_height(o, font->line_height + 4);
	lv_obj_add_style(o, dim ? &theme_style_text_dim : &theme_style_text, 0);
	lv_obj_set_style_text_font(o, font, 0);
	return o;
}
static lv_obj_t *panel(lv_obj_t *p, int w, int h) {
	lv_obj_t *o = lv_obj_create(p);
	lv_obj_remove_style_all(o);
	lv_obj_add_style(o, &theme_style_screen, 0);
	lv_obj_set_size(o, w, h);
	lv_obj_set_style_pad_all(o, 0, 0);
	lv_obj_set_scrollable(o, false);
	if (theme_is_dark())
		lv_obj_set_style_bg_color(o, lv_color_hex(0x121212), 0);
	return o;
}
static lv_obj_t *button(lv_obj_t *p, const char *s, int w, int h, lv_event_cb_t cb, void *u) {
	lv_obj_t *o = panel(p, w, h);
	lv_obj_add_style(o, &theme_style_card, 0);
	lv_obj_set_style_pad_all(o, 0, 0);
	lv_obj_set_style_radius(o, 8, 0);
	lv_obj_set_clickable(o, true);
	if (s) {
		lv_obj_t *l = text(o, s, w - 16, &font_ui_18, false);
		lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_center(l);
	}
	lv_obj_add_event_cb(o, cb, LV_EVENT_CLICKED, u);
	return o;
}
/* Several chosen tracks on their way to one destination (selection mode's
 * "Add to playlist"): the sheet lists destinations instead of memberships. */
static char **bulk_paths;
static int bulk_count;
static void bulk_free(void) {
	for (int i = 0; i < bulk_count; i++)
		free(bulk_paths[i]);
	free(bulk_paths);
	bulk_paths = NULL;
	bulk_count = 0;
}
static void sheet_cleanup(void) {
	bulk_free();
	member_art_drop();
	sheet = sheet_body = name_layer = name_field = member_list = member_search = NULL;
	member_keyboard = NULL;
	suppress_open = false;
	player_refresh_membership();
	collectionview_refresh();
}
static void close_sheet(lv_event_t *e) {
	(void)e;
	bottomsheet_close(true);
}
/* The shared sheet: an optional heading under the handle (the close button
 * is the sheet's own), then a scrolling column. `height` is the whole sheet. */
static void sheet_open_sized(const char *title, int height) {
	int w = lv_display_get_horizontal_resolution(NULL);
	sheet = bottomsheet_open(height, sheet_cleanup);
	int top = 4;
	if (title && *title) {
		lv_obj_t *l = text(sheet, title, w - 100, &font_ui_24_bold, false);
		lv_obj_set_pos(l, 20, 4);
		top = 52;
	}
	sheet_body = panel(sheet, w - 32, lv_obj_get_height(sheet) - top - 12);
	lv_obj_set_pos(sheet_body, 16, top);
	lv_obj_set_style_bg_opa(sheet_body, 0, 0);
	lv_obj_set_scrollable(sheet_body, true);
	lv_obj_set_scroll_dir(sheet_body, LV_DIR_VER);
	lv_obj_set_flex_flow(sheet_body, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(sheet_body, 10, 0);
	lv_obj_update_layout(sheet_body);
}
static void sheet_new(const char *title) { sheet_open_sized(title, lv_display_get_vertical_resolution(NULL) - 60); }
/* A menu sized to its rows, the way Spotify's option sheets rise only as far
 * as they need. */
static void sheet_menu(const char *title, int rows_height) {
	sheet_open_sized(title, 36 + (title && *title ? 52 : 4) + rows_height + 24);
	lv_obj_set_style_pad_row(sheet_body, 0, 0);
}
#define MENU_ROW 56
#define MENU_ROW_TALL 76
/* One option: an icon, the title, an optional line under it, and a green
 * check on the current choice. With a subtitle the icon sits in a circle
 * (Spotify's Create sheet). */
static lv_obj_t *sheet_row(const lv_image_dsc_t *icon, const char *title, const char *subtitle, bool checked,
						   lv_event_cb_t cb, void *u) {
	int w = lv_obj_get_width(sheet_body), rh = subtitle ? MENU_ROW_TALL : MENU_ROW;
	lv_obj_t *row = lv_obj_create(sheet_body);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, w, rh);
	lv_obj_set_scrollable(row, false);
	lv_obj_set_clickable(row, true);
	lv_obj_set_event_bubble(row, true);
	lv_obj_set_style_radius(row, 8, 0);
	lv_obj_set_style_bg_color(row, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
	lv_obj_add_event_cb(row, cb, LV_EVENT_CLICKED, u);
	int x = 6;
	if (icon) {
		lv_obj_t *holder = row;
		if (subtitle) {
			holder = lv_obj_create(row);
			lv_obj_remove_style_all(holder);
			lv_obj_set_size(holder, 52, 52);
			lv_obj_set_style_radius(holder, LV_RADIUS_CIRCLE, 0);
			lv_obj_set_style_bg_color(holder, lv_color_hex(0x3e3e3e), 0);
			lv_obj_set_style_bg_opa(holder, LV_OPA_COVER, 0);
			lv_obj_remove_flag(holder, LV_OBJ_FLAG_CLICKABLE);
			lv_obj_set_pos(holder, 4, (rh - 52) / 2);
		}
		lv_obj_t *im = lv_image_create(holder);
		lv_image_set_src(im, icon);
		lv_obj_set_size(im, 26, 26);
		lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_style_image_recolor(im, subtitle ? lv_color_white() : lv_color_hex(0xb3b3b3), 0);
		lv_obj_set_style_image_recolor_opa(im, LV_OPA_COVER, 0);
		if (subtitle)
			lv_obj_center(im);
		else
			lv_obj_align(im, LV_ALIGN_LEFT_MID, 8, 0);
		x = subtitle ? 72 : 52;
	}
	const lv_font_t *font = subtitle ? &font_ui_20_bold : &font_ui_20;
	lv_obj_t *l = text(row, title, w - x - (checked ? 44 : 8), font, false);
	lv_obj_set_style_text_color(l, checked ? lv_color_hex(0x1ed760) : lv_color_white(), 0);
	lv_obj_set_pos(l, x, subtitle ? 12 : (rh - (font->line_height + 4)) / 2);
	if (subtitle) {
		lv_obj_t *sub = text(row, subtitle, w - x - 8, &font_ui_16, true);
		lv_obj_set_style_text_color(sub, lv_color_hex(0xb3b3b3), 0);
		lv_obj_set_pos(sub, x, 42);
	}
	if (checked) {
		lv_obj_t *check = lv_image_create(row);
		lv_image_set_src(check, &icon_check);
		lv_obj_set_style_image_recolor(check, lv_color_hex(0x1ed760), 0);
		lv_obj_set_style_image_recolor_opa(check, LV_OPA_COVER, 0);
		lv_obj_align(check, LV_ALIGN_RIGHT_MID, -8, 0);
	}
	return row;
}
static void open_row(lv_event_t *e) {
	collection_item_t value = *(collection_item_t *)lv_event_get_user_data(e), *r = &value;
	if (suppress_open || player_sheet_drag_active() || rowswipe_was_swiped(lv_event_get_current_target(e)))
		return;
	if (sheet)
		bottomsheet_close(false);
	if (r->kind == COLLECTION_FOLDER) {
		snprintf(folder, sizeof(folder), "%.511s", r->key);
		page = 0;
		collectionview_refresh();
		return;
	}
	collections_touch(r->kind, r->key);
	if (r->kind == COLLECTION_PLAYLIST)
		playlistview_open(r->key, false);
	else if (r->kind == COLLECTION_ALBUM)
		medialist_open(r->title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, r->key);
	else if (r->kind == COLLECTION_ARTIST)
		medialist_open_artist(r->key);
}
static void save_action(lv_event_t *e) {
	(void)e;
	bool saved = !(collections_get(menu_item.kind, menu_item.key, &menu_item) && menu_item.saved);
	if (!collections_put(menu_item.kind, menu_item.key, menu_item.title, menu_item.creator, saved))
		gui_notify_popup("Could not save this collection.");
	close_sheet(NULL);
}
static void pin_action(lv_event_t *e) {
	(void)e;
	collection_item_t current = {0};
	bool pinned = collections_get(menu_item.kind, menu_item.key, &current) && current.pin;
	if (!pinned && !current.saved)
		collections_put(menu_item.kind, menu_item.key, menu_item.title, menu_item.creator, true);
	if (!collections_pin(menu_item.kind, menu_item.key, !pinned))
		gui_notify_popup("You can pin up to four collections.");
	close_sheet(NULL);
}
void collectionview_collection_options(collection_kind_t kind, const char *key, const char *title, const char *creator) {
	if (!key || !*key)
		return;
	collections_init(playlist_card_root());
	memset(&menu_item, 0, sizeof(menu_item));
	menu_item.kind = kind;
	snprintf(menu_item.key, sizeof(menu_item.key), "%s", key);
	snprintf(menu_item.title, sizeof(menu_item.title), "%s", title ? title : key);
	snprintf(menu_item.creator, sizeof(menu_item.creator), "%s", creator ? creator : "");
	collection_item_t saved = {0};
	collections_get(kind, key, &saved);
	int rows = kind == COLLECTION_PLAYLIST ? 9 : kind == COLLECTION_FOLDER ? 4 : 3;
	sheet_menu(title ? title : key, rows * MENU_ROW);
	suppress_open = true;
	if (kind == COLLECTION_PLAYLIST || kind == COLLECTION_FOLDER) {
		if (kind == COLLECTION_PLAYLIST) {
			sheet_row(&icon_plus, "Add songs", NULL, false, playlist_action, (void *)1);
			sheet_row(&icon_reorder, "Edit tracks", NULL, false, playlist_action, (void *)2);
			sheet_row(&icon_list_queue, "Add to queue", NULL, false, playlist_action, (void *)3);
			sheet_row(&icon_folder, "Move to folder", NULL, false, playlist_action, (void *)4);
			sheet_row(&icon_album, "Choose cover", NULL, false, playlist_action, (void *)8);
		}
		sheet_row(&sonix_icon_pencil, "Rename", NULL, false, playlist_action, (void *)5);
		sheet_row(&icon_trash, kind == COLLECTION_FOLDER ? "Delete folder" : "Delete playlist", NULL, false, playlist_action,
				  (void *)6);
	} else
		sheet_row(kind == COLLECTION_ARTIST ? &icon_artist : &sonix_icon_heart,
				  kind == COLLECTION_ARTIST ? (saved.saved ? "Unfollow artist" : "Follow artist")
											: (saved.saved ? "Unsave album" : "Save album"),
				  NULL, false, save_action, NULL);
	sheet_row(&sonix_icon_pin, saved.pin ? "Unpin" : "Pin to top", NULL, false, pin_action, NULL);
	sheet_row(&icon_chevron_up, "First in custom order", NULL, false, playlist_action, (void *)7);
}
static void row_options(lv_event_t *e) {
	if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED) {
		lv_indev_t *in = lv_indev_active();
		lv_point_t now;
		if (in) {
			lv_indev_get_point(in, &now);
			if (LV_ABS(now.x - row_press.x) > 10 || LV_ABS(now.y - row_press.y) > 10)
				return;
		}
	}
	collection_item_t *r = lv_event_get_user_data(e);
	collectionview_collection_options(r->kind, r->key, r->title, r->creator);
}
static void after_pin(void *unused) {
	(void)unused;
	suppress_open = false;
	collectionview_refresh();
}
/* Swipe right pins or unpins (the shared row swipe, green pin strip). */
static void row_swipe(lv_event_t *e) {
	lv_indev_t *in = lv_indev_active();
	if (!in || sheet)
		return;
	lv_point_t now;
	lv_indev_get_point(in, &now);
	if (lv_event_get_code(e) == LV_EVENT_PRESSED)
		row_press = now;
	static const rowswipe_spec_t spec = {&sonix_icon_pin, ROWSWIPE_GREEN, NULL, 0};
	int committed = 0;
	if (rowswipe_event(e, lv_event_get_current_target(e), &spec, &committed))
		lv_event_stop_bubbling(e);
	if (committed <= 0)
		return;
	rowswipe_settle(); // the list is rebuilt around the pinned row
	collection_item_t *r = lv_event_get_user_data(e), saved = {0};
	collections_get(r->kind, r->key, &saved);
	if (!saved.saved)
		collections_put(r->kind, r->key, r->title, r->creator, true);
	suppress_open = true;
	if (!collections_pin(r->kind, r->key, !saved.pin))
		gui_notify_popup("You can pin up to four collections.");
	lv_async_call(after_pin, NULL);
}
static void route(lv_event_t *e) {
	if ((intptr_t)lv_event_get_user_data(e) == 1)
		medialist_open("Liked Songs", LIBRARY_LIST_FAVOURITES, LIBRARY_FILTER_NONE, NULL);
	else
		medialist_open("All songs", LIBRARY_LIST_TRACKS, LIBRARY_FILTER_NONE, NULL);
}
static bool register_playlist(const char *name, void *user) {
	(void)user;
	collection_item_t existing;
	if (!collections_get(COLLECTION_PLAYLIST, name, &existing))
		collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
	return true;
}
void collectionview_set_filter(int chosen) {
	filter = chosen;
	folder[0] = 0;
	page = 0;
	config_set_int("collections", "filter", filter);
	config_save();
	collectionview_refresh();
}
/* Smart Filters (Spotify iOS, captured 2026-11-04): a round filter button
 * before the chips opens "Filter by"; offline there are no moods or
 * activities, so the sheet lists the library's genres from its tags, each
 * with how many albums and artists carry it. The chosen genre shows as a
 * green chip that clears it. Not kept across a restart. */
static char genre_filter[64];
#define GENRE_CHOICES 12
static char genre_names[GENRE_CHOICES][64];
static void genre_pick_cb(lv_event_t *e) {
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	snprintf(genre_filter, sizeof(genre_filter), "%s", i >= 0 ? genre_names[i] : "");
	page = 0;
	bottomsheet_close(true);
	collectionview_refresh();
}
static void genre_sheet_cb(lv_event_t *e) {
	(void)e;
	int counts[GENRE_CHOICES];
	int n = library_top_genres(genre_names, counts, GENRE_CHOICES);
	lv_obj_t *body = bottomsheet_open(560, NULL);
	lv_obj_set_style_pad_hor(body, 20, 0);
	lv_obj_set_flex_flow(body, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_pad_row(body, 10, 0);
	lv_obj_set_style_pad_column(body, 8, 0);
	lv_obj_update_layout(body);
	int w = lv_obj_get_content_width(body);
	lv_obj_t *t = text(body, "Filter by", w, &font_ui_22_bold, false);
	lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_t *h = text(body, "Genres", w, &font_ui_22_bold, false);
	lv_obj_set_style_pad_top(h, 8, 0);
	for (int i = 0; i < n; i++) {
		char label[96];
		snprintf(label, sizeof(label), "%s (%d)", genre_names[i], counts[i]);
		lv_obj_t *c = lv_button_create(body);
		lv_obj_set_size(c, LV_SIZE_CONTENT, 42);
		lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
		lv_obj_set_style_pad_hor(c, 18, 0);
		bool on = !strcasecmp(genre_filter, genre_names[i]);
		lv_obj_set_style_bg_color(c, on ? lv_color_hex(0x1ed760) : lv_color_hex(0x2a2a2a), 0);
		lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
		lv_obj_set_style_shadow_width(c, 0, 0);
		lv_obj_set_style_border_width(c, 0, 0);
		lv_obj_add_event_cb(c, genre_pick_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
		lv_obj_t *l = lv_label_create(c);
		lv_label_set_text(l, label);
		lv_obj_set_style_text_font(l, &font_ui_18, 0);
		lv_obj_set_style_text_color(l, on ? lv_color_black() : lv_color_white(), 0);
		lv_obj_center(l);
	}
	if (!n)
		text(body, "Your music has no genre tags yet", w, &font_ui_18, true);
}
static bool genre_match(const collection_item_t *r) {
	if (!genre_filter[0])
		return true;
	if (r->kind == COLLECTION_FOLDER)
		return false;
	int kind = r->kind == COLLECTION_ALBUM ? 0 : r->kind == COLLECTION_ARTIST ? 1 : 2;
	return library_in_genre(kind, r->key, genre_filter);
}

static void filters_cb(lv_event_t *e) {
	int next = (int)(intptr_t)lv_event_get_user_data(e);
	filter = filter == next ? 0 : next;
	folder[0] = 0;
	page = 0;
	config_set_int("collections", "filter", filter);
	config_save();
	collectionview_refresh();
}
static void sort_select(lv_event_t *e) {
	sort_mode = (int)(intptr_t)lv_event_get_user_data(e);
	page = 0;
	config_set_int("collections", "sort", sort_mode);
	config_save();
	bottomsheet_close(false);
	collectionview_refresh();
}
static void sort_cb(lv_event_t *e) {
	(void)e;
	sheet_menu("Sort by", 5 * MENU_ROW);
	const char *names[] = {"Recents", "Recently added", "Alphabetical", "Creator", "Custom"};
	for (int i = 0; i < 5; i++)
		sheet_row(NULL, names[i], NULL, i == sort_mode, sort_select, (void *)(intptr_t)i);
}
static void grid_cb(lv_event_t *e) {
	(void)e;
	grid = !grid;
	config_set_int("collections", "grid", grid);
	config_save();
	collectionview_refresh();
}
static void members_draw(void);
static void name_close(lv_event_t *e) {
	(void)e;
	if (name_layer) {
		lv_obj_delete(name_layer);
		name_layer = NULL;
	}
	if (!name_returns)
		bottomsheet_close(true);
}
/* The name offered when the field is left empty ("My playlist #4"), shown as
 * the field's hint: one tap on Create takes it, typing replaces it. */
static char default_name[64];
static void smart_make(int rule, const char *value);
static void name_accept(lv_event_t *e) {
	(void)e;
	char entered[201];
	snprintf(entered, sizeof(entered), "%s", lv_textarea_get_text(name_field));
	if (name_operation >= 10) { /* a smart playlist's genre, artist or year */
		if (!entered[0]) {
			gui_notify_popup("Type what the playlist should hold.");
			return;
		}
		smart_make(name_operation - 10, entered);
		return;
	}
	if (!entered[0] && name_operation < 2)
		snprintf(entered, sizeof(entered), "%s", default_name);
	const char *name = entered;
	if (name_operation) {
		bool ok = false;
		if (name_operation == 1)
			ok = collections_folder_create(name);
		else if (name_operation == 2 && strcasecmp(name, "Liked Songs") && playlists_rename(menu_item.key, name)) {
			ok = collections_rename(COLLECTION_PLAYLIST, menu_item.key, name, name);
			playlistview_renamed(menu_item.key, name);
		} else if (name_operation == 3) {
			collection_item_t exists;
			if (!collections_get(COLLECTION_FOLDER, name, &exists))
				ok = collections_rename(COLLECTION_FOLDER, menu_item.key, name, name);
			if (ok && !strcmp(folder, menu_item.key))
				snprintf(folder, sizeof(folder), "%s", name);
		}
		if (!ok) {
			gui_notify_popup("Choose an unused, valid name.");
			return;
		}
		bottomsheet_close(false);
		collectionview_refresh();
		return;
	}
	if (!strcasecmp(name, "Liked Songs") || !playlists_create(name)) {
		gui_notify_popup("Choose a new, valid playlist name.");
		return;
	}
	if (bulk_count) {
		/* The new playlist is the destination: the worker fills it and the
		 * snackbar reports it, so the sheet is done. */
		collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
		if (!playlistpage_add_tracks_to(name, (const char *const *)bulk_paths, bulk_count))
			gui_notify_popup("Playlist created, but the tracks could not be added.");
		bottomsheet_close(true);
		collectionview_refresh();
		return;
	}
	bool ok = !track[0] || playlists_add_track(name, track);
	collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
	if (folder[0] && !name_returns)
		collections_move_playlist(name, folder);
	if (name_returns && member_search) {
		snprintf(member_query, sizeof(member_query), "%s", name);
		member_page = 0;
		lv_textarea_set_text(member_search, member_query);
	}
	bool from_create = !name_returns;
	char created[201];
	snprintf(created, sizeof(created), "%s", name);
	name_close(NULL);
	if (!ok)
		gui_notify_popup("Playlist created, but the track could not be added.");
	if (sheet)
		members_draw();
	collectionview_refresh();
	/* From the Create sheet the new, empty playlist opens, ready for songs,
	 * as Spotify does. */
	if (from_create)
		playlistview_open(created, false);
}
static bool count_playlist(const char *name, void *user) {
	(void)name;
	(*(int *)user)++;
	return true;
}
/* Spotify's pill buttons: outlined (secondary), green (primary) or white
 * (the sheet's own create action). Shrinks a little while pressed. */
enum { PILL_OUTLINE, PILL_GREEN, PILL_WHITE };
static lv_obj_t *pill(lv_obj_t *parent, const char *label, int kind, int width, lv_event_cb_t cb, void *user) {
	lv_obj_t *b = lv_obj_create(parent);
	lv_obj_remove_style_all(b);
	lv_obj_set_size(b, width, 50);
	lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_opa(b, kind == PILL_OUTLINE ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(b, kind == PILL_WHITE ? lv_color_white() : lv_color_hex(0x1ed760), 0);
	lv_obj_set_style_border_width(b, kind == PILL_OUTLINE ? 1 : 0, 0);
	lv_obj_set_style_border_color(b, lv_color_hex(0x7c7c7c), 0);
	lv_obj_set_style_transform_scale(b, 245, LV_STATE_PRESSED);
	lv_obj_set_style_transform_pivot_x(b, width / 2, 0);
	lv_obj_set_style_transform_pivot_y(b, 25, 0);
	lv_obj_set_clickable(b, true);
	lv_obj_set_scrollable(b, false);
	lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, user);
	lv_obj_t *l = lv_label_create(b);
	lv_label_set_text(l, label);
	lv_obj_set_style_text_font(l, &font_ui_20_bold, 0);
	lv_obj_set_style_text_color(l, kind == PILL_OUTLINE ? lv_color_white() : lv_color_black(), 0);
	lv_obj_center(l);
	return b;
}
/* Spotify's naming step: a centred question, the name large on an underline
 * with a suggested default, and Cancel / Create pills; the keyboard below
 * finishes with the same action. */
static void name_layer_build(const char *heading, const char *hint, const char *action) {
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_obj_get_height(sheet);
	name_layer = lv_obj_create(sheet);
	lv_obj_remove_style_all(name_layer);
	lv_obj_set_size(name_layer, w, h);
	lv_obj_set_pos(name_layer, 0, 0);
	lv_obj_set_style_bg_color(name_layer, lv_color_hex(0x202124), 0);
	lv_obj_set_style_bg_opa(name_layer, LV_OPA_COVER, 0);
	lv_obj_set_scrollable(name_layer, false);
	lv_obj_set_clickable(name_layer, true);
	lv_obj_move_foreground(name_layer);
	lv_obj_t *l = lv_label_create(name_layer);
	lv_label_set_text(l, heading);
	lv_obj_set_width(l, w - 48);
	lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_font(l, &font_ui_24_bold, 0);
	lv_obj_set_style_text_color(l, lv_color_white(), 0);
	lv_obj_align(l, LV_ALIGN_TOP_MID, 0, 28);
	name_field = lv_textarea_create(name_layer);
	lv_obj_remove_style_all(name_field);
	lv_obj_set_size(name_field, w - 64, 58);
	lv_obj_align(name_field, LV_ALIGN_TOP_MID, 0, 92);
	lv_textarea_set_one_line(name_field, true);
	lv_textarea_set_max_length(name_field, 200);
	lv_textarea_set_placeholder_text(name_field, hint);
	lv_obj_set_style_text_font(name_field, &font_ui_32, 0);
	lv_obj_set_style_text_color(name_field, lv_color_white(), 0);
	lv_obj_set_style_text_align(name_field, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_color(name_field, lv_color_hex(0x8a8a8a), LV_PART_TEXTAREA_PLACEHOLDER);
	lv_obj_set_style_border_side(name_field, LV_BORDER_SIDE_BOTTOM, 0);
	lv_obj_set_style_border_width(name_field, 2, 0);
	lv_obj_set_style_border_color(name_field, lv_color_hex(0x7c7c7c), 0);
	lv_obj_set_style_border_color(name_field, lv_color_white(), LV_STATE_FOCUSED);
	lv_obj_set_style_pad_bottom(name_field, 6, 0);
	keyboard_style_caret(name_field);
	lv_obj_t *row = lv_obj_create(name_layer);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, 132 * 2 + 16, 50);
	lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 176);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 16, 0);
	lv_obj_set_scrollable(row, false);
	pill(row, "Cancel", PILL_OUTLINE, 132, name_close, NULL);
	pill(row, action, PILL_GREEN, 132, name_accept, NULL);
	keyboard_create(name_layer, w, 300, name_field, NULL, action, name_accept, NULL);
}
static void create_cb(lv_event_t *e) {
	bool membership = e && lv_event_get_user_data(e) != NULL;
	name_operation = 0;
	name_returns = membership;
	if (!membership)
		track[0] = 0;
	if (!sheet)
		sheet_new(NULL);
	int count = 0;
	playlists_for_each(count_playlist, &count);
	snprintf(default_name, sizeof(default_name), "My playlist #%d", count + 1);
	name_layer_build("Give your playlist a name", default_name, "Create");
}
void collectionview_create_playlist(lv_event_t *e) { create_cb(e); }
static void name_begin(int operation) {
	sheet_new(NULL);
	name_operation = operation;
	name_returns = false;
	track[0] = 0;
	if (operation == 1) {
		snprintf(default_name, sizeof(default_name), "New folder");
		name_layer_build("Give your folder a name", default_name, "Create");
	} else {
		name_layer_build(operation == 3 ? "Rename folder" : "Rename playlist", "Name", "Save");
		lv_textarea_set_text(name_field, menu_item.title);
	}
}
/* Smart playlists (an R1 addition; the foobar2000 / Rockbox database lists):
 * picked from a few rules, filled from the library and filled again each
 * time they open. The ones that need a word ask for it with the keyboard. */
static const char *const SMART_NAMES[LIBRARY_SMART_RULES] = {"Most played", "Recently added", "Never played",
															   "Lossless files", "Genre is...", "Artist is...", "Year is..."};
static void smart_make(int rule, const char *value) {
	char name[200];
	switch (rule) {
	case LIBRARY_SMART_GENRE: snprintf(name, sizeof(name), "%s songs", value); break;
	case LIBRARY_SMART_ARTIST: snprintf(name, sizeof(name), "Songs by %s", value); break;
	case LIBRARY_SMART_YEAR: snprintf(name, sizeof(name), "From %s", value); break;
	default: snprintf(name, sizeof(name), "%s", SMART_NAMES[rule]); break;
	}
	char base[200];
	snprintf(base, sizeof(base), "%s", name);
	for (int k = 2; playlists_exists(name) && k < 100; k++) snprintf(name, sizeof(name), "%s %d", base, k);
	if (!playlists_create(name)) {
		gui_notify_popup("Could not create the playlist.");
		return;
	}
	library_smart_set(name, rule, value);
	int n = library_smart_refresh(name);
	if (name_layer) {
		lv_obj_delete(name_layer);
		name_layer = NULL;
	}
	bottomsheet_close(false);
	collectionview_refresh();
	char msg[260];
	snprintf(msg, sizeof(msg), n == 1 ? "%s: 1 song" : "%s: %d songs", name, n < 0 ? 0 : n);
	toast_success(msg);
}
static void smart_rule_cb(lv_event_t *e) {
	int rule = (int)(intptr_t)lv_event_get_user_data(e);
	if (rule < LIBRARY_SMART_GENRE) {
		smart_make(rule, "");
		return;
	}
	sheet_new(NULL);
	name_operation = 10 + rule;
	name_returns = false;
	track[0] = 0;
	default_name[0] = 0;
	name_layer_build(rule == LIBRARY_SMART_GENRE ? "Which genre?" : rule == LIBRARY_SMART_ARTIST ? "Which artist?" : "Which year?",
					 "", "Create");
}
static void smart_menu(void) {
	sheet_menu("Smart playlist", LIBRARY_SMART_RULES * MENU_ROW);
	for (int i = 0; i < LIBRARY_SMART_RULES; i++)
		sheet_row(&icon_list_music, SMART_NAMES[i], NULL, false, smart_rule_cb, (void *)(intptr_t)i);
}

static void create_kind(lv_event_t *e) {
	int k = (int)(intptr_t)lv_event_get_user_data(e);
	if (k == 2)
		smart_menu();
	else if (k)
		name_begin(1);
	else {
		sheet_new(NULL);
		create_cb(NULL);
	}
}
/* Spotify's Create sheet: each kind in a circle with a line saying what it
 * is for. A folder is the R1's own way of grouping playlists. */
void collectionview_create_menu(lv_event_t *e) {
	(void)e;
	sheet_menu(NULL, 3 * MENU_ROW_TALL);
	sheet_row(&icon_list_plus, "Playlist", "Build a playlist with songs", false, create_kind, NULL);
	sheet_row(&icon_list_music, "Smart playlist", "Fills itself: most played, a genre, a year", false, create_kind, (void *)2);
	sheet_row(&icon_folder_new, "Folder", "Group your playlists together", false, create_kind, (void *)1);
}
static collection_item_t folders[24];
static int folder_page;
static void move_pick(lv_event_t *e) {
	const char *name = lv_event_get_user_data(e);
	if (!collections_move_playlist(menu_item.key, name))
		gui_notify_popup("Could not move playlist.");
	bottomsheet_close(false);
	collectionview_refresh();
}
static void move_draw(void);
static void move_page(lv_event_t *e) {
	folder_page += (int)(intptr_t)lv_event_get_user_data(e);
	move_draw();
}
static void move_draw(void) {
	int n = collections_list(COLLECTION_FOLDER, COLLECTION_NAME, folder_page * 24, folders, 24);
	int rows = 1 + n + (folder_page ? 1 : 0) + (n == 24 ? 1 : 0);
	sheet_menu("Move to folder", rows * MENU_ROW);
	sheet_row(&icon_folder_root, "Your Library (no folder)", NULL, false, move_pick, "");
	for (int i = 0; i < n; i++)
		sheet_row(&icon_folder, folders[i].title, NULL, false, move_pick, folders[i].key);
	if (folder_page)
		sheet_row(&icon_chevron_up, "Previous folders", NULL, false, move_page, (void *)-1);
	if (n == 24)
		sheet_row(&icon_chevron_down, "More folders", NULL, false, move_page, (void *)1);
}
static void delete_confirm(void *user) {
	(void)user;
	bool ok = menu_item.kind == COLLECTION_FOLDER || playlists_delete(menu_item.key);
	if (ok) {
		collections_forget(menu_item.kind, menu_item.key);
		if (menu_item.kind == COLLECTION_PLAYLIST)
			medialist_playlist_deleted(menu_item.key);
		if (!strcmp(folder, menu_item.key))
			folder[0] = 0;
		toast_plain(menu_item.kind == COLLECTION_FOLDER ? "Folder deleted" : "Playlist deleted");
	} else
		gui_notify_popup("Could not delete playlist.");
	collectionview_refresh();
}
static bool enqueue_path(const char *name, const char *path, const char *artist, void *u) {
	(void)name;
	(void)artist;
	snprintf(u, 512, "%s", path);
	return false;
}
static void playlist_do(int action);
static void playlist_action(lv_event_t *e) { playlist_do((int)(intptr_t)lv_event_get_user_data(e)); }

/* The playlist page's pills (Add, Edit, Name & details) run the same actions
 * as its options sheet. */
void collectionview_playlist_command(const char *name, int action) {
	if (!name || !name[0]) return;
	memset(&menu_item, 0, sizeof(menu_item));
	menu_item.kind = COLLECTION_PLAYLIST;
	snprintf(menu_item.key, sizeof(menu_item.key), "%s", name);
	snprintf(menu_item.title, sizeof(menu_item.title), "%s", name);

	playlist_do(action);
}

/* Spotify's playlist cover, chosen here from the albums in the playlist (its
 * own picture is not something a card player has); "First song" goes back
 * to the default. */
static char cover_titles[12][128], cover_paths[12][512];
static void cover_pick_cb(lv_event_t *e) {
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	library_playlist_cover_set(menu_item.key, i >= 0 ? cover_paths[i] : NULL);
	bottomsheet_close(false);
	toast_success(i >= 0 ? "Cover changed" : "Cover reset");
	collectionview_refresh();
}
static void cover_choose(void) {
	int n = library_playlist_albums(menu_item.key, cover_titles, cover_paths, 12);
	char current[512] = "";
	library_playlist_cover_get(menu_item.key, current, sizeof(current));
	sheet_menu("Choose cover", (n + 1) * MENU_ROW);
	sheet_row(&icon_files_playlist, "First song", NULL, !current[0], cover_pick_cb, (void *)(intptr_t)-1);
	for (int i = 0; i < n; i++)
		sheet_row(&icon_album, cover_titles[i], NULL, !strcmp(current, cover_paths[i]), cover_pick_cb, (void *)(intptr_t)i);
}

static void playlist_do(int action) {
	if (action == 8) {
		cover_choose();
		return;
	}
	if (action == 7) {
		collections_custom_first(menu_item.kind, menu_item.key);
		sort_mode = COLLECTION_CUSTOM;
		config_set_int("collections", "sort", sort_mode);
		config_save();
		bottomsheet_close(false);
		collectionview_refresh();
		return;
	}
	if (action == 5) {
		name_begin(menu_item.kind == COLLECTION_FOLDER ? 3 : 2);
		return;
	}
	if (action == 4) {
		folder_page = 0;
		move_draw();
		return;
	}
	if (action == 6) {
		/* The shared confirmation, as Spotify asks before deleting. */
		bool folder_kind = menu_item.kind == COLLECTION_FOLDER;
		static char message[400];
		snprintf(message, sizeof(message),
				 folder_kind ? "\"%.200s\" will be deleted. Its playlists return to Your Library."
							 : "\"%.200s\" will be deleted. Your audio files stay on the card.",
				 menu_item.title);
		bottomsheet_close(false);
		confirm_show(folder_kind ? "Delete folder?" : "Delete playlist?", message, "Delete", delete_confirm, NULL);
		return;
	}
	char name[512];
	snprintf(name, sizeof(name), "%s", menu_item.key);
	if (action != 1) bottomsheet_close(false);
	if (action == 1 || action == 2) {
		playlistview_open(name, action == 2);
		if (action == 1)
			playlistview_add_songs();
	}
	if (action == 3) {
		library_index_t *ix = library_index_open(LIBRARY_LIST_PLAYLIST, LIBRARY_FILTER_NONE, name, LIBRARY_ORDER_DEFAULT, false);
		int n = 0;
		if (ix) {
			for (int i = 0; i < library_index_count(ix) && i < 64; i++) {
				char path[512] = "";
				library_index_window(ix, i, 1, enqueue_path, path);
				if (!playlist_priority_add(path))
					break;
				n++;
			}
			library_index_close(ix);
		}
		device_state_queue_changed();
		char message[64];
		snprintf(message, sizeof(message), "Added %d songs to queue", n);
		gui_notify_popup(message);
	}
}
static lv_obj_t *library_search_field, *library_search_results;
static keyboard_t *library_keyboard;
static collection_item_t search_rows[24];
static void search_liked(lv_event_t *e) {
	(void)e;
	bottomsheet_close(false);
	medialist_open("Liked Songs", LIBRARY_LIST_FAVOURITES, LIBRARY_FILTER_NONE, NULL);
}
static void search_draw(lv_event_t *e) {
	(void)e;
	snprintf(library_query, sizeof(library_query), "%s", lv_textarea_get_text(library_search_field));
	member_art_drop();
	lv_obj_clean(library_search_results);
	lv_obj_update_layout(library_search_results);
	char folded[512];
	library_search_fold(library_query, folded, sizeof(folded));
	bool liked = !folder[0] && (filter == 0 || filter == 1) && (!folded[0] || strstr("liked songs", folded));
	if (liked)
		button(library_search_results, "Liked Songs", lv_obj_get_width(library_search_results), 54, search_liked, NULL);
	int n = collections_browse(filter, sort_mode, library_query, folder, 0, search_rows, 24);
	for (int i = 0; i < n; i++) {
		collection_item_t *item = &search_rows[i];
		int w = lv_obj_get_width(library_search_results);
		lv_obj_t *row = button(library_search_results, NULL, w, 74, open_row, item);
		lv_obj_t *im = lv_image_create(row);
		lv_obj_set_size(im, 48, 48);
		lv_obj_set_pos(im, 4, 12);
		lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CENTER);
		lv_image_set_src(im, item->kind == COLLECTION_ARTIST ? &icon_artist : item->kind == COLLECTION_FOLDER ? &icon_folder : &icon_album);
		if (member_art_count < 8 && (item->kind == COLLECTION_ALBUM || item->kind == COLLECTION_PLAYLIST)) {
			char path[512] = "";
			library_for_each(item->kind == COLLECTION_ALBUM ? LIBRARY_LIST_TRACKS : LIBRARY_LIST_PLAYLIST, item->kind == COLLECTION_ALBUM ? LIBRARY_FILTER_ALBUM : LIBRARY_FILTER_NONE, item->key, sample_track, path);
			if (path[0]) {
				int slot = member_art_count++;
				member_art[slot] = im;
				coverloader_request(COVERLOADER_MEMBERS_BASE + slot, path, 48);
			}
		}
		lv_obj_t *l = text(row, item->title, w - 76, &font_ui_20, false);
		lv_obj_set_pos(l, 66, 9);
		const char *kind = item->kind == COLLECTION_ARTIST ? "Artist" : item->kind == COLLECTION_ALBUM ? "Album" : item->kind == COLLECTION_FOLDER ? "Playlist folder" : "Playlist";
		char subtitle[300];
		snprintf(subtitle, sizeof(subtitle), "%s%s%s", kind, item->creator[0] ? " \xC2\xB7 " : "", item->creator);
		l = text(row, subtitle, w - 76, &font_ui_16, true);
		lv_obj_set_pos(l, 66, 40);
	}
	if (!member_timer)
		member_timer = lv_timer_create(member_art_tick, 120, NULL);
	if (!n && !liked)
		text(library_search_results, "No matching collections", 400, &font_ui_18, true);
}
static void search_done(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(library_keyboard, false);
	lv_obj_set_height(library_search_results, lv_obj_get_height(sheet_body) - 72);
	collectionview_refresh();
}
static void search_focus(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(library_keyboard, true);
	lv_obj_set_height(library_search_results, lv_obj_get_height(sheet_body) - 342);
}
void collectionview_search(lv_event_t *e) {
	(void)e;
	sheet_new("Search Your Library");
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	library_search_field = lv_textarea_create(sheet_body);
	lv_obj_add_style(library_search_field, &theme_style_card, 0);
	lv_obj_add_style(library_search_field, &theme_style_text, 0);
	lv_obj_set_size(library_search_field, w, 56);
	lv_textarea_set_one_line(library_search_field, true);
	lv_textarea_set_max_length(library_search_field, 255);
	lv_textarea_set_placeholder_text(library_search_field, "Find your music");
	lv_textarea_set_text(library_search_field, library_query);
	lv_obj_set_style_text_font(library_search_field, &font_ui_20, 0);
	library_search_results = panel(sheet_body, w, lv_obj_get_height(sheet_body) - 342);
	lv_obj_set_flex_flow(library_search_results, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_scrollable(library_search_results, true);
	library_keyboard = keyboard_create(sheet, w + 32, 270, library_search_field, NULL, "Done", search_done, NULL);
	lv_obj_add_event_cb(library_search_field, search_draw, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(library_search_field, search_focus, LV_EVENT_CLICKED, NULL);
	search_draw(NULL);
}

static bool member_collect(const char *name, void *user) {
	(void)user;
	char folded[512], query[512];
	library_search_fold(name, folded, sizeof(folded));
	library_search_fold(member_query, query, sizeof(query));
	if (query[0] && !strstr(folded, query))
		return true;
	if (member_seen++ < member_page * 24)
		return true;
	if (member_count == 24)
		return false;
	snprintf(member_names[member_count++], 201, "%s", name);
	return true;
}
static void member_toggle(lv_event_t *e) {
	const char *name = lv_event_get_user_data(e);
	bool exists = playlists_has_track(name, track), ok = true;
	if (exists)
		ok = library_playlist_remove_all_path(name, track);
	else
		ok = playlists_add_track(name, track);
	if (!ok)
		gui_notify_popup("Could not update playlist membership.");
	members_draw();
}
static void liked_cb(lv_event_t *e) {
	(void)e;
	library_fav_toggle(track, track_title, track_artist);
	members_draw();
}
static void member_page_cb(lv_event_t *e) {
	member_page += (int)(intptr_t)lv_event_get_user_data(e);
	if (member_page < 0)
		member_page = 0;
	members_draw();
}
static lv_obj_t *member_row(const char *name, bool selected, lv_event_cb_t cb, void *u) {
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	lv_obj_t *row = button(member_list, NULL, w, 74, cb, u);
	lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0); // membership is the check, not a tinted row
	lv_obj_t *art = lv_image_create(row);
	lv_image_set_src(art, !strcmp(name, "Liked Songs") ? &icon_heart : &icon_files_playlist);
	lv_obj_set_size(art, 48, 48);
	lv_image_set_inner_align(art, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_set_pos(art, 4, 12);
	if (!strcmp(name, "Liked Songs")) {
		lv_obj_set_style_bg_color(art, lv_color_hex(0x6432e8), 0);
		lv_obj_set_style_bg_grad_color(art, lv_color_hex(0xa8dbca), 0);
		lv_obj_set_style_bg_grad_dir(art, LV_GRAD_DIR_VER, 0);
		lv_obj_set_style_bg_opa(art, 255, 0);
		lv_obj_set_style_radius(art, 5, 0);
	} else if (member_art_count < 8) {
		library_playlist_row_t first;
		if (library_playlist_page(name, 0, 1, &first) == 1) {
			int slot = member_art_count++;
			member_art[slot] = art;
			coverloader_request(COVERLOADER_MEMBERS_BASE + slot, first.path, 48);
		}
	}
	lv_obj_t *l = text(row, name, w - 122, &font_ui_22, false);
	lv_obj_set_pos(l, 68, 12);
	l = text(row, "Playlist", w - 122, &font_ui_16, true);
	lv_obj_set_pos(l, 68, 42);
	lv_obj_t *check = lv_image_create(row);
	lv_image_set_src(check, selected ? &icon_circle_check : &icon_plus);
	lv_obj_set_size(check, 30, 30);
	lv_image_set_inner_align(check, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_align(check, LV_ALIGN_RIGHT_MID, -4, 0);
	lv_obj_set_style_image_recolor(check, lv_color_hex(selected ? 0x1ed760 : 0xb3b3b3), 0);
	lv_obj_set_style_image_recolor_opa(check, 255, 0);
	return row;
}
/* A destination for the chosen tracks: they go in on a worker (tags are read
 * for each), the snackbar says when, and the sheet closes. */
static void bulk_pick(lv_event_t *e) {
	const char *name = lv_event_get_user_data(e);
	if (!bulk_count)
		return;
	if (!name)
		playlistpage_add_favourites((const char *const *)bulk_paths, bulk_count);
	else if (!playlistpage_add_tracks_to(name, (const char *const *)bulk_paths, bulk_count))
		gui_notify_popup("playlist_cannot_add_the_tracks");
	bottomsheet_close(true);
}
static void members_draw(void) {
	if (!member_list)
		return;
	member_art_drop();
	lv_obj_clean(member_list);
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	char query[512];
	library_search_fold(member_query, query, sizeof(query));
	bool liked_visible = !query[0] || strstr("liked songs", query), liked = library_fav_contains(track);
	member_count = member_seen = 0;
	playlists_for_each(member_collect, NULL);
	if (bulk_count) {
		if (liked_visible)
			member_row("Liked Songs", false, bulk_pick, NULL);
		for (int i = 0; i < member_count; i++)
			member_row(member_names[i], false, bulk_pick, member_names[i]);
		goto drawn;
	}
	for (int group = 1; group >= 0; group--) {
		bool heading = false;
		if (liked_visible && liked == group) {
			text(member_list, group ? "Saved in" : "Playlists", w, &font_ui_18, true);
			heading = true;
			member_row("Liked Songs", liked, liked_cb, NULL);
		}
		for (int i = 0; i < member_count; i++)
			if (playlists_has_track(member_names[i], track) == group) {
				if (!heading) {
					text(member_list, group ? "Saved in" : "Playlists", w, &font_ui_18, true);
					heading = true;
				}
				member_row(member_names[i], group, member_toggle, member_names[i]);
			}
	}
drawn:
	if (!member_timer)
		member_timer = lv_timer_create(member_art_tick, 120, NULL);
	if (!member_count && !liked_visible && member_query[0])
		text(member_list, "No matching playlists", w, &font_ui_18, true);
	if (member_page)
		button(member_list, "Previous playlists", w, 48, member_page_cb, (void *)(intptr_t)-1);
	if (member_count == 24)
		button(member_list, "More playlists", w, 48, member_page_cb, (void *)(intptr_t)1);
	player_refresh_membership();
}
static void member_search_changed(lv_event_t *e) {
	(void)e;
	snprintf(member_query, sizeof(member_query), "%s", lv_textarea_get_text(member_search));
	member_page = 0;
	members_draw();
}
static void member_search_done(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(member_keyboard, false);
	lv_obj_set_height(member_list, lv_obj_get_height(sheet_body) - 120 - (bulk_count ? 34 : 74));
}
static void member_search_focus(lv_event_t *e) {
	(void)e;
	if (!member_keyboard)
		member_keyboard = keyboard_create(sheet, lv_obj_get_width(sheet), 270, member_search, NULL, "Done", member_search_done, NULL);
	keyboard_set_visible(member_keyboard, true);
	lv_obj_set_height(member_list, LV_MAX(80, lv_obj_get_height(sheet_body) - 394));
}
typedef struct {
	const char *path;
	bool found;
} membership_t;
static bool any_member(const char *name, void *u) {
	membership_t *m = u;
	m->found = playlists_has_track(name, m->path);
	return !m->found;
}
bool collectionview_track_saved(const char *path) {
	if (library_fav_contains(path))
		return true;
	membership_t m = {path, false};
	playlists_for_each(any_member, &m);
	return m.found;
}
static void members_sheet(const char *const *paths, int count);
void collectionview_manage_track(const char *path) {
	if (!path || !*path || !gui_card_available())
		return;
	snprintf(track, sizeof(track), "%s", path);
	track_title[0] = track_artist[0] = 0;
	playlists_track_names(path, track_title, sizeof(track_title), track_artist, sizeof(track_artist));
	members_sheet(NULL, 0);
}
void collectionview_add_tracks(const char *const *paths, int count) {
	if (!paths || count <= 0 || !gui_card_available())
		return;
	track[0] = track_title[0] = track_artist[0] = 0;
	members_sheet(paths, count);
}
/* The membership sheet for one track, or the destination sheet for several.
 * The set is copied after the sheet opens: opening runs the previous sheet's
 * cleanup, which lets go of any earlier set. */
static void members_sheet(const char *const *paths, int count) {
	member_page = 0;
	collections_init(playlist_card_root());
	sheet_new("Add to playlist");
	if (count > 0 && (bulk_paths = calloc((size_t)count, sizeof(*bulk_paths)))) {
		for (int i = 0; i < count && (bulk_paths[bulk_count] = strdup(paths[i] ? paths[i] : "")); i++)
			bulk_count++;
	}
	member_query[0] = 0;
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	if (bulk_count) {
		char line[48];
		snprintf(line, sizeof(line), bulk_count == 1 ? "%d song" : "%d songs", bulk_count);
		text(sheet_body, line, w, &font_ui_18, true);
	}
	/* Spotify's sheet: a white New playlist pill, a rounded find field, the
	 * playlists, and a green Done pill at the foot (one track's memberships). */
	lv_obj_t *create_row = lv_obj_create(sheet_body);
	lv_obj_remove_style_all(create_row);
	lv_obj_set_size(create_row, w, 50);
	lv_obj_set_scrollable(create_row, false);
	lv_obj_t *create = pill(create_row, "New playlist", PILL_WHITE, 190, create_cb, (void *)1);
	lv_obj_center(create);
	member_search = lv_textarea_create(sheet_body);
	lv_obj_remove_style_all(member_search);
	lv_obj_set_size(member_search, w, 48);
	lv_textarea_set_one_line(member_search, true);
	lv_textarea_set_placeholder_text(member_search, "Find playlist");
	lv_obj_set_style_text_font(member_search, &font_ui_20, 0);
	lv_obj_set_style_text_color(member_search, lv_color_white(), 0);
	lv_obj_set_style_text_color(member_search, lv_color_hex(0x8a8a8a), LV_PART_TEXTAREA_PLACEHOLDER);
	lv_obj_set_style_bg_color(member_search, lv_color_hex(0x333333), 0);
	lv_obj_set_style_bg_opa(member_search, LV_OPA_COVER, 0);
	lv_obj_set_style_radius(member_search, 6, 0);
	lv_obj_set_style_pad_hor(member_search, 14, 0);
	lv_obj_set_style_pad_ver(member_search, 11, 0);
	keyboard_style_caret(member_search);
	lv_textarea_set_max_length(member_search, 200);
	lv_obj_add_event_cb(member_search, member_search_changed, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(member_search, member_search_focus, LV_EVENT_CLICKED, NULL);
	int foot = bulk_count ? 0 : 74;
	member_list = panel(sheet_body, w, lv_obj_get_height(sheet_body) - 120 - foot - (bulk_count ? 34 : 0));
	if (!bulk_count) {
		lv_obj_t *done = pill(sheet, "Done", PILL_GREEN, 132, close_sheet, NULL);
		lv_obj_add_flag(done, LV_OBJ_FLAG_IGNORE_LAYOUT);
		lv_obj_align(done, LV_ALIGN_BOTTOM_MID, 0, -14);
	}
	lv_obj_set_style_bg_opa(member_list, 0, 0);
	lv_obj_set_scrollable(member_list, true);
	lv_obj_set_scroll_dir(member_list, LV_DIR_VER);
	lv_obj_set_flex_flow(member_list, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(member_list, 4, 0);
	members_draw();
}
static lv_obj_t *collection_shortcut(const char *title, const char *subtitle, const lv_image_dsc_t *icon, uint32_t color, int action, int width) {
	int rw = grid ? (width - 10) / 2 : width, rh = grid ? rw + 76 : 76;
	lv_obj_t *row = button(list, NULL, rw, rh, route, (void *)(intptr_t)action);
	lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
	lv_obj_t *art = panel(row, grid ? rw - 12 : 64, grid ? rw - 12 : 64);
	/* Decorative artwork must not swallow the collection row's tap. */
	lv_obj_set_clickable(art, false);
	lv_obj_set_pos(art, grid ? 6 : 0, 6);
	lv_obj_set_style_bg_color(art, lv_color_hex(color), 0);
	lv_obj_set_style_bg_grad_color(art, lv_color_lighten(lv_color_hex(color), LV_OPA_50), 0);
	lv_obj_set_style_bg_grad_dir(art, LV_GRAD_DIR_VER, 0);
	lv_obj_t *im = lv_image_create(art);
	lv_obj_set_clickable(im, false);
	lv_image_set_src(im, icon);
	lv_obj_set_size(im, 30, 30);
	lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_center(im);
	lv_obj_set_style_image_recolor(im, lv_color_white(), 0);
	lv_obj_set_style_image_recolor_opa(im, LV_OPA_COVER, 0);
	lv_obj_t *l = text(row, title, grid ? rw - 16 : width - 98, &font_ui_22, false);
	lv_obj_set_pos(l, grid ? 8 : 82, grid ? rw + 4 : 12);
	l = text(row, subtitle, grid ? rw - 16 : width - 98, &font_ui_16, true);
	lv_obj_set_pos(l, grid ? 8 : 82, grid ? rw + 34 : 43);
	return row;
}
static void clear_query(lv_event_t *e) {
	(void)e;
	library_query[0] = 0;
	page = 0;
	collectionview_refresh();
}
/* One row of Your Library, for the item in slot i, appended to the list. */
static void lib_row_build(int i, int w) {
	collection_item_t *r = &rows[i];
	if (r->kind == COLLECTION_PLAYLIST && !playlists_exists(r->key))
		return;
	if (!genre_match(r))
		return;
	int rw = grid ? (w - 10) / 2 : w, rh = grid ? rw + 76 : 80;
	lv_obj_t *row = button(list, NULL, rw, rh, open_row, r);
	row_boxes[i] = row;
	lv_obj_add_event_cb(row, row_options, LV_EVENT_LONG_PRESSED, r);
	lv_obj_add_event_cb(row, row_swipe, LV_EVENT_PRESSED, r);
	lv_obj_add_event_cb(row, row_swipe, LV_EVENT_RELEASED, r);
	lv_obj_add_event_cb(row, row_swipe, LV_EVENT_PRESSING, r);
	lv_obj_add_event_cb(row, row_swipe, LV_EVENT_PRESS_LOST, r);
	lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
	row_images[i] = lv_image_create(row);
	lv_image_set_src(row_images[i], r->kind == COLLECTION_ARTIST ? &icon_artist : r->kind == COLLECTION_FOLDER ? &icon_folder : r->kind == COLLECTION_PLAYLIST ? &icon_files_playlist : &icon_album);
	lv_obj_set_size(row_images[i], grid ? rw - 12 : 64, grid ? rw - 12 : 64);
	lv_image_set_inner_align(row_images[i], LV_IMAGE_ALIGN_CENTER);
	lv_obj_set_pos(row_images[i], grid ? 6 : 0, 7);
	lv_obj_set_style_bg_color(row_images[i], lv_color_hex(0x252525), 0);
	lv_obj_set_style_bg_opa(row_images[i], 255, 0);
	if (r->kind == COLLECTION_ARTIST) {
		lv_obj_set_style_bg_color(row_images[i], lv_color_hex(0x383838), 0);
		lv_obj_set_style_bg_opa(row_images[i], LV_OPA_COVER, 0);
		lv_obj_set_style_radius(row_images[i], LV_RADIUS_CIRCLE, 0);
	}
	char title[280];
	snprintf(title, sizeof(title), "%s", r->title);
	lv_obj_t *l = text(row, title, grid ? rw - 24 : rw - 92, &font_ui_22, false);
	lv_obj_set_pos(l, grid ? 12 : 82, grid ? rw + 4 : 12);
	const char *kind = r->kind == COLLECTION_PLAYLIST ? "Playlist" : r->kind == COLLECTION_ALBUM ? "Album" : r->kind == COLLECTION_ARTIST ? "Artist" : "Playlist folder";
	char sub[300];
	if (r->kind == COLLECTION_FOLDER) {
		int children = collections_folder_count(r->key);
		snprintf(sub, sizeof(sub), "%d playlist%s", children, children == 1 ? "" : "s");
	} else if (r->kind == COLLECTION_PLAYLIST)
		{
		int songs = playlists_count_tracks(r->key);
		snprintf(sub, sizeof(sub), "Playlist \xC2\xB7 %d song%s", songs, songs == 1 ? "" : "s");
	}
	else if (r->kind == COLLECTION_ALBUM && r->year)
		snprintf(sub, sizeof(sub), "Album \xC2\xB7 %s", r->creator);
	else
		snprintf(sub, sizeof(sub), "%s%s%s", kind, r->creator[0] ? " \xC2\xB7 " : "", r->creator);
	l = text(row, sub, grid ? rw - 24 : rw - 98, &font_ui_16, true);
	lv_obj_set_pos(l, grid ? 12 : 82, grid ? rw + 34 : 43);
	if (r->pin) {
		lv_obj_set_x(l, grid ? 34 : 104);
		lv_obj_set_width(l, grid ? rw - 48 : rw - 120);
		lv_obj_t *pin = lv_image_create(row);
		lv_image_set_src(pin, &sonix_icon_pin);
		lv_obj_set_size(pin, 16, 16);
		lv_image_set_inner_align(pin, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_pos(pin, grid ? 12 : 82, grid ? rw + 35 : 44);
		lv_obj_set_style_image_recolor(pin, lv_color_hex(0x1ed760), 0);
		lv_obj_set_style_image_recolor_opa(pin, LV_OPA_COVER, 0);
	}
	/* No per-row "...": as in Spotify's library a long press opens the
	 * options, and a right swipe pins. */
}

// Drops the rows of items [first_item, first_item + count) and their artwork.
static void lib_rows_drop(int first_item, int count) {
	for (int a = first_item; a < first_item + count; a++) {
		int slot = a % LIB_SLOTS;
		for (int k = 0; k < 8; k++) {
			if (art[k].row == slot) {
				coverloader_release(COVERLOADER_COLLECTION_BASE + k);
				if (art[k].loaded) {
					cover_free(&art[k].image);
				}
				art[k].loaded = false;
				art[k].row = -1;
			}
		}
		if (row_boxes[slot]) {
			lv_obj_delete(row_boxes[slot]);
		}
		row_boxes[slot] = NULL;
		row_images[slot] = NULL;
	}
}

// The first row on screen of items [first_item, first_item + count), to keep
// the list where it is while rows come and go above it.
static lv_obj_t *lib_anchor(int first_item, int count) {
	for (int a = first_item; a < first_item + count; a++) {
		if (row_boxes[a % LIB_SLOTS]) {
			return row_boxes[a % LIB_SLOTS];
		}
	}
	return NULL;
}

// "All songs" closes the list, once the end of it is loaded.
static void lib_footer_sync(int w) {
	bool want = lib_end && filter == 0 && !folder[0] && !library_query[0] && !genre_filter[0];
	if (want && !lib_footer) {
		lib_footer = collection_shortcut("All songs", "Every indexed recording", &icon_music_note, 0x303030, 2, w);
	} else if (!want && lib_footer) {
		lv_obj_delete(lib_footer);
		lib_footer = NULL;
	}
}

// Reads the next page (or the one before the window) and builds its rows, then
// lets the page at the other end go once the window is over LIB_ROWS. A row
// that stays is held where it was on screen: whatever LVGL did to the scroll
// position as the content changed, the list does not move under the reader.
static void lib_load_page(bool before) {
	int w = lib_width;
	int start = before ? lib_first - LIB_PAGE : lib_first + lib_loaded;
	if (start < 0 || (!before && lib_end)) {
		return;
	}
	static collection_item_t page_rows[LIB_PAGE];
	int n = collections_browse(filter, (collection_sort_t)sort_mode, library_query, folder, start, page_rows, LIB_PAGE);
	if (!before && n < LIB_PAGE) {
		lib_end = true;
	}
	if (n == 0) {
		lib_footer_sync(w);
		return;
	}
	// The row to hold still: the first of the window going up, and going down
	// the first one past the page that may go.
	lv_obj_t *anchor = before ? lib_anchor(lib_first, lib_loaded)
							  : lib_anchor(lib_first + LIB_PAGE, lib_loaded > LIB_PAGE ? lib_loaded - LIB_PAGE : 0);
	lv_area_t was = {0};
	if (anchor) {
		lv_obj_update_layout(list);
		lv_obj_get_coords(anchor, &was);
	}
	for (int i = 0; i < n; i++) {
		int slot = (start + i) % LIB_SLOTS;
		rows[slot] = page_rows[i];
		lib_row_build(slot, w);
		if (before && row_boxes[slot]) {
			lv_obj_move_to_index(row_boxes[slot], lib_head + i);
		}
	}
	if (before) {
		lib_first = start;
	}
	lib_loaded += n;
	if (lib_loaded > LIB_ROWS) {
		if (before) {
			// The last page goes; it is read again on the way down.
			int last = lib_loaded % LIB_PAGE ? lib_loaded % LIB_PAGE : LIB_PAGE;
			lib_rows_drop(lib_first + lib_loaded - last, last);
			lib_loaded -= last;
			lib_end = false;
		} else {
			lib_rows_drop(lib_first, LIB_PAGE);
			lib_first += LIB_PAGE;
			lib_loaded -= LIB_PAGE;
		}
	}
	if (lib_footer) {
		lv_obj_move_foreground(lib_footer);
	}
	lib_footer_sync(w);
	if (anchor) {
		lv_obj_update_layout(list);
		lv_area_t now;
		lv_obj_get_coords(anchor, &now);
		if (now.y1 != was.y1) {
			lv_obj_scroll_by(list, 0, was.y1 - now.y1, LV_ANIM_OFF);
		}
	}
}

// A screen from the end, the next page; half a screen from the top of a
// window that has let pages go, the one before it.
static void lib_window_check(void) {
	if (lib_busy || !list || !lv_obj_is_visible(list)) {
		return;
	}
	lib_busy = true;
	int32_t h = lv_obj_get_height(list);
	if (!lib_end && lv_obj_get_scroll_bottom(list) < h) {
		lib_load_page(false);
	} else if (lib_first > 0 && lv_obj_get_scroll_top(list) < h / 2) {
		lib_load_page(true);
	}
	lib_busy = false;
}

/* The fast-scroll handle, Spotify's on a long Your Library: a grip on the
 * right edge that shows while the list moves. Dragged, it says which record
 * it is over (the title in a bubble beside it) and, let go, the window is read
 * again from there -- a jump of hundreds of records without paging through
 * them (device round 4: the list felt endless on a small screen). */
#define SCRUB_MIN_ITEMS 40
#define SCRUB_W 28
#define SCRUB_H 56
#define SCRUB_SHOW_MS 1600
static lv_obj_t *scrub, *scrub_bubble, *scrub_label;
static lv_timer_t *scrub_timer;
static int scrub_total, scrub_top, scrub_range, scrub_target = -1;
static bool scrub_dragging;
static uint32_t scrub_seen_ms, scrub_label_ms;

// The record at the top of the screen, as an index into the whole list.
static int lib_top_index(void) {
	int32_t top = lv_obj_get_scroll_y(list);
	for (int a = lib_first; a < lib_first + lib_loaded; a++) {
		lv_obj_t *row = row_boxes[a % LIB_SLOTS];
		if (row && lv_obj_get_y(row) + lv_obj_get_height(row) > top) {
			return a;
		}
	}
	return lib_first + lib_loaded;
}

static void scrub_place(int index) {
	if (!scrub || scrub_total < 2) {
		return;
	}
	if (index < 0) {
		index = 0;
	}
	if (index > scrub_total - 1) {
		index = scrub_total - 1;
	}
	lv_obj_set_y(scrub, scrub_top + (int)((int64_t)index * scrub_range / (scrub_total - 1)));
}

static void scrub_show(void) {
	if (!scrub) {
		return;
	}
	scrub_seen_ms = lv_tick_get();
	lv_obj_set_hidden(scrub, false);
}

static void scrub_timer_cb(lv_timer_t *t) {
	(void)t;
	if (scrub && !scrub_dragging && !lv_obj_is_hidden(scrub) && lv_tick_elaps(scrub_seen_ms) > SCRUB_SHOW_MS) {
		lv_obj_set_hidden(scrub, true);
	}
}

// Reads the window again with `index` in its first page, and puts that record
// at the top of the screen.
static void lib_jump(int index) {
	if (index < 0) {
		index = 0;
	}
	if (index < lib_first || index >= lib_first + lib_loaded) {
		lib_busy = true;
		lib_rows_drop(lib_first, lib_loaded);
		if (lib_footer) {
			lv_obj_delete(lib_footer);
			lib_footer = NULL;
		}
		lib_first = index / LIB_PAGE * LIB_PAGE;
		lib_loaded = 0;
		lib_end = false;
		lib_load_page(false);
		if (!lib_end) {
			lib_load_page(false);
		}
		lib_busy = false;
	}
	lv_obj_update_layout(list);
	lv_obj_t *row = NULL;
	for (int a = index; a < lib_first + lib_loaded && !row; a++) {
		row = row_boxes[a % LIB_SLOTS];
	}
	lv_obj_scroll_to_y(list, row ? lv_obj_get_y(row) : 0, LV_ANIM_OFF);
}

static void scrub_cb(lv_event_t *e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_indev_t *indev = lv_indev_active();
	if (!indev || !scrub) {
		return;
	}
	lv_point_t p;
	lv_indev_get_point(indev, &p);
	lv_area_t root_area;
	lv_obj_get_coords(root, &root_area);
	int y = p.y - root_area.y1 - SCRUB_H / 2;
	if (code == LV_EVENT_PRESSED) {
		scrub_dragging = true;
		scrub_label_ms = 0;
		lv_anim_delete(list, NULL); // a flick in flight stops under the finger
		lv_obj_set_hidden(scrub_bubble, false);
	}
	if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
		y = y < scrub_top ? scrub_top : y > scrub_top + scrub_range ? scrub_top + scrub_range : y;
		scrub_target = scrub_range > 0 ? (int)((int64_t)(y - scrub_top) * (scrub_total - 1) / scrub_range) : 0;
		lv_obj_set_y(scrub, y);
		// The title under the grip, read at most every 80 ms: one row of the
		// collections index each time.
		if (code == LV_EVENT_PRESSED || lv_tick_elaps(scrub_label_ms) > 80) {
			scrub_label_ms = lv_tick_get();
			collection_item_t item;
			if (collections_browse(filter, (collection_sort_t)sort_mode, library_query, folder, scrub_target, &item, 1) == 1) {
				char text[300];
				snprintf(text, sizeof(text), "%s", item.title);
				if (sort_mode == COLLECTION_NAME && text[0]) {
					/* Use the same folded initial as the ordered list. Upper-
					 * and lowercase names belong to one alphabet, including
					 * accented Latin letters and ignored leading articles. */
					snprintf(text, sizeof(text), "%c", library_index_letter(item.title));
				}
				lv_label_set_text(scrub_label, text);
			}
		}
		lv_obj_update_layout(scrub_bubble);
		lv_obj_set_y(scrub_bubble, y + (SCRUB_H - lv_obj_get_height(scrub_bubble)) / 2);
		scrub_seen_ms = lv_tick_get();
		return;
	}
	if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
		scrub_dragging = false;
		lv_obj_set_hidden(scrub_bubble, true);
		scrub_seen_ms = lv_tick_get();
		if (scrub_target >= 0) {
			printf("library: fast scroll to %d of %d (%s)\n", scrub_target, scrub_total,
				   code == LV_EVENT_RELEASED ? "released" : "press lost");
			lib_jump(scrub_target);
		}
		scrub_target = -1;
	}
}

static void scrub_build(int w) {
	scrub = scrub_bubble = scrub_label = NULL;
	scrub_dragging = false;
	scrub_target = -1;
	scrub_total = collections_browse_count(filter, library_query, folder);
	if (scrub_total < SCRUB_MIN_ITEMS) {
		return;
	}
	/* New filter/list geometry is not resolved until layout runs. Reading a
	 * zero height here made the travel negative and sent the grip above the
	 * header as the user scrolled, especially on Artists. */
	lv_obj_update_layout(root);
	scrub_top = lv_obj_get_y(list) + 8;
	scrub_range = LV_MAX(0, lv_obj_get_height(list) - SCRUB_H - 16);
	scrub = lv_obj_create(root);
	lv_obj_remove_style_all(scrub);
	lv_obj_set_size(scrub, SCRUB_W, SCRUB_H);
	lv_obj_set_x(scrub, w - SCRUB_W - 2);
	lv_obj_set_style_radius(scrub, SCRUB_W / 2, 0);
	lv_obj_set_style_bg_color(scrub, lv_color_hex(0x5a5a5a), 0);
	lv_obj_set_style_bg_color(scrub, lv_color_hex(0x8a8a8a), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(scrub, LV_OPA_COVER, 0);
	lv_obj_set_ext_click_area(scrub, 16);
	lv_obj_set_clickable(scrub, true);
	lv_obj_set_scrollable(scrub, false);
	/* A drag on the grip is the grip's: without this LVGL hands a vertical
	 * drag on to the first parent that scrolls, and the grip loses the press
	 * (on the R1 the list jumped back to its top). */
	lv_obj_set_scroll_chain_ver(scrub, false);
	lv_obj_set_scroll_chain_hor(scrub, false);
	for (int i = 0; i < 3; i++) {
		lv_obj_t *line = lv_obj_create(scrub);
		lv_obj_remove_style_all(line);
		lv_obj_set_size(line, 12, 2);
		lv_obj_set_style_bg_color(line, lv_color_white(), 0);
		lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
		lv_obj_set_clickable(line, false);
		lv_obj_align(line, LV_ALIGN_CENTER, 0, (i - 1) * 6);
	}
	lv_obj_add_event_cb(scrub, scrub_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_add_event_cb(scrub, scrub_cb, LV_EVENT_PRESSING, NULL);
	lv_obj_add_event_cb(scrub, scrub_cb, LV_EVENT_RELEASED, NULL);
	lv_obj_add_event_cb(scrub, scrub_cb, LV_EVENT_PRESS_LOST, NULL);
	scrub_bubble = lv_obj_create(root);
	lv_obj_remove_style_all(scrub_bubble);
	lv_obj_set_size(scrub_bubble, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_style_radius(scrub_bubble, 12, 0);
	lv_obj_set_style_bg_color(scrub_bubble, lv_color_hex(0x2a2a2a), 0);
	lv_obj_set_style_bg_opa(scrub_bubble, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_hor(scrub_bubble, 16, 0);
	lv_obj_set_style_pad_ver(scrub_bubble, 10, 0);
	lv_obj_set_clickable(scrub_bubble, false);
	lv_obj_set_scrollable(scrub_bubble, false);
	lv_obj_align(scrub_bubble, LV_ALIGN_TOP_RIGHT, -(SCRUB_W + 14), 0);
	scrub_label = lv_label_create(scrub_bubble);
	lv_label_set_text(scrub_label, "");
	lv_label_set_long_mode(scrub_label, LV_LABEL_LONG_DOT);
	lv_obj_set_style_max_width(scrub_label, w - SCRUB_W - 80, 0);
	lv_obj_set_style_text_font(scrub_label, &font_ui_22_bold, 0);
	lv_obj_set_style_text_color(scrub_label, lv_color_white(), 0);
	lv_obj_set_hidden(scrub_bubble, true);
	lv_obj_set_hidden(scrub, true);
	scrub_place(0);
	if (!scrub_timer) {
		scrub_timer = lv_timer_create(scrub_timer_cb, 250, NULL);
	}
}

static void lib_scroll_cb(lv_event_t *e) {
	(void)e;
	lib_window_check();
	if (scrub && !scrub_dragging) {
		scrub_place(lib_top_index());
		scrub_show();
	}
}
void collectionview_scroll_top(void) {
	if (!list) {
		return;
	}
	lv_anim_delete(list, NULL);
	if (lib_first > 0) {
		lib_jump(0);
	}
	lv_obj_scroll_to_y(list, 0, LV_ANIM_ON);
}
static unsigned catalog_requested;
static int catalog_result; /* 0 busy, 1 success, -1 failure; release/acquire */
static bool catalog_running;
static void *catalog_worker(void *unused) {
    (void)unused;
    thread_be_low_priority("library catalog");
    bool ok=collections_catalog_sync();
    __atomic_store_n(&catalog_result,ok?1:-1,__ATOMIC_RELEASE);
    return NULL;
}
static void catalog_poll(void) {
    if(catalog_running) {
        int result=__atomic_load_n(&catalog_result,__ATOMIC_ACQUIRE);
        if(!result) return;
        catalog_running=false;
        if(result>0) seen_music=catalog_requested;
    }
    if(!library_scan_running() && seen_music!=library_revision(LIBRARY_LIST_TRACKS)) {
        catalog_requested=library_revision(LIBRARY_LIST_TRACKS);
        __atomic_store_n(&catalog_result,0,__ATOMIC_RELEASE);
        pthread_t thread;
        pthread_attr_t attr;pthread_attr_init(&attr);pthread_attr_setstacksize(&attr,256*1024);
        int rc=pthread_create(&thread,&attr,catalog_worker,NULL);pthread_attr_destroy(&attr);
        if(!rc) {pthread_detach(thread);catalog_running=true;}
    }
}
void collectionview_refresh(void) {
	if (!root || !cfg)
		return;
	uint32_t started = lv_tick_get();
	collections_init(cfg->sd_root_path);
	if (seen_playlists != library_revision(LIBRARY_LIST_PLAYLIST))
		playlists_for_each(register_playlist, NULL);
	uint32_t registered = lv_tick_get();
	art_drop();
	memset(row_images, 0, sizeof(row_images));
	memset(row_boxes, 0, sizeof(row_boxes));
	lib_footer = NULL;
	/* Layout/scroll callbacks during rebuild must not use deleted handles. */
	scrub = scrub_bubble = scrub_label = NULL;
	lv_obj_clean(root);
	// The size the page was given, when it has not been laid out yet: built
	// ahead while hidden (main_menu.c), it measures 0 and every row came out
	// 0 pixels wide.
	int w = lv_obj_get_width(root), h = lv_obj_get_height(root);
	if (w <= 0 && LV_COORD_IS_PX(lv_obj_get_style_width(root, LV_PART_MAIN)))
		w = lv_obj_get_style_width(root, LV_PART_MAIN);
	if (h <= 0 && LV_COORD_IS_PX(lv_obj_get_style_height(root, LV_PART_MAIN)))
		h = lv_obj_get_style_height(root, LV_PART_MAIN);
	const char *names[] = {"All", "Playlists", "Albums", "Artists"};
	lv_obj_t *filters = panel(root, w, 44);
	lv_obj_set_flex_flow(filters, LV_FLEX_FLOW_ROW);
	lv_obj_set_scrollable(filters, true);
	lv_obj_set_scroll_dir(filters, LV_DIR_HOR);
	lv_obj_set_style_pad_column(filters, 8, 0);
	lv_obj_t *fb = button(filters, NULL, 42, 42, genre_sheet_cb, NULL);
	lv_obj_set_style_radius(fb, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_color(fb, lv_color_hex(0x2a2a2a), 0);
	lv_obj_set_style_bg_opa(fb, LV_OPA_COVER, 0);
	lv_obj_t *fi = lv_image_create(fb);
	lv_image_set_src(fi, &icon_equalizer);
	lv_obj_set_size(fi, 22, 22);
	lv_image_set_inner_align(fi, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_center(fi);
	if (genre_filter[0]) {
		lv_obj_t *g = button(filters, genre_filter, LV_SIZE_CONTENT, 42, genre_pick_cb, (void *)(intptr_t)-1);
		lv_obj_set_style_radius(g, 24, 0);
		lv_obj_set_style_pad_hor(g, 16, 0);
		lv_obj_set_style_bg_color(g, lv_color_hex(0x1ed760), 0);
		lv_obj_set_style_bg_opa(g, LV_OPA_COVER, 0);
		lv_obj_set_style_text_color(lv_obj_get_child(g, 0), lv_color_black(), 0);
		lv_obj_set_width(lv_obj_get_child(g, 0), LV_SIZE_CONTENT);
		/* The clear mark: the font has no multiplication sign. */
		lv_obj_set_flex_flow(g, LV_FLEX_FLOW_ROW);
		lv_obj_set_flex_align(g, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_column(g, 8, 0);
		lv_obj_t *gx = lv_image_create(g);
		lv_image_set_src(gx, &icon_close);
		lv_obj_set_size(gx, 16, 16);
		lv_image_set_inner_align(gx, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_style_image_recolor(gx, lv_color_black(), 0);
		lv_obj_set_style_image_recolor_opa(gx, LV_OPA_COVER, 0);
	}
	for (int i = 0; i < 4; i++) {
		lv_obj_t *b = button(filters, names[i], i ? 104 : 52, 42, filters_cb, (void *)(intptr_t)i);
		lv_obj_set_style_radius(b, 24, 0);
		lv_obj_set_style_bg_opa(b, filter == i ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
		lv_obj_set_style_bg_color(b, lv_color_hex(0x242424), 0);
		lv_obj_set_style_border_width(b, 1, 0);
		lv_obj_set_style_border_color(b, lv_color_hex(filter == i ? 0xaaaaaa : 0x555555), 0);
	}
	static const char *sorts[] = {"Recents", "Recently added", "Alphabetical", "Creator", "Custom"};
	lv_obj_t *b = button(root, sorts[sort_mode], 210, 42, sort_cb, NULL);
	lv_obj_set_pos(b, 0, 54);
	lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);
	lv_obj_set_style_text_align(lv_obj_get_child(b, 0), LV_TEXT_ALIGN_LEFT, 0);
	b = button(root, NULL, 60, 42, grid_cb, NULL);
	lv_obj_t *layout_icon = lv_image_create(b);
	lv_image_set_src(layout_icon, grid ? &sonix_icon_library : &sonix_icon_grid);
	lv_obj_set_size(layout_icon, 22, 22);
	lv_image_set_inner_align(layout_icon, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_center(layout_icon);
	lv_obj_set_pos(b, w - 60, 54);
	lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);

	list = panel(root, w, h - 108);
	lv_obj_set_pos(list, 0, 108);
	lv_obj_set_scrollable(list, true);
	lv_obj_set_scroll_dir(list, LV_DIR_VER);
	lv_obj_set_flex_flow(list, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_pad_row(list, 4, 0);
	lv_obj_set_style_pad_column(list, 10, 0);

	if (page == 0 && !folder[0] && !library_query[0] && !genre_filter[0] && (filter == 0 || filter == 1))
		collection_shortcut("Liked Songs", "Playlist \xC2\xB7 Your favorite tracks", &icon_heart, 0x5330aa, 1, w);
	catalog_poll();
	if (folder[0]) {
		button(list, "Back to Your Library", w, 44, folder_back, NULL);
		text(list, folder, w, &font_ui_22_bold, false);
	}
	if (library_query[0])
		button(list, "Clear search", w, 44, clear_query, NULL);
	lib_head = (int)lv_obj_get_child_count(list);
	lib_width = w;
	lib_first = 0;
	lib_loaded = 0;
	lib_end = false;
	lib_footer = NULL;
	uint32_t rows_started = lv_tick_get();
	lib_load_page(false);
	/* One page already exceeds the viewport. The scroll window fetches the
	 * next page near the edge; preloading 48 rows doubled initial query and
	 * widget cost on the R1 without putting another row on screen. */
	if (!lib_loaded && !folder[0] && filter)
		text(list, "No collections yet", w, &font_ui_18, true);
	if (!lib_loaded && folder[0])
		text(list, "Move playlists here from their options menu", w, &font_ui_16, true);
	lv_obj_add_event_cb(list, lib_scroll_cb, LV_EVENT_SCROLL, NULL);
	uint32_t rows_done = lv_tick_get();
	scrub_build(w);
	seen_revision = collections_revision();
	seen_playlists = library_revision(LIBRARY_LIST_PLAYLIST);
	if (lv_tick_elaps(started) > 100)
		printf("collections: refresh %u ms (register %u, chrome %u, rows %u, count %u)\n",
			(unsigned)lv_tick_elaps(started), (unsigned)(registered-started),
			(unsigned)(rows_started-registered), (unsigned)(rows_done-rows_started),
			(unsigned)lv_tick_elaps(rows_done));
}
static void request_row_art(int slot) {
    int row=art[slot].row;
    if(row<0 || row>=LIB_SLOTS || !row_boxes[row])return;
    collection_item_t *item=&rows[row];char path[512]="";
    if(item->kind==COLLECTION_ARTIST) {
        /* One row per try, not a whole index of the artist's songs: those
         * took up to 1.2 s each on the R1 for every artist on screen. */
        while(art[slot].candidate<8 && !path[0]) {
            char candidate[512];
            if(!library_sample_path(LIBRARY_LIST_TRACKS,LIBRARY_FILTER_ARTIST,item->key,(int)art[slot].candidate++,candidate,sizeof(candidate)))break;
            if(!access(candidate,R_OK))snprintf(path,sizeof(path),"%s",candidate);
        }
    } else if(item->kind==COLLECTION_PLAYLIST && library_playlist_cover_get(item->key,path,sizeof(path))) {
        /* the cover chosen for it */
    } else {
        library_for_each(item->kind==COLLECTION_ALBUM?LIBRARY_LIST_TRACKS:LIBRARY_LIST_PLAYLIST,
            item->kind==COLLECTION_ALBUM?LIBRARY_FILTER_ALBUM:LIBRARY_FILTER_NONE,item->key,sample_track,path);
    }
    if(path[0])coverloader_request(COVERLOADER_COLLECTION_BASE+slot,path,grid?(lv_obj_get_width(list)-10)/2-12:64);
}

static void art_update(void) {
	if (!root || !lv_obj_is_visible(root)) {
		/* Keep the eight bounded visible covers across tab switches. A refresh
		 * or recycled row still releases them; returning to the same page
		 * should not repeat metadata reads while background queries run. */
		return;
	}
	lv_area_t viewport;
	lv_obj_get_coords(list, &viewport);
	for (int slot = 0; slot < 8; slot++) {
		int row = art[slot].row;
		if (row < 0 || !row_boxes[row])
			continue;
		lv_area_t box;
		lv_obj_get_coords(row_boxes[row], &box);
		if (box.y2 < viewport.y1 || box.y1 > viewport.y2) {
			coverloader_release(COVERLOADER_COLLECTION_BASE + slot);
			if (art[slot].loaded) {
				lv_image_set_src(row_images[row], rows[row].kind==COLLECTION_ARTIST?&icon_artist:&icon_album);
				cover_free(&art[slot].image);
			}
			art[slot].row = -1;
			art[slot].loaded = false;
		} else if (!art[slot].loaded) {
			bool found = false;
			if (coverloader_take(COVERLOADER_COLLECTION_BASE + slot, &art[slot].image, &found) && found) {
				art[slot].loaded = true;
				lv_image_set_src(row_images[row], &art[slot].image.dsc);
			} else if(found && rows[row].kind==COLLECTION_ARTIST && art[slot].candidate<8) {
                request_row_art(slot);
            }
		}
	}
	for (int row = 0; row < LIB_SLOTS; row++) {
		if (!row_boxes[row] || rows[row].kind == COLLECTION_PODCAST || rows[row].kind == COLLECTION_FOLDER)
			continue;
		lv_area_t box;
		lv_obj_get_coords(row_boxes[row], &box);
		if (box.y2 < viewport.y1 || box.y1 > viewport.y2)
			continue;
		int empty = -1;
		bool held = false;
		for (int i = 0; i < 8; i++) {
			if (art[i].row == row)
				held = true;
			else if (art[i].row < 0)
				empty = i;
		}
		if (held || empty < 0)
			continue;
        art[empty].row=row;art[empty].candidate=0;
        request_row_art(empty);
	}
}
bool collectionview_stale(void) {
	return seen_revision != collections_revision() || seen_playlists != library_revision(LIBRARY_LIST_PLAYLIST) ||
		   (!catalog_running && !library_scan_running() && seen_music != library_revision(LIBRARY_LIST_TRACKS));
}
void collectionview_refresh_if_stale(void) {
	if (!list || collectionview_stale())
		collectionview_refresh();
}
static void refresh_cb(lv_timer_t *t) {
	(void)t;
    catalog_poll();
	if (root && list && lv_obj_is_visible(root)) {
		lib_window_check();
	}
	art_update();
	if (root && lv_obj_is_visible(root) && (seen_revision != collections_revision() || seen_playlists != library_revision(LIBRARY_LIST_PLAYLIST) || (!catalog_running && !library_scan_running() && seen_music != library_revision(LIBRARY_LIST_TRACKS))) && !sheet)
		collectionview_refresh();
}
static void root_deleted(lv_event_t *e) {
	(void)e;
	art_drop();
	root = NULL;
	if (refresh_timer) {
		lv_timer_delete(refresh_timer);
		refresh_timer = NULL;
	}
}
void collectionview_build(lv_obj_t *parent, gui_config_t *config) {
	root = parent;
	cfg = config;
	seen_music = ~0u;
	filter = (int)config_get_int("collections", "filter", 0);
	if (filter < 0 || filter > 3)
		filter = 0;
	sort_mode = (int)config_get_int("collections", "sort", 0);
	if (sort_mode < 0 || sort_mode > 4)
		sort_mode = 0;
	grid = config_get_int("collections", "grid", 0) != 0;
	lv_obj_add_event_cb(root, root_deleted, LV_EVENT_DELETE, NULL);
	collectionview_refresh();
	if (!refresh_timer)
		refresh_timer = lv_timer_create(refresh_cb, 750, NULL);
}
