#include "discography.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/toast.h"
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
#include <stdlib.h>
#include <string.h>

/* An artist's releases, the whole catalogue by scrolling (2026-11-05): read
 * PAGE at a time as the list nears its end, up to RELEASES_MAX, with no "More"
 * button. "All" is every release in one list, a heading where the kind
 * changes, as Spotify's discography lists them. Covers are decoded only for
 * the rows on screen, through ART_SLOTS loader slots. */
#define PAGE 12
#define RELEASES_MAX 300
#define ART_SLOTS 12

static lv_obj_t *screen, *heading, *chip_row, *list, *chips[4];
static library_release_t *releases;
static lv_obj_t **rows, **images;
static int capacity, count, selected, shown_category;
static bool at_end, loading;
static unsigned revision;
static char artist_name[256];
static struct {
	int row; // the release the slot holds, -1 for none
	cover_image_t image;
	bool loaded;
} art[ART_SLOTS];
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
	for (int i = 0; i < ART_SLOTS; i++) {
		coverloader_release(COVERLOADER_DISCOGRAPHY_BASE + i);
		if (art[i].row >= 0 && art[i].row < count && images && images[art[i].row])
			lv_image_set_src(images[art[i].row], &icon_album);
		cover_free(&art[i].image);
		art[i].loaded = false;
		art[i].row = -1;
	}
}
static void release_open(lv_event_t *e) {
    if(bottomsheet_active()) return;
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	if (i >= 0 && i < count)
		medialist_open(releases[i].title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, releases[i].value);
}
static void draw(void);
static char editing_release[300];
static void type_selected(lv_event_t *e) {
    const char *type=lv_event_get_user_data(e);
    if(library_release_type_set(editing_release,type)) {bottomsheet_close(true);draw();}
    else toast_error("Could not save the release type");
}
static void release_options(lv_event_t *e) {
    if(lv_indev_active() && lv_indev_get_scroll_obj(lv_indev_active())) return;
    int i=(int)(intptr_t)lv_event_get_user_data(e);
    if(i<0 || i>=count) return;
    snprintf(editing_release,sizeof(editing_release),"%s",releases[i].value);
    lv_obj_t *body=bottomsheet_open(402,NULL);
    int w=lv_display_get_horizontal_resolution(NULL)-40;
    lv_obj_set_style_pad_hor(body,20,0);
    lv_obj_set_style_pad_row(body,8,0);
    lv_obj_set_flex_flow(body,LV_FLEX_FLOW_COLUMN);
    label(body,"Release type",w,&font_ui_24_bold,0xffffff);
    label(body,releases[i].title,w,&font_ui_18,0xb3b3b3);
    const char *names[]={"Automatic","Album","Single","EP"};
    const char *values[]={"","Album","Single","EP"};
    for(int n=0;n<4;n++) {
        lv_obj_t *b=button(body,w,52,type_selected,(void *)values[n]);
        lv_obj_t *l=label(b,names[n],w-16,&font_ui_20,0xffffff);lv_obj_align(l,LV_ALIGN_LEFT_MID,8,0);
    }
}
static void filter_cb(lv_event_t *e) {
	selected = (int)(intptr_t)lv_event_get_user_data(e);
	draw();
}
static bool grow(int need) {
	if (need <= capacity)
		return true;
	int cap = capacity ? capacity : PAGE;
	while (cap < need)
		cap *= 2;
	if (cap > RELEASES_MAX)
		cap = RELEASES_MAX;
	if (cap < need)
		return false;
	library_release_t *r = realloc(releases, (size_t)cap * sizeof(*r));
	lv_obj_t **ro = r ? realloc(rows, (size_t)cap * sizeof(*ro)) : NULL;
	if (r)
		releases = r;
	if (ro)
		rows = ro;
	lv_obj_t **im = ro ? realloc(images, (size_t)cap * sizeof(*im)) : NULL;
	if (im)
		images = im;
	if (!r || !ro || !im)
		return false;
	capacity = cap;
	return true;
}
// The next page of releases, as rows at the end of the list.
static void load_page(void) {
	if (at_end || loading)
		return;
	loading = true;
	int w = lv_obj_get_width(list);
	if (w <= 0)
		w = lv_display_get_horizontal_resolution(NULL) - 32;
	if (!grow(count + PAGE)) {
		at_end = true;
		loading = false;
		return;
	}
	int n = library_discography(artist_name, selected, count, PAGE, releases + count);
	if (n < PAGE)
		at_end = true;
	for (int i = count; i < count + n; i++) {
		library_release_t *r = &releases[i];
		// In All, a heading where the kind changes; it filters to that kind.
		if (!selected && r->category != shown_category) {
			shown_category = r->category;
			lv_obj_t *header = button(list, w, 40, filter_cb, (void *)(intptr_t)r->category);
			if (r->category == 4)
				lv_obj_set_clickable(header, false);
			lv_obj_t *l = label(header, r->category == 4 ? "Other releases" : categories[r->category], w,
								&font_ui_22_bold, 0xffffff);
			lv_obj_align(l, LV_ALIGN_BOTTOM_LEFT, 0, 0);
		}
		lv_obj_t *row = button(list, w, 86, release_open, (void *)(intptr_t)i);
		rows[i] = row;
        lv_obj_add_event_cb(row,release_options,LV_EVENT_LONG_PRESSED,(void *)(intptr_t)i);
		images[i] = lv_image_create(row);
		lv_image_set_src(images[i], &icon_album);
		lv_obj_set_size(images[i], 66, 66);
		lv_image_set_inner_align(images[i], LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_pos(images[i], 0, 8);
		lv_obj_t *l = label(row, r->title, w - 88, &font_ui_22, 0xffffff);
		lv_obj_set_pos(l, 82, 10);
		lv_obj_set_height(l, 30);
		char sub[340];
		const char *year = strcmp(r->date, "0000") ? r->date : "";
		snprintf(sub, sizeof(sub), "%.4s%s%s", year, *year ? " \xc2\xb7 " : "",
				 !strcmp(r->type, "unclassified-credit") ? "Credited artist / owner unknown"
				 : r->category == 3						  ? r->artist
				 : !strcmp(r->type,"Release") ? "Type not tagged"
                 : r->type[0]							  ? r->type
														  : "Release");
		l = label(row, sub, w - 88, &font_ui_16, 0xb3b3b3);
		lv_obj_set_pos(l, 82, 46);
		lv_obj_set_height(l, 24);
	}
	count += n;
	if (count == 0)
		label(list, "No local releases", w, &font_ui_18, 0xb3b3b3);
	loading = false;
}
// A screen from the end, the next page.
static void scrolled(lv_event_t *e) {
	(void)e;
	if (!at_end && lv_obj_get_scroll_bottom(list) < lv_obj_get_height(list))
		load_page();
}
static void draw(void) {
	revision = library_revision(LIBRARY_LIST_ALBUMS);
	art_drop();
	lv_obj_clean(list);
	lv_obj_scroll_to_y(list, 0, LV_ANIM_OFF);
	for (int i = 0; i < 4; i++) {
		lv_obj_set_style_bg_color(chips[i], lv_color_hex(i == selected ? 0xf4f4f4 : 0x121212), 0);
		lv_obj_set_style_text_color(lv_obj_get_child(chips[i], 0), lv_color_hex(i == selected ? 0x121212 : 0xeeeeee), 0);
	}
	lv_obj_scroll_to_view(chips[selected], LV_ANIM_OFF);
	count = 0;
	at_end = false;
	shown_category = -1;
	load_page();
	// Enough to fill the screen and give it something to scroll into.
	lv_obj_update_layout(list);
	while (!at_end && lv_obj_get_scroll_bottom(list) < lv_obj_get_height(list))
		load_page();
}
static void art_update(void) {
	lv_area_t view;
	lv_obj_get_coords(list, &view);
	for (int s = 0; s < ART_SLOTS; s++) {
		int r = art[s].row;
		if (r < 0)
			continue;
		lv_area_t box;
		lv_obj_get_coords(rows[r], &box);
		if (box.y2 < view.y1 || box.y1 > view.y2) {
			coverloader_release(COVERLOADER_DISCOGRAPHY_BASE + s);
			if (art[s].loaded) {
				lv_image_set_src(images[r], &icon_album);
				cover_free(&art[s].image);
			}
			art[s].loaded = false;
			art[s].row = -1;
		} else if (!art[s].loaded) {
			bool done = false;
			if (coverloader_take(COVERLOADER_DISCOGRAPHY_BASE + s, &art[s].image, &done) && done) {
				art[s].loaded = true;
				lv_image_set_src(images[r], &art[s].image.dsc);
			}
		}
	}
	for (int r = 0; r < count; r++) {
		lv_area_t box;
		lv_obj_get_coords(rows[r], &box);
		if (box.y2 < view.y1 || box.y1 > view.y2)
			continue;
		int free_slot = -1;
		bool held = false;
		for (int s = 0; s < ART_SLOTS && !held; s++) {
			if (art[s].row == r)
				held = true;
			else if (art[s].row < 0 && free_slot < 0)
				free_slot = s;
		}
		if (held || free_slot < 0)
			continue;
		art[free_slot].row = r;
		art[free_slot].loaded = false;
		coverloader_request(COVERLOADER_DISCOGRAPHY_BASE + free_slot, releases[r].path, 66);
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
	art_update();
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
	collections_touch(COLLECTION_ARTIST, artist);
	selected = 0;
	if (!screen) {
		for (int i = 0; i < ART_SLOTS; i++)
			art[i].row = -1;
		int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
		screen = lv_obj_create(NULL);
		lv_obj_remove_style_all(screen);
		lv_obj_set_size(screen, w, h);
		// The page itself never moves: the chips scroll sideways in their own
		// row and the releases up and down in theirs. A chip row wider than
		// the screen used to drag the whole page along (2026-11-05).
		lv_obj_set_scrollable(screen, false);
		lv_obj_set_style_bg_color(screen, lv_color_hex(0x121212), 0);
		lv_obj_set_style_bg_opa(screen, 255, 0);
		heading = label(screen, "", w - 94, &font_ui_24_bold, 0xffffff);
		lv_obj_set_pos(heading, 76, 70);
		lv_obj_set_height(heading, 34);
		chip_row = lv_obj_create(screen);
		lv_obj_remove_style_all(chip_row);
		lv_obj_set_pos(chip_row, 0, 128);
		lv_obj_set_size(chip_row, w, 42);
		lv_obj_set_flex_flow(chip_row, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_pad_hor(chip_row, 16, 0);
		lv_obj_set_style_pad_column(chip_row, 6, 0);
		lv_obj_set_scroll_dir(chip_row, LV_DIR_HOR);
		lv_obj_set_scrollbar_mode(chip_row, LV_SCROLLBAR_MODE_OFF);
		int widths[] = {58, 90, 158, 126};
		for (int i = 0; i < 4; i++) {
			chips[i] = button(chip_row, widths[i], 42, filter_cb, (void *)(intptr_t)i);
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
		lv_obj_add_event_cb(list, scrolled, LV_EVENT_SCROLL, NULL);
		lv_obj_add_event_cb(screen, loaded, LV_EVENT_SCREEN_LOADED, NULL);
		lv_obj_add_event_cb(screen, unloaded, LV_EVENT_SCREEN_UNLOADED, NULL);
		lv_timer_create(tick, 120, NULL);
	}
	lv_label_set_text(heading, artist_name);
	switch_screen(screen);
}
