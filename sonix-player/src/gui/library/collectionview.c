#include "collectionview.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/library/medialist.h"
#include "src/gui/library/playlistpage.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/shell/popover.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/gui/streaming/podcastpage.h"
#include "src/system/core/config.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/playback/playlist.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static gui_config_t *cfg;
static lv_obj_t *row_images[24], *row_boxes[24];
static struct {
	int row;
	cover_image_t image;
	bool loaded;
} art[8];
static void art_drop(void) {
	for (int i = 0; i < 8; i++) {
		coverloader_release(COVERLOADER_COLLECTION_BASE + i);
		if (art[i].loaded) {
			if (art[i].row >= 0 && art[i].row < 24 && row_images[art[i].row])
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
static bool grid;
static collection_item_t rows[24], menu_item;
static int row_count;
static char track[512], track_title[256], track_artist[256];
static int member_page;
static char member_names[24][201];
static int member_count, member_seen;
static unsigned seen_revision, seen_playlists;
static lv_timer_t *refresh_timer;
static lv_obj_t *text(lv_obj_t *p, const char *value, int width, const lv_font_t *font, bool dim) {
	lv_obj_t *o = lv_label_create(p);
	lv_obj_set_width(o, width);
	lv_label_set_text(o, value);
	lv_label_set_long_mode(o, LV_LABEL_LONG_DOT);
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
	return o;
}
static lv_obj_t *button(lv_obj_t *p, const char *s, int w, int h, lv_event_cb_t cb, void *u) {
	lv_obj_t *o = panel(p, w, h);
	lv_obj_add_style(o, &theme_style_card, 0);
	lv_obj_set_style_radius(o, 8, 0);
	lv_obj_set_clickable(o, true);
	if (s) {
		lv_obj_t *l = text(o, s, w - 16, &font_ui_18, false);
		lv_obj_center(l);
	}
	lv_obj_add_event_cb(o, cb, LV_EVENT_CLICKED, u);
	return o;
}
static void close_sheet(lv_event_t *e) {
	(void)e;
	if (sheet) {
		lv_obj_delete(sheet);
		sheet = NULL;
		sheet_body = NULL;
	}
	collectionview_refresh();
}
static void sheet_new(const char *title) {
	if (sheet)
		lv_obj_delete(sheet);
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
	sheet = panel(lv_layer_top(), w, h);
	lv_obj_set_style_bg_color(sheet, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(sheet, LV_OPA_70, 0);
	lv_obj_set_clickable(sheet, true);
	lv_obj_t *body = panel(sheet, w, h - 100);
	lv_obj_set_align(body, LV_ALIGN_BOTTOM_MID);
	lv_obj_add_style(body, &theme_style_card, 0);
	lv_obj_set_style_radius(body, 20, 0);
	lv_obj_t *l = text(body, title, w - 110, &font_ui_24_bold, false);
	lv_obj_set_pos(l, 20, 18);
	lv_obj_t *done = button(body, "Done", 76, 46, close_sheet, NULL);
	lv_obj_set_pos(done, w - 92, 10);
	sheet_body = panel(body, w - 32, h - 174);
	lv_obj_set_pos(sheet_body, 16, 70);
	lv_obj_set_scrollable(sheet_body, true);
	lv_obj_set_scroll_dir(sheet_body, LV_DIR_VER);
	lv_obj_set_flex_flow(sheet_body, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(sheet_body, 10, 0);
	lv_obj_update_layout(body);
}
static void open_row(lv_event_t *e) {
	collection_item_t *r = lv_event_get_user_data(e);
	collections_touch(r->kind, r->key);
	if (r->kind == COLLECTION_PLAYLIST)
		medialist_open(r->title, LIBRARY_LIST_PLAYLIST, LIBRARY_FILTER_NONE, r->key);
	else if (r->kind == COLLECTION_ALBUM)
		medialist_open(r->title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, r->key);
	else if (r->kind == COLLECTION_ARTIST)
		medialist_open_artist(r->key);
	else
		podcastpage_open_feed(strtoll(r->key, NULL, 10), r->title);
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
	sheet_new(title ? title : key);
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	button(sheet_body, kind == COLLECTION_ARTIST ? (saved.saved ? "Unfollow artist" : "Follow artist") : (saved.saved ? "Remove from Your Library" : "Save to Your Library"), w, 58, save_action, NULL);
	button(sheet_body, saved.pin ? "Unpin" : "Pin to top", w, 58, pin_action, NULL);
	text(sheet_body, "Your music files stay on the card.", w, &font_ui_16, true);
}
static void row_options(lv_event_t *e) {
	collection_item_t *r = lv_event_get_user_data(e);
	collectionview_collection_options(r->kind, r->key, r->title, r->creator);
}
static void route(lv_event_t *e) {
	switch ((intptr_t)lv_event_get_user_data(e)) {
	case 1:
		medialist_open("Liked Songs", LIBRARY_LIST_FAVOURITES, LIBRARY_FILTER_NONE, NULL);
		break;
	case 2:
		medialist_open("All albums", LIBRARY_LIST_ALBUMS, LIBRARY_FILTER_NONE, NULL);
		break;
	case 3:
		medialist_open("All artists", LIBRARY_LIST_ARTISTS, LIBRARY_FILTER_NONE, NULL);
		break;
	case 4:
		podcastpage_open_followed();
		break;
	case 5:
		playlistpage_open();
		break;
	case 6:
		medialist_open("Most played", LIBRARY_LIST_MOST_PLAYED, LIBRARY_FILTER_NONE, NULL);
		break;
	}
}
static bool register_playlist(const char *name, void *user) {
	(void)user;
	collection_item_t existing;
	if (!collections_get(COLLECTION_PLAYLIST, name, &existing))
		collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
	return true;
}
static void page_cb(lv_event_t *e) {
	page += (int)(intptr_t)lv_event_get_user_data(e);
	if (page < 0)
		page = 0;
	collectionview_refresh();
}
static void filters_cb(lv_event_t *e) {
	filter = (int)(intptr_t)lv_event_get_user_data(e);
	page = 0;
	config_set_int("collections", "filter", filter);
	config_save();
	collectionview_refresh();
}
static void sort_cb(lv_event_t *e) {
	(void)e;
	sort_mode = (sort_mode + 1) % 4;
	page = 0;
	config_set_int("collections", "sort", sort_mode);
	config_save();
	collectionview_refresh();
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
}
static void name_accept(lv_event_t *e) {
	(void)e;
	const char *name = lv_textarea_get_text(name_field);
	if (!playlists_create(name)) {
		gui_notify_popup("Choose a new, valid playlist name.");
		return;
	}
	bool ok = !track[0] || playlists_add_track(name, track);
	collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
	name_close(NULL);
	if (!ok)
		gui_notify_popup("Playlist created, but the track could not be added.");
	if (sheet)
		members_draw();
	collectionview_refresh();
}
static void create_cb(lv_event_t *e) {
	bool membership = lv_event_get_user_data(e) != NULL;
	if (!membership)
		track[0] = 0;
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
	name_layer = panel(lv_layer_top(), w, h);
	lv_obj_set_clickable(name_layer, true);
	lv_obj_t *l = text(name_layer, "New playlist", w - 40, &font_ui_24_bold, false);
	lv_obj_set_pos(l, 20, 60);
	name_field = lv_textarea_create(name_layer);
	lv_obj_add_style(name_field, &theme_style_card, 0);
	lv_obj_add_style(name_field, &theme_style_text, 0);
	lv_obj_set_style_text_font(name_field, &font_ui_24, 0);
	lv_obj_set_size(name_field, w - 40, 64);
	lv_obj_set_pos(name_field, 20, 120);
	lv_textarea_set_one_line(name_field, true);
	lv_textarea_set_max_length(name_field, 200);
	lv_textarea_set_placeholder_text(name_field, "Playlist name");
	keyboard_style_caret(name_field);
	lv_obj_t *cancel = button(name_layer, "Cancel", 100, 48, name_close, NULL);
	lv_obj_set_pos(cancel, 20, 205);
	keyboard_create(name_layer, w, 300, name_field, NULL, "Create", name_accept, NULL);
}
static bool member_collect(const char *name, void *user) {
	(void)user;
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
static void members_draw(void) {
	if (!sheet_body)
		return;
	lv_obj_clean(sheet_body);
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	text(sheet_body, track_title, w, &font_ui_22_bold, false);
	text(sheet_body, track_artist, w, &font_ui_16, true);
	button(sheet_body, library_fav_contains(track) ? "[x] Liked Songs" : "[ ] Liked Songs", w, 56, liked_cb, NULL);
	button(sheet_body, "+ Create new playlist", w, 56, create_cb, (void *)1);
	member_count = member_seen = 0;
	playlists_for_each(member_collect, NULL);
	for (int i = 0; i < member_count; i++) {
		char line[220];
		snprintf(line, sizeof(line), "[%c] %.200s", playlists_has_track(member_names[i], track) ? 'x' : ' ', member_names[i]);
		button(sheet_body, line, w, 56, member_toggle, member_names[i]);
	}
	if (member_page)
		button(sheet_body, "Previous playlists", w, 48, member_page_cb, (void *)(intptr_t)-1);
	if (member_count == 24)
		button(sheet_body, "More playlists", w, 48, member_page_cb, (void *)(intptr_t)1);
}
void collectionview_manage_track(const char *path) {
	if (!path || !*path || !gui_card_available())
		return;
	snprintf(track, sizeof(track), "%s", path);
	track_title[0] = track_artist[0] = 0;
	playlists_track_names(path, track_title, sizeof(track_title), track_artist, sizeof(track_artist));
	member_page = 0;
	collections_init(playlist_card_root());
	sheet_new("Add to playlists");
	members_draw();
}
void collectionview_refresh(void) {
	if (!root || !cfg)
		return;
	collections_init(cfg->sd_root_path);
	playlists_for_each(register_playlist, NULL);
	art_drop();
	memset(row_images, 0, sizeof(row_images));
	memset(row_boxes, 0, sizeof(row_boxes));
	lv_obj_clean(root);
	int w = lv_obj_get_width(root), h = lv_obj_get_height(root);
	const char *names[] = {"All", "Playlists", "Albums", "Artists", "Podcasts"};
	lv_obj_t *filters = panel(root, w, 44);
	lv_obj_set_flex_flow(filters, LV_FLEX_FLOW_ROW);
	lv_obj_set_scrollable(filters, true);
	lv_obj_set_scroll_dir(filters, LV_DIR_HOR);
	lv_obj_set_style_pad_column(filters, 8, 0);
	for (int i = 0; i < 5; i++) {
		lv_obj_t *b = button(filters, names[i], i ? 104 : 56, 42, filters_cb, (void *)(intptr_t)i);
		if (filter == i)
			lv_obj_set_style_bg_color(b, lv_color_hex(0x1DB954), 0);
	}
	static const char *sorts[] = {"Recents", "Recently added", "Alphabetical", "Creator"};
	lv_obj_t *b = button(root, sorts[sort_mode], 180, 42, sort_cb, NULL);
	lv_obj_set_pos(b, 0, 54);
	b = button(root, grid ? "List" : "Grid", 80, 42, grid_cb, NULL);
	lv_obj_set_pos(b, w - 166, 54);
	b = button(root, "+", 68, 42, create_cb, NULL);
	lv_obj_set_pos(b, w - 68, 54);
	list = panel(root, w, h - 108);
	lv_obj_set_pos(list, 0, 108);
	lv_obj_set_scrollable(list, true);
	lv_obj_set_scroll_dir(list, LV_DIR_VER);
	lv_obj_set_flex_flow(list, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_pad_row(list, 10, 0);
	lv_obj_set_style_pad_column(list, 10, 0);

	row_count = collections_list(filter, (collection_sort_t)sort_mode, page * 24, rows, 24);
	for (int i = 0; i < row_count; i++) {
		collection_item_t *r = &rows[i];
		if (r->kind == COLLECTION_PLAYLIST && !playlists_exists(r->key))
			continue;
		int rw = grid ? (w - 10) / 2 : w, rh = grid ? 190 : 88;
		lv_obj_t *row = button(list, NULL, rw, rh, open_row, r);
		row_boxes[i] = row;
		row_images[i] = lv_image_create(row);
		lv_image_set_src(row_images[i], r->kind == COLLECTION_ARTIST ? &icon_artist : &icon_album);
		lv_obj_set_size(row_images[i], 64, 64);
		lv_image_set_inner_align(row_images[i], LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_pos(row_images[i], 12, 12);
		char title[280];
		snprintf(title, sizeof(title), "%s%s", r->pin ? "PIN  " : "", r->title);
		lv_obj_t *l = text(row, title, grid ? rw - 24 : rw - 132, &font_ui_22_bold, false);
		lv_obj_set_pos(l, grid ? 12 : 86, grid ? 92 : 12);
		const char *kind = r->kind == COLLECTION_PLAYLIST ? "Playlist" : r->kind == COLLECTION_ALBUM ? "Saved album" : r->kind == COLLECTION_ARTIST ? "Followed artist" : "Podcast";
		char sub[300];
		snprintf(sub, sizeof(sub), "%s%s%s", kind, r->creator[0] ? " / " : "", r->creator);
		l = text(row, sub, grid ? rw - 24 : rw - 98, &font_ui_16, true);
		lv_obj_set_pos(l, grid ? 12 : 86, grid ? 130 : 48);
		b = button(row, "...", 42, 40, row_options, r);
		lv_obj_set_pos(b, rw - 46, 5);
	}
	if (filter == 0) {
		button(list, "Liked Songs", w, 62, route, (void *)1);
		button(list, "Most played", w, 54, route, (void *)6);
	}
	if (!row_count)
		text(list, "Save albums or follow artists from song options. Your files remain available below.", w, &font_ui_18, true);
	if (page)
		button(list, "Previous", (w - 10) / 2, 48, page_cb, (void *)(intptr_t)-1);
	if (row_count == 24)
		button(list, "More", (w - 10) / 2, 48, page_cb, (void *)(intptr_t)1);
	if (filter == 0 || filter == 1)
		button(list, "All playlists / rename / order", w, 54, route, (void *)5);
	if (filter == 0 || filter == 2)
		button(list, "All indexed albums", w, 54, route, (void *)2);
	if (filter == 0 || filter == 3)
		button(list, "All indexed artists", w, 54, route, (void *)3);
	if (filter == 0 || filter == 4)
		button(list, "Podcasts & shows", w, 54, route, (void *)4);
	seen_revision = collections_revision();
	seen_playlists = library_revision(LIBRARY_LIST_PLAYLIST);
}
static void art_update(void) {
	if (!root || !lv_obj_is_visible(root)) {
		art_drop();
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
				lv_image_set_src(row_images[row], &icon_album);
				cover_free(&art[slot].image);
			}
			art[slot].row = -1;
			art[slot].loaded = false;
		} else if (!art[slot].loaded) {
			bool found = false;
			if (coverloader_take(COVERLOADER_COLLECTION_BASE + slot, &art[slot].image, &found) && found) {
				art[slot].loaded = true;
				lv_image_set_src(row_images[row], &art[slot].image.dsc);
			}
		}
	}
	for (int row = 0; row < row_count; row++) {
		if (!row_boxes[row] || rows[row].kind == COLLECTION_ARTIST || rows[row].kind == COLLECTION_PODCAST)
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
		char path[512] = "";
		library_for_each(rows[row].kind == COLLECTION_ALBUM ? LIBRARY_LIST_TRACKS : LIBRARY_LIST_PLAYLIST, rows[row].kind == COLLECTION_ALBUM ? LIBRARY_FILTER_ALBUM : LIBRARY_FILTER_NONE, rows[row].key, sample_track, path);
		art[empty].row = row;
		if (path[0])
			coverloader_request(COVERLOADER_COLLECTION_BASE + empty, path, 64);
	}
}
static void refresh_cb(lv_timer_t *t) {
	(void)t;
	art_update();
	if (root && lv_obj_is_visible(root) && (seen_revision != collections_revision() || seen_playlists != library_revision(LIBRARY_LIST_PLAYLIST)) && !sheet)
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
	filter = (int)config_get_int("collections", "filter", 0);
	if (filter < 0 || filter > 4)
		filter = 0;
	sort_mode = (int)config_get_int("collections", "sort", 0);
	if (sort_mode < 0 || sort_mode > 3)
		sort_mode = 0;
	grid = config_get_int("collections", "grid", 0) != 0;
	lv_obj_add_event_cb(root, root_deleted, LV_EVENT_DELETE, NULL);
	collectionview_refresh();
	if (!refresh_timer)
		refresh_timer = lv_timer_create(refresh_cb, 750, NULL);
}
