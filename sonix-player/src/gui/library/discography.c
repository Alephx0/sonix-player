#include "discography.h"
#include "src/system/library/collections.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/library/medialist.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/main_menu.h"
#include "src/gui/shell/switcher.h"
#include "src/system/library/library.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static lv_obj_t *screen, *heading, *list, *chips[4], *images[12];
static cover_image_t covers[12];
static library_release_t releases[12];
static int selected, page, count;
static unsigned revision;
static char artist_name[256];
static const char *categories[] = {"All", "Albums", "Singles & EPs", "Featured On"};
static lv_obj_t *label(lv_obj_t *p, const char *s, int w, const lv_font_t *font, uint32_t color) {
	lv_obj_t *l = lv_label_create(p);
	lv_label_set_text(l, s);
	lv_obj_set_width(l, w);
	lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
	lv_obj_set_style_text_font(l, font, 0);
	lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
	return l;
}
static lv_obj_t *button(lv_obj_t *p, int w, int h, lv_event_cb_t cb, void *u) {
	lv_obj_t *b = lv_obj_create(p);
	lv_obj_remove_style_all(b);
	lv_obj_set_size(b, w, h);
	lv_obj_set_clickable(b, true);
	lv_obj_set_scrollable(b, false);
	lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, u);
	return b;
}
static void art_drop(void) {
	for (int i = 0; i < 12; i++) {
		coverloader_release(COVERLOADER_DISCOGRAPHY_BASE + i);
		if (images[i])
			lv_image_set_src(images[i], &icon_album);
		cover_free(&covers[i]);
		images[i] = NULL;
	}
}
static void release_open(lv_event_t *e) {
	library_release_t *r = lv_event_get_user_data(e);
	medialist_open(r->title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, r->value);
}
static void draw(void);
static void page_cb(lv_event_t *e) {
	page += (int)(intptr_t)lv_event_get_user_data(e);
	if (page < 0)
		page = 0;
	draw();
}
static void filter_cb(lv_event_t *e) {
	selected = (int)(intptr_t)lv_event_get_user_data(e);
	page = 0;
	draw();
}
static void draw(void) {
	revision = library_revision(LIBRARY_LIST_ALBUMS);
	art_drop();
	lv_obj_clean(list);
	lv_obj_scroll_to_y(list, 0, LV_ANIM_OFF);
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	for (int i = 0; i < 4; i++) {
		lv_obj_set_style_bg_color(chips[i], lv_color_hex(i == selected ? 0xf4f4f4 : 0x121212), 0);
		lv_obj_set_style_text_color(lv_obj_get_child(chips[i], 0), lv_color_hex(i == selected ? 0x121212 : 0xeeeeee), 0);
	}
	count = 0;
	int first = selected ? selected : 1, last = selected ? selected : 3;
	for (int category = first; category <= last; category++) {
		lv_obj_t *header = button(list, w, 34, filter_cb, (void *)(intptr_t)category);
		label(header, categories[category], w - 100, &font_ui_22_bold, 0xffffff);
		if (!selected) {
			lv_obj_t *more = label(header, "See all", 92, &font_ui_16, 0xb3b3b3);
			lv_obj_align(more, LV_ALIGN_RIGHT_MID, 0, 0);
		}
		int start = count;
		count += library_discography(artist_name, category, selected ? page * 12 : 0, selected ? 12 : 4, releases + count);
		if (count == start)
			label(list, "No local releases", w, &font_ui_18, 0xb3b3b3);
		for (int i = start; i < count; i++) {
			library_release_t *r = &releases[i];
			lv_obj_t *row = button(list, w, 86, release_open, r);
			images[i] = lv_image_create(row);
			lv_image_set_src(images[i], &icon_album);
			lv_obj_set_size(images[i], 66, 66);
			lv_image_set_inner_align(images[i], LV_IMAGE_ALIGN_CONTAIN);
			lv_obj_set_pos(images[i], 0, 8);
			coverloader_request(COVERLOADER_DISCOGRAPHY_BASE + i, r->path, 66);
			lv_obj_t *l = label(row, r->title, w - 88, &font_ui_22, 0xffffff);
			lv_obj_set_pos(l, 82, 10);
			lv_obj_set_height(l, 30);
			char sub[340];
			const char *year = strcmp(r->date, "0000") ? r->date : "";
			snprintf(sub, sizeof(sub), "%.4s%s%s", year, *year ? " \xc2\xb7 " : "", !strcmp(r->type,"unclassified-credit") ? "Credited artist / owner unknown" : r->category == 3 ? r->artist : r->type[0] ? r->type : "Release");
			l = label(row, sub, w - 88, &font_ui_16, 0xb3b3b3);
			lv_obj_set_pos(l, 82, 46);
			lv_obj_set_height(l, 24);
		}
	}
	if (page) {
		lv_obj_t *b = button(list, w, 48, page_cb, (void *)(intptr_t)-1);
		lv_obj_center(label(b, "Previous releases", w, &font_ui_18, 0xffffff));
	}
	if (selected && count == 12) {
		lv_obj_t *b = button(list, w, 48, page_cb, (void *)(intptr_t)1);
		lv_obj_center(label(b, "More releases", w, &font_ui_18, 0xffffff));
	}
}
static void tick(lv_timer_t *t) {
	(void)t;
	if (lv_screen_active() != screen)
		return;
	if (revision != library_revision(LIBRARY_LIST_ALBUMS)) {
		draw();
		return;
	}
	for (int i = 0; i < count; i++)
		if (images[i] && !covers[i].pixels) {
			bool done = false;
			if (coverloader_take(COVERLOADER_DISCOGRAPHY_BASE + i, &covers[i], &done))
				lv_image_set_src(images[i], &covers[i].dsc);
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
	art_drop();
}
void discography_open(const char *artist) {
	if (!artist || !*artist)
		return;
	snprintf(artist_name, sizeof(artist_name), "%s", artist);
    collections_touch(COLLECTION_ARTIST,artist);
	selected = page = 0;
	if (!screen) {
		int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
		screen = lv_obj_create(NULL);
		lv_obj_remove_style_all(screen);
		lv_obj_set_size(screen, w, h);
		lv_obj_set_style_bg_color(screen, lv_color_hex(0x121212), 0);
		lv_obj_set_style_bg_opa(screen, 255, 0);
		heading = label(screen, "", w - 94, &font_ui_24_bold, 0xffffff);
		lv_obj_set_pos(heading, 76, 70);
		lv_obj_set_height(heading, 34);
		int widths[] = {58, 90, 158, 126}, x = 16;
		for (int i = 0; i < 4; i++) {
			chips[i] = button(screen, widths[i], 42, filter_cb, (void *)(intptr_t)i);
			lv_obj_set_pos(chips[i], x, 128);
			x += widths[i] + 4;
			lv_obj_set_style_radius(chips[i], 22, 0);
			lv_obj_set_style_bg_opa(chips[i], 255, 0);
			lv_obj_set_style_border_width(chips[i], 1, 0);
			lv_obj_set_style_border_color(chips[i], lv_color_hex(0x555555), 0);
			lv_obj_t *l = label(chips[i], categories[i], widths[i] - 8, &font_ui_16, 0xffffff);
			lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
			lv_obj_center(l);
		}
		list = lv_obj_create(screen);
		lv_obj_remove_style_all(list);
		lv_obj_set_pos(list, 16, 188);
		lv_obj_set_size(list, w - 32, h - 336);
		lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_scroll_dir(list, LV_DIR_VER);
		lv_obj_set_style_pad_row(list, 8, 0);
		lv_obj_add_event_cb(screen, loaded, LV_EVENT_SCREEN_LOADED, NULL);
		lv_obj_add_event_cb(screen, unloaded, LV_EVENT_SCREEN_UNLOADED, NULL);
		lv_timer_create(tick, 120, NULL);
	}
	lv_label_set_text(heading, artist_name);
	switch_screen(screen);
}
