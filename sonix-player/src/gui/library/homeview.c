#include "homeview.h"
#include "playlistview.h"
#include "collectionview.h"
#include "medialist.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/nowplaying/player.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/theme.h"
#include "src/gui/streaming/podcastpage.h"
#include "src/system/core/config.h"
#include "src/system/library/collections.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HOME_MAX 36
#define HOME_ART 8
#define HOME_SLOT 182
typedef struct {
	library_home_context_t context;
	collection_kind_t collection;
	char creator[256];
	lv_obj_t *art, *card;
	bool resume;
	int art_size;
} item_t;
static item_t items[HOME_MAX], menu;
static int item_count, scope;
static bool menu_open;
static lv_obj_t *root;
static gui_config_t *config;
static struct {
	int item;
	bool requested, settled;
	cover_image_t cover;
} art[HOME_ART];
static const char *sections[] = {"Quick access", "Jump back in", "Recently added", "On repeat", "Made for you", "Rediscover"};
static int order[6];
static lv_obj_t *text(lv_obj_t *p, const char *s, int width, const lv_font_t *font, bool dim) {
	lv_obj_t *l = lv_label_create(p);
	lv_label_set_text(l, s);
	lv_obj_set_width(l, width);
	lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
	lv_obj_set_style_text_font(l, font, 0);
	lv_obj_set_style_text_color(l, lv_color_hex(dim ? 0xb3b3b3 : 0xffffff), 0);
	return l;
}
static lv_obj_t *box(lv_obj_t *p, int w, int h) {
	lv_obj_t *o = lv_obj_create(p);
	lv_obj_remove_style_all(o);
	lv_obj_set_size(o, w, h);
	lv_obj_set_scrollable(o, false);
	return o;
}
static lv_obj_t *button(lv_obj_t *p, const char *s, int w, int h, lv_event_cb_t cb, void *u) {
	lv_obj_t *o = box(p, w, h);
	lv_obj_set_clickable(o, true);
	lv_obj_add_event_cb(o, cb, LV_EVENT_CLICKED, u);
	if (s) {
		lv_obj_t *l = text(o, s, w - 12, &font_ui_18, false);
		lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_center(l);
	}
	return o;
}
static void icon(lv_obj_t *b, const lv_image_dsc_t *glyph, int size, uint32_t color) {
	lv_obj_t *im = lv_image_create(b);
	lv_image_set_src(im, glyph);
	lv_obj_set_size(im, size, size);
	lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_center(im);
	lv_obj_set_style_image_recolor(im, lv_color_hex(color), 0);
	lv_obj_set_style_image_recolor_opa(im, 255, 0);
}
static bool sample(const char *name, const char *path, const char *artist, void *u) {
	(void)name;
	item_t *item = u;
	snprintf(item->context.path, sizeof(item->context.path), "%s", path ? path : "");
	snprintf(item->creator, sizeof(item->creator), "%s", artist ? artist : "");
	return false;
}
static bool resolve(item_t *item) {
	if (item->collection == COLLECTION_ARTIST || item->collection == COLLECTION_PODCAST)
		return true;
	library_index_spec_t *s = &item->context.spec;
	library_index_t *ix = library_index_open(s->kind, s->filter, s->value, s->order, s->desc);
	bool valid = ix && library_index_count(ix) > 0;
	if (valid) {
		char saved[512];
		snprintf(saved, sizeof(saved), "%s", item->context.path);
		library_index_window(ix, 0, 1, sample, item);
		if (item->resume && saved[0] && library_index_find_path(ix, saved) >= 0)
			snprintf(item->context.path, sizeof(item->context.path), "%s", saved);
		else if (item->resume)
			item->context.position = 0;
	}
	if (ix)
		library_index_close(ix);
	return valid;
}
static void open_item(lv_event_t *e) {
	item_t *i = lv_event_get_user_data(e);
	if (player_sheet_drag_active() || menu_open)
		return;
	if (i->collection == COLLECTION_ARTIST) {
		medialist_open_artist(i->context.spec.value);
		return;
	}
	if (i->collection == COLLECTION_PODCAST) {
		podcastpage_open_feed(strtoll(i->context.spec.value, NULL, 10), i->context.title);
		return;
	}
	if(i->context.spec.kind==LIBRARY_LIST_PLAYLIST){playlistview_open(i->context.spec.value,false);return;}
	medialist_open(i->context.title, i->context.spec.kind, i->context.spec.filter, i->context.spec.value);
}
static void play(item_t *i) {
    if(i->collection)collections_touch(i->collection,i->context.spec.value);
	library_index_spec_t *s = &i->context.spec;
	library_index_t *ix = library_index_open(s->kind, s->filter, s->value, s->order, s->desc);
	if (!ix)
		return;
	int start = i->resume ? library_index_find_path(ix, i->context.path) : 0;
	if (start < 0)
		start = 0;
	if (device_state_play_index_at(ix, start, i->resume ? i->context.position : 0)) {
		player_refresh_now_playing();
		player_sheet_open(true);
	}
}
static void play_cb(lv_event_t *e) { play(lv_event_get_user_data(e)); }
static bool queue_track(const char *name, const char *path, const char *artist, void *u) {
	(void)name;
	(void)artist;
	snprintf(u, 512, "%s", path ? path : "");
	return false;
}
static void menu_action(lv_event_t *e) {
	int action = (int)(intptr_t)lv_event_get_user_data(e);
	bottomsheet_close(true);
	if (action == 0)
		play(&menu);
	if (action == 1) {
		collection_item_t saved = {0};
		collections_get(menu.collection, menu.context.spec.value, &saved);
		if (!saved.saved)
			collections_put(menu.collection, menu.context.spec.value, menu.context.title, menu.creator, true);
		if (!collections_pin(menu.collection, menu.context.spec.value, !saved.pin))
			gui_notify_popup("You can pin up to four collections.");
	}
	if (action == 2) {
		library_index_spec_t *s = &menu.context.spec;
		int n = 0;
		library_index_t *ix = library_index_open(s->kind, s->filter, s->value, s->order, s->desc);
		if (ix) {
			int total = library_index_count(ix);
			for (int j = 0; j < total && j < 64; j++) {
				char path[512] = "";
				library_index_window(ix, j, 1, queue_track, path);
				if (!path[0] || !playlist_priority_add(path))
					break;
				n++;
			}
			library_index_close(ix);
		}
		device_state_queue_changed();
		char msg[80];
		snprintf(msg, sizeof(msg), "Added %d tracks to queue", n);
		gui_notify_popup(msg);
	}
	if (action == 3 && menu.creator[0])
		medialist_open_artist(menu.creator);
}
static void menu_cleanup(void) { menu_open = false; }
static void options(lv_event_t *e) {
	menu = *(item_t *)lv_event_get_user_data(e);
	lv_obj_t *body = bottomsheet_open(360, menu_cleanup);
	menu_open = true;
	lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_all(body, 16, 0);
	int w = config->screen_width - 32;
	text(body, menu.context.title, w, &font_ui_24_bold, false);
	if (menu.collection != COLLECTION_ARTIST && menu.collection != COLLECTION_PODCAST) {
		button(body, menu.resume ? "Resume listening" : "Play collection", w, 52, menu_action, (void *)0);
		button(body, "Add to queue", w, 52, menu_action, (void *)2);
	}
	if (menu.collection) {
		collection_item_t saved = {0};
		collections_get(menu.collection, menu.context.spec.value, &saved);
		button(body, saved.pin ? "Unpin from Home" : "Pin to Home", w, 52, menu_action, (void *)1);
	}
	if (menu.creator[0])
		button(body, "Go to artist", w, 52, menu_action, (void *)3);
}
static bool visible(item_t *i) {
	if (!i->art || !lv_obj_is_visible(i->art))
		return false;
	lv_area_t r, a, p;
	lv_obj_get_coords(root, &r);
	lv_obj_get_coords(i->art, &a);
	lv_obj_get_coords(lv_obj_get_parent(i->card), &p);
	return a.y2 >= r.y1 && a.y1 <= r.y2 && a.x2 >= p.x1 && a.x1 <= p.x2 && a.x2 >= r.x1 && a.x1 <= r.x2;
}
static void art_release(int slot) {
	int n = art[slot].item;
	coverloader_release(HOME_SLOT + slot);
	if (n >= 0 && n < item_count && items[n].art)
		lv_image_set_src(items[n].art, &icon_album);
	cover_free(&art[slot].cover);
	art[slot].item = -1;
	art[slot].requested = art[slot].settled = false;
}
void homeview_art_drop(void) {
	for (int i = 0; i < HOME_ART; i++)
		art_release(i);
}
void homeview_art_update(void) {
	if (!root || !lv_obj_is_visible(root) || player_sheet_is_open())
		return;
	for (int i = 0; i < HOME_ART; i++)
		if (art[i].item >= 0 && !visible(&items[art[i].item]))
			art_release(i);
	for (int j = 0; j < item_count; j++) {
		if (!items[j].context.path[0] || !visible(&items[j]))
			continue;
		bool found = false;
		int free_slot = -1;
		for (int i = 0; i < HOME_ART; i++) {
			if (art[i].item == j)
				found = true;
			if (art[i].item < 0)
				free_slot = i;
		}
		if (!found && free_slot >= 0) {
			art[free_slot].item = j;
			coverloader_request(HOME_SLOT + free_slot, items[j].context.path, items[j].art_size);
			art[free_slot].requested = true;
		}
	}
	for (int i = 0; i < HOME_ART; i++)
		if (art[i].item >= 0 && !art[i].settled) {
			bool done = false;
			if (coverloader_take(HOME_SLOT + i, &art[i].cover, &done))
				lv_image_set_src(items[art[i].item].art, &art[i].cover.dsc);
			art[i].settled = done;
		}
}
static void card(lv_obj_t *parent, const item_t *source, bool compact) {
	if (item_count == HOME_MAX)
		return;
	item_t *i = &items[item_count++];
	*i = *source;
	int w = compact ? (config->screen_width - 48) / 2 : 166, h = compact ? 76 : 228;
	i->card = button(parent, NULL, w, h, open_item, i);
	lv_obj_set_style_radius(i->card, 5, 0);
	if (compact) {
		lv_obj_set_style_bg_color(i->card, lv_color_hex(0x282828), 0);
		lv_obj_set_style_bg_opa(i->card, 255, 0);
	}
	i->art_size = compact ? 62 : 156;
	i->art = lv_image_create(i->card);
	lv_image_set_src(i->art, i->collection == COLLECTION_ARTIST ? &icon_artist : i->collection == COLLECTION_PODCAST ? &icon_podcast_list : i->context.spec.kind == LIBRARY_LIST_FAVOURITES ? &icon_heart : &icon_album);
	lv_obj_set_size(i->art, i->art_size, i->art_size);
	lv_image_set_inner_align(i->art, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_set_pos(i->art, compact ? 0 : 5, compact ? 7 : 0);
	if (!i->context.path[0]) {
		lv_obj_set_style_bg_color(i->art, lv_color_hex(0x303b42 + item_count * 0x080300), 0);
		lv_obj_set_style_bg_opa(i->art, 255, 0);
		if (i->context.spec.kind == LIBRARY_LIST_FAVOURITES) {
			lv_obj_set_style_bg_color(i->art, lv_color_hex(0x5821cf), 0);
			lv_obj_set_style_bg_grad_color(i->art, lv_color_hex(0x98c8ca), 0);
			lv_obj_set_style_bg_grad_dir(i->art, LV_GRAD_DIR_VER, 0);
		}
	}
	lv_obj_t *t = text(i->card, i->context.title, compact ? w - 74 : w - 8, compact ? &font_ui_16 : &font_ui_18, false);
	lv_obj_set_pos(t, compact ? 70 : 5, compact ? 12 : 164);
	lv_obj_set_height(t, compact ? 42 : 26);
	if (!compact) {
		t = text(i->card, i->resume ? "Resume listening" : i->creator[0] ? i->creator : i->collection == COLLECTION_PLAYLIST ? "Playlist" : "Local collection", w - 8, &font_ui_14, true);
		lv_obj_set_pos(t, 5, 194);
		lv_obj_set_height(t, 22);
		if (i->collection != COLLECTION_ARTIST && i->collection != COLLECTION_PODCAST) {
			lv_obj_t *b = button(i->card, NULL, 42, 42, play_cb, i);
			lv_obj_set_pos(b, 114, 108);
			lv_obj_set_style_bg_color(b, lv_color_hex(0x1ed760), 0);
			lv_obj_set_style_bg_opa(b, 255, 0);
			lv_obj_set_style_radius(b, 22, 0);
			icon(b, &icon_play, 20, 0x111111);
		}
	}
	lv_obj_t *more = button(i->card, "...", 32, 30, options, i);
	lv_obj_set_pos(more, compact ? w - 32 : 126, compact ? 45 : 0);
	lv_obj_set_style_bg_color(more, lv_color_hex(0x202020), 0);
	lv_obj_set_style_bg_opa(more, 180, 0);
	lv_obj_add_event_cb(i->card, options, LV_EVENT_LONG_PRESSED, i);
}
static lv_obj_t *shelf(const char *title) {
	text(root, title, config->screen_width - 40, &font_ui_24_bold, false);
	lv_obj_t *row = box(root, config->screen_width - 40, 236);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 12, 0);
	lv_obj_set_scrollable(row, true);
	lv_obj_set_scroll_dir(row, LV_DIR_HOR);
	lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
	return row;
}
static item_t from_context(library_home_context_t context) {
	item_t i = {0};
	i.context = context;
	i.collection = context.spec.kind == LIBRARY_LIST_PLAYLIST ? COLLECTION_PLAYLIST : COLLECTION_ALBUM;
	return i;
}
static item_t from_saved(collection_item_t r) {
	item_t i = {0};
	i.collection = r.kind;
	i.context.spec.valid = true;
	i.context.spec.kind = r.kind == COLLECTION_PLAYLIST ? LIBRARY_LIST_PLAYLIST : LIBRARY_LIST_TRACKS;
	i.context.spec.filter = r.kind == COLLECTION_ALBUM ? LIBRARY_FILTER_ALBUM : LIBRARY_FILTER_NONE;
	snprintf(i.context.spec.value, sizeof(i.context.spec.value), "%.255s", r.key);
	snprintf(i.context.title, sizeof(i.context.title), "%s", r.title);
	snprintf(i.creator, sizeof(i.creator), "%s", r.creator);
	i.context.added = r.added;
	return i;
}
static item_t quick[6];
static int quick_count;
static void quick_add(item_t i) {
	if (quick_count == 6)
		return;
	for (int j = 0; j < quick_count; j++)
		if (quick[j].context.spec.kind == i.context.spec.kind && quick[j].context.spec.filter == i.context.spec.filter && !strcmp(quick[j].context.spec.value, i.context.spec.value))
			return;
	if (i.context.spec.kind==LIBRARY_LIST_FAVOURITES || resolve(&i))
		quick[quick_count++] = i;
}
static bool register_playlist(const char *name, void *u) {
	(void)u;
	collection_item_t r;
	if (!collections_get(COLLECTION_PLAYLIST, name, &r))
		collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
	return true;
}
static item_t mixes[4];
static int mix_count;
static bool genre(const char *name, const char *path, const char *artist, void *u) {
	(void)path;
	(void)artist;
	(void)u;
	item_t i = {0};
	i.context.spec.kind = LIBRARY_LIST_TRACKS;
	i.context.spec.filter = LIBRARY_FILTER_GENRE;
	snprintf(i.context.spec.value, sizeof(i.context.spec.value), "%s", name);
	snprintf(i.context.title, sizeof(i.context.title), "%.240s Mix", name);
	if (mix_count < 4)
		mixes[mix_count++] = i;
	return mix_count < 4;
}
static void rebuild(void) { homeview_build(root, config); }
static void scope_cb(lv_event_t *e) {
	scope = (int)(intptr_t)lv_event_get_user_data(e);
	config_set_int("home", "scope", scope);
	config_save();
	rebuild();
}
static void customize(lv_event_t *e);
static void customize_change(lv_event_t *e) {
	intptr_t v = (intptr_t)lv_event_get_user_data(e);
	int pos = v / 2;
	char key[32];
	if (v % 2) {
		if (pos > 0) {
			int n = order[pos];
			order[pos] = order[pos - 1];
			order[pos - 1] = n;
		}
	} else {
		snprintf(key, sizeof(key), "hide_%d", order[pos]);
		config_set_bool("home", key, !config_get_bool("home", key, false));
	}
	for (int j = 0; j < 6; j++) {
		snprintf(key, sizeof(key), "order_%d", j);
		config_set_int("home", key, order[j]);
	}
	config_save();
	rebuild();
	customize(NULL);
}
static void customize(lv_event_t *e) {
	(void)e;
	lv_obj_t *b = bottomsheet_open(530, NULL);
	lv_obj_set_flex_flow(b, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_all(b, 16, 0);
	int w = config->screen_width - 32;
	text(b, "Customize Home", w, &font_ui_24_bold, false);
	for (int j = 0; j < 6; j++) {
		char key[32], title[80];
		snprintf(key, sizeof(key), "hide_%d", order[j]);
		snprintf(title, sizeof(title), "%s  %s", config_get_bool("home", key, false) ? "+" : "-", sections[order[j]]);
		lv_obj_t *r = box(b, w, 64);
		button(r, title, w - 80, 56, customize_change, (void *)(intptr_t)(j * 2));
		if (j) {
			lv_obj_t *up = button(r, "Up", 72, 56, customize_change, (void *)(intptr_t)(j * 2 + 1));
			lv_obj_align(up, LV_ALIGN_RIGHT_MID, 0, 0);
		}
	}
}
static void podcast_open_cb(lv_event_t *e) {
	(void)e;
	podcastpage_open_followed();
}
void homeview_build(lv_obj_t *parent, gui_config_t *cfg) {
	homeview_art_drop();
	root = parent;
	config = cfg;
	item_count = 0;
	lv_obj_clean(root);
	scope = config_get_int("home", "scope", 0);
	if (scope < 0 || scope > 2)
		scope = 0;
	collections_init(cfg->sd_root_path);
	playlists_for_each(register_playlist, NULL);
	int w = cfg->screen_width - 40;
	lv_obj_t *filters = box(root, w, 44);
	lv_obj_set_flex_flow(filters, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(filters, 8, 0);
	const char *names[] = {"All", "Music", "Podcasts & Shows"};
	int widths[] = {54, 78, 186};
	for (int j = 0; j < 3; j++) {
		lv_obj_t *b = button(filters, names[j], widths[j], 42, scope_cb, (void *)(intptr_t)j);
		lv_obj_set_style_radius(b, 22, 0);
		lv_obj_set_style_bg_opa(b, 255, 0);
		lv_obj_set_style_bg_color(b, lv_color_hex(scope == j ? 0x1ed760 : 0x282828), 0);
		lv_obj_set_style_text_color(lv_obj_get_child(b, 0), lv_color_hex(scope == j ? 0x111111 : 0xffffff), 0);
	}
	lv_obj_t *edit = button(filters, NULL, 40, 42, customize, NULL);
	icon(edit, &icon_music_settings, 24, 0xb3b3b3);
	collection_item_t saved[24];
	int ns = collections_list(0, COLLECTION_RECENT, 0, saved, 24);
	library_home_context_t recent[12], frequent[12], added[12];
	int nr = library_home_contexts(recent, 12, false), nf = library_home_contexts(frequent, 12, true), na = library_home_added(added, 12);
	bool used[6] = {0};
	for (int j = 0; j < 6; j++) {
		char key[32];
		snprintf(key, sizeof(key), "order_%d", j);
		int n = config_get_int("home", key, j);
		if (n < 0 || n > 5 || used[n]) {
			for (n = 0; n < 6 && used[n]; n++)
				;
		}
		order[j] = n;
		used[n] = true;
	}
	if (scope != 2)
		for (int z = 0; z < 6; z++) {
			int section = order[z];
			char key[32];
			snprintf(key, sizeof(key), "hide_%d", section);
			if (config_get_bool("home", key, false))
				continue;
			if (section == 0) {
				quick_count = 0;
				for (int j = 0; j < ns; j++)
					if (saved[j].kind!=COLLECTION_FOLDER && saved[j].pin && (scope == 0 || saved[j].kind != COLLECTION_PODCAST))
						quick_add(from_saved(saved[j]));
				for (int j = 0; j < nr && j < 3; j++)
					quick_add(from_context(recent[j]));
				for (int j = 0; j < nf && j < 3; j++)
					quick_add(from_context(frequent[j]));
				item_t liked = {0};
				liked.context.spec.kind = LIBRARY_LIST_FAVOURITES;
				snprintf(liked.context.title, sizeof(liked.context.title), "Liked Songs");
				quick_add(liked);
				for (int j = 0; j < na; j++)
					quick_add(from_context(added[j]));
				for (int j = 0; j < ns; j++)
					if (saved[j].kind == COLLECTION_PLAYLIST)
						quick_add(from_saved(saved[j]));
				if (quick_count) {
					lv_obj_t *grid = box(root, w, ((quick_count + 1) / 2) * 84);
					lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
					lv_obj_set_style_pad_column(grid, 8, 0);
					lv_obj_set_style_pad_row(grid, 8, 0);
					for (int j = 0; j < quick_count; j++)
						card(grid, &quick[j], true);
				}
			} else if (section == 1 && nr) {
				lv_obj_t *row = NULL;
				int shown = 0;
				for (int j = 0; j < nr && shown < 6; j++) {
					item_t i = from_context(recent[j]);
					i.resume = true;
					if (!resolve(&i))
						continue;
					if (!row)
						row = shelf(sections[1]);
					card(row, &i, false);
					shown++;
				}
			} else if (section == 2) {
				item_t candidates[24];
				int n = 0;
				for (int j = 0; j < na; j++)
					candidates[n++] = from_context(added[j]);
				for (int j = 0; j < ns && n < 24; j++)
					if (saved[j].kind == COLLECTION_PLAYLIST)
						candidates[n++] = from_saved(saved[j]);
				for (int j = 1; j < n; j++) {
					item_t v = candidates[j];
					int k = j;
					while (k > 0 && candidates[k - 1].context.added < v.context.added) {
						candidates[k] = candidates[k - 1];
						k--;
					}
					candidates[k] = v;
				}
				lv_obj_t *row = NULL;
				int shown = 0;
				for (int j = 0; j < n && shown < 6; j++)
					if (resolve(&candidates[j])) {
						if (!row)
							row = shelf(sections[2]);
						card(row, &candidates[j], false);
						shown++;
					}
			} else if (section == 3 || section == 5) {
				item_t i = {0};
				i.context.spec.kind = LIBRARY_LIST_TRACKS;
				i.context.spec.filter = section == 3 ? LIBRARY_FILTER_HOME_REPEAT : LIBRARY_FILTER_HOME_REDISCOVER;
				snprintf(i.context.title, sizeof(i.context.title), "%s", section == 3 ? "Your repeat mix" : "Rediscover your music");
				if (resolve(&i))
					card(shelf(sections[section]), &i, false);
			} else if (section == 4) {
				library_index_t *ix = library_index_open(LIBRARY_LIST_GENRES, LIBRARY_FILTER_NONE, NULL, LIBRARY_ORDER_DEFAULT, false);
				mix_count = 0;
				if (ix) {
					library_index_window(ix, 0, 4, genre, NULL);
					library_index_close(ix);
				}
				lv_obj_t *row = NULL;
				for (int j = 0; j < mix_count; j++)
					if (resolve(&mixes[j])) {
						if (!row)
							row = shelf(sections[4]);
						card(row, &mixes[j], false);
					}
			}
		}
	if (scope != 1) {
		lv_obj_t *row = NULL;
		for (int j = 0; j < ns; j++)
			if (saved[j].kind == COLLECTION_PODCAST) {
				if (!row)
					row = shelf("Your shows");
				item_t i = from_saved(saved[j]);
				card(row, &i, false);
			}
		if (scope == 2 && !row)
			text(root, "Your followed shows will appear here.", w, &font_ui_20, true);
		if (scope == 2) {
			lv_obj_t *b = button(root, "Browse podcasts & shows", w, 56, podcast_open_cb, NULL);
			(void)b;
		}
	}
	if (!item_count && scope != 2)
		text(root, "Add music to your card to start your Home feed.", w, &font_ui_20, true);
	lv_obj_update_layout(root);
}
