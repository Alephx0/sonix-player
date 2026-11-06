#include "activity.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "src/gui/fonts/fonts.h"
#include "src/gui/library/medialist.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/nowplaying/player.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/rowswipe.h"
#include "src/gui/shell/settingsrow.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/gui/shell/toast.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"

/* Recents and Listening stats: the two personal pages Spotify keeps behind the
 * profile picture, built from the local play history. Both are pushed pages in
 * the settings scaffold (title, back, mini-player and navigation) and are
 * rebuilt each time they open. Artwork comes from one small pool of loader
 * slots handed to whichever rows are on screen, so a long Recents list never
 * holds more than COVERLOADER_ACTIVITY_COUNT pictures. */

#define ROWS_MAX 100
#define HISTORY_SCAN 320 /* history entries read for Recents, newest first */
#define TOP_SONGS 10
#define TOP_ARTISTS 10
#define QUIET 0xb3b3b3

typedef struct {
	lv_obj_t *img, *glyph;
	char path[512];
	int slot;
} art_row_t;

static gui_config_t *cfg;
static lv_obj_t *screen[4], *list[4]; /* Recents, Listening stats, a week, Composers */
static int showing = -1;
static art_row_t rows[ROWS_MAX];
static int row_count;
static int slot_owner[COVERLOADER_ACTIVITY_COUNT];
static cover_image_t covers[COVERLOADER_ACTIVITY_COUNT];
static lv_timer_t *art_timer;
/* Top songs play as one list, in rank order. */
static char top_paths[TOP_SONGS][512];
static int top_count;
static char artist_names[TOP_ARTISTS][256];

static bool tapped(void) { return !player_sheet_drag_active() && !switcher_back_drag_active(); }

static lv_obj_t *text(lv_obj_t *parent, const char *s, const lv_font_t *font, uint32_t color, int w) {
	lv_obj_t *l = lv_label_create(parent);
	lv_label_set_text(l, s);
	lv_obj_set_style_text_font(l, font, 0);
	lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
	if (w > 0) {
		lv_obj_set_size(l, w, lv_font_get_line_height(font));
		lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
	}
	return l;
}

static lv_obj_t *heading(lv_obj_t *parent, const char *s) {
	lv_obj_t *l = text(parent, s, &font_ui_22_bold, 0xffffff, lv_obj_get_content_width(parent));
	lv_obj_set_height(l, LV_SIZE_CONTENT); /* the padding is inside it */
	lv_obj_set_style_pad_top(l, 18, 0);
	lv_obj_set_style_pad_bottom(l, 6, 0);
	return l;
}

/* ---- artwork pool ---------------------------------------------------- */

static void art_reset(art_row_t *r) {
	if (r->img) lv_image_set_src(r->img, NULL);
	if (r->glyph) lv_obj_set_hidden(r->glyph, false);
	r->slot = -1;
}

static void slot_free(int k) {
	coverloader_release(COVERLOADER_ACTIVITY_BASE + k);
	cover_free(&covers[k]);
	if (slot_owner[k] >= 0 && slot_owner[k] < row_count) art_reset(&rows[slot_owner[k]]);
	slot_owner[k] = -1;
}

static void art_drop(void) {
	for (int k = 0; k < COVERLOADER_ACTIVITY_COUNT; k++) slot_free(k);
}

static bool on_screen(lv_obj_t *o) {
	if (!lv_obj_is_visible(o)) return false; /* a folded Recents group */
	lv_area_t a;
	lv_obj_get_coords(o, &a);
	return a.y2 >= 0 && a.y1 < cfg->screen_height && a.x2 >= 0 && a.x1 < cfg->screen_width;
}

static void art_tick(lv_timer_t *t) {
	(void)t;
	if (showing < 0 || lv_screen_active() != screen[showing]) return;
	for (int k = 0; k < COVERLOADER_ACTIVITY_COUNT; k++) {
		int owner = slot_owner[k];
		if (owner < 0 || covers[k].pixels) continue;
		bool done = false;
		if (coverloader_take(COVERLOADER_ACTIVITY_BASE + k, &covers[k], &done)) {
			lv_image_set_src(rows[owner].img, &covers[k].dsc);
			if (rows[owner].glyph) lv_obj_set_hidden(rows[owner].glyph, true);
		}
	}
	/* Rows that came into view take a slot, from the pool's free ones first
	 * and then from rows that scrolled away. */
	for (int i = 0; i < row_count; i++) {
		art_row_t *r = &rows[i];
		if (r->slot >= 0 || !r->path[0] || !on_screen(r->img)) continue;
		int k = -1;
		for (int j = 0; j < COVERLOADER_ACTIVITY_COUNT && k < 0; j++)
			if (slot_owner[j] < 0) k = j;
		for (int j = 0; j < COVERLOADER_ACTIVITY_COUNT && k < 0; j++)
			if (!on_screen(rows[slot_owner[j]].img)) k = j;
		if (k < 0) break;
		slot_free(k);
		slot_owner[k] = i;
		r->slot = k;
		lv_area_t a;
		lv_obj_get_coords(r->img, &a);
		coverloader_request(COVERLOADER_ACTIVITY_BASE + k, r->path, lv_area_get_width(&a));
	}
}

static art_row_t *art(lv_obj_t *parent, const char *path, int size, bool round) {
	if (row_count >= ROWS_MAX) return NULL;
	art_row_t *r = &rows[row_count++];
	memset(r, 0, sizeof(*r));
	r->slot = -1;
	snprintf(r->path, sizeof(r->path), "%s", path ? path : "");
	r->img = lv_image_create(parent);
	lv_obj_set_size(r->img, size, size);
	lv_image_set_inner_align(r->img, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_set_style_bg_color(r->img, lv_color_hex(0x282828), 0);
	lv_obj_set_style_bg_opa(r->img, LV_OPA_COVER, 0);
	lv_obj_set_style_radius(r->img, round ? LV_RADIUS_CIRCLE : 4, 0);
	lv_obj_set_style_clip_corner(r->img, true, 0);
	lv_obj_set_clickable(r->img, false);
	r->glyph = lv_image_create(r->img);
	lv_image_set_src(r->glyph, round ? &icon_artist : &icon_music_note);
	lv_obj_set_size(r->glyph, size * 2 / 5, size * 2 / 5);
	lv_image_set_inner_align(r->glyph, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_set_style_image_recolor(r->glyph, lv_color_hex(QUIET), 0);
	lv_obj_set_style_image_recolor_opa(r->glyph, LV_OPA_COVER, 0);
	lv_obj_center(r->glyph);
	return r;
}

/* ---- song rows ------------------------------------------------------- */

/* A song from Recents continues in its own album, as a searched song does;
 * without an indexed album it plays alone. Playback stays in preview. */
static void play_song(const char *path) {
	char album[256];
	library_index_t *ix = NULL;
	int start = -1;
	if (library_track_album_value(path, album, sizeof(album)) && album[0]) {
		ix = library_index_open(LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, album, LIBRARY_ORDER_DEFAULT, false);
		if (ix) start = library_index_find_path(ix, path);
	}
	if (ix && start >= 0) {
		if (device_state_play_index(ix, start)) {
			playlist_set_album(album);
			player_refresh_now_playing();
		}
	} else {
		if (ix) library_index_close(ix);
		player_play_file(path);
	}
}

static void song_clicked_cb(lv_event_t *e) {
	lv_obj_t *row = lv_event_get_current_target(e);
	if (!tapped() || rowswipe_was_swiped(row)) return;
	if (!gui_card_available()) {
		gui_notify_no_card();
		return;
	}
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	if (i < 0) {
		/* A top song: the ranked list plays as one context. */
		int start = -i - 1;
		const char *paths[TOP_SONGS];
		for (int k = 0; k < top_count; k++) paths[k] = top_paths[k];
		device_state_play_list(paths, top_count, start);
		playlist_set_context("", "Your top songs");
		player_refresh_now_playing();
	} else if (i < row_count) {
		play_song(rows[i].path);
	}
}

/* Swipe right to queue, the shared strip of every song list. */
static void song_swipe_cb(lv_event_t *e) {
	lv_obj_t *row = lv_event_get_current_target(e);
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	const char *path = i < 0 ? top_paths[-i - 1] : (i < row_count ? rows[i].path : "");
	static const rowswipe_spec_t spec = {&icon_list_plus, ROWSWIPE_GREEN, NULL, 0};
	int committed = 0;
	if (rowswipe_event(e, row, &spec, &committed)) lv_event_stop_bubbling(e);
	if (committed > 0 && path[0] && playlist_priority_add(path)) {
		device_state_queue_changed();
		toast_glyph(&icon_list_music, "added_to_the_queue");
	}
}

static lv_obj_t *song_row(lv_obj_t *parent, const char *path, const char *title, const char *sub, int rank,
						  intptr_t user) {
	int w = lv_obj_get_content_width(parent);
	lv_obj_t *row = lv_obj_create(parent);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, w, 72);
	lv_obj_set_style_bg_color(row, lv_color_hex(0x1f1f1f), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
	lv_obj_set_style_radius(row, 6, 0);
	lv_obj_set_clickable(row, true);
	lv_obj_set_scrollable(row, false);
	lv_obj_set_event_bubble(row, true);
	int x = 0;
	if (rank > 0) {
		char n[8];
		snprintf(n, sizeof(n), "%d", rank);
		lv_obj_t *num = text(row, n, &font_ui_20, 0xffffff, 0);
		lv_obj_set_width(num, 30);
		lv_obj_set_style_text_align(num, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_align(num, LV_ALIGN_LEFT_MID, 0, 0);
		x = 40;
	}
	art_row_t *r = art(row, path, 56, false);
	if (r) lv_obj_set_pos(r->img, x, 8);
	lv_obj_t *t = text(row, title, &font_ui_20, 0xffffff, w - x - 72);
	lv_obj_set_pos(t, x + 70, 10);
	lv_obj_t *s = text(row, sub, &font_ui_16, QUIET, w - x - 72);
	lv_obj_set_pos(s, x + 70, 40);
	lv_obj_add_event_cb(row, song_clicked_cb, LV_EVENT_CLICKED, (void *)user);
	lv_obj_add_event_cb(row, song_swipe_cb, LV_EVENT_PRESSED, (void *)user);
	lv_obj_add_event_cb(row, song_swipe_cb, LV_EVENT_PRESSING, (void *)user);
	lv_obj_add_event_cb(row, song_swipe_cb, LV_EVENT_RELEASED, (void *)user);
	lv_obj_add_event_cb(row, song_swipe_cb, LV_EVENT_PRESS_LOST, (void *)user);
	return row;
}

static void empty_note(lv_obj_t *parent, const char *title, const char *body) {
	int w = lv_obj_get_content_width(parent);
	lv_obj_t *t = text(parent, title, &font_ui_24_bold, 0xffffff, 0);
	lv_obj_set_width(t, w);
	lv_label_set_long_mode(t, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_pad_top(t, 120, 0);
	lv_obj_t *b = text(parent, body, &font_ui_18, QUIET, 0);
	lv_obj_set_width(b, w);
	lv_label_set_long_mode(b, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(b, LV_TEXT_ALIGN_CENTER, 0);
}

/* ---- pages ----------------------------------------------------------- */

static void page_unloaded_cb(lv_event_t *e) {
	(void)e;
	art_drop();
}

static void page_begin(int which, const char *title) {
	if (!screen[which]) {
		screen[which] = lv_obj_create(NULL);
		list[which] = settingsrow_page(screen[which], cfg, title);
		settingsrow_title_corner_slots(settingsrow_page_title(screen[which]), cfg, 0);
		lv_obj_set_style_pad_gap(list[which], 0, 0);
		lv_obj_set_style_pad_bottom(list[which], 24, 0);
		lv_obj_add_event_cb(screen[which], page_unloaded_cb, LV_EVENT_SCREEN_UNLOADED, NULL);
		lv_obj_update_layout(screen[which]); /* rows size themselves from the list's width */
	}
	art_drop();
	row_count = 0;
	lv_obj_clean(list[which]);
	lv_obj_scroll_to_y(list[which], 0, LV_ANIM_OFF);
	showing = which;
	if (!art_timer) art_timer = lv_timer_create(art_tick, 120, NULL);
}

static void day_name(long long at, char *out, size_t size) {
	if (at < 1577836800LL) { /* before 2020: the clock was not set */
		snprintf(out, size, "Earlier");
		return;
	}
	static const char *const MONTHS[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
										 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	static const char *const DAYS[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	time_t now_t = time(NULL), at_t = (time_t)at;
	struct tm now, then;
	localtime_r(&now_t, &now);
	localtime_r(&at_t, &then);
	struct tm a = now, b = then;
	a.tm_hour = b.tm_hour = 12;
	a.tm_min = b.tm_min = a.tm_sec = b.tm_sec = 0;
	long days = (long)((mktime(&a) - mktime(&b)) / 86400);
	if (days <= 0) snprintf(out, size, "Today");
	else if (days == 1) snprintf(out, size, "Yesterday");
	else if (days < 7) snprintf(out, size, "%s", DAYS[then.tm_wday]);
	else if (then.tm_year == now.tm_year) snprintf(out, size, "%s %d", MONTHS[then.tm_mon], then.tm_mday);
	else snprintf(out, size, "%s %d, %d", MONTHS[then.tm_mon], then.tm_mday, then.tm_year + 1900);
}

/* Recents (Spotify iOS, captured 2026-11-04): under each day, what was
 * played from one place is one row -- its name, "N songs played", its kind --
 * with a round chevron that opens the songs underneath. Songs played from no
 * known place group the same way under "N songs played". */
static void group_toggle_cb(lv_event_t *e) {
	if (!tapped()) return;
	lv_obj_t *songs = lv_event_get_user_data(e);
	lv_obj_t *row = lv_event_get_current_target(e);
	bool open = lv_obj_has_flag(songs, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_hidden(songs, !open);
	lv_obj_t *chev = lv_obj_get_child(row, -1);
	lv_image_set_src(lv_obj_get_child(chev, 0), open ? &icon_chevron_up : &icon_chevron_down);
}

static void kind_word(const char *kind, char *out, size_t size) {
	/* "PLAYLIST" -> "Playlist" */
	size_t n = 0;
	for (; kind[n] && n + 1 < size; n++) out[n] = n ? (char)tolower((unsigned char)kind[n]) : kind[n];
	out[n] = '\0';
}

static lv_obj_t *group_row(lv_obj_t *parent, const char *first_path, const char *title, const char *sub) {
	int w = lv_obj_get_content_width(parent);
	lv_obj_t *row = lv_obj_create(parent);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, w, 84);
	lv_obj_set_style_bg_color(row, lv_color_hex(0x1f1f1f), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
	lv_obj_set_style_radius(row, 6, 0);
	lv_obj_set_clickable(row, true);
	lv_obj_set_scrollable(row, false);
	lv_obj_set_event_bubble(row, true);
	art_row_t *r = art(row, first_path, 72, false);
	if (r) lv_obj_set_pos(r->img, 0, 6);
	lv_obj_t *t = text(row, title, &font_ui_20, 0xffffff, w - 140);
	lv_obj_set_pos(t, 86, 14);
	lv_obj_t *s = text(row, sub, &font_ui_16, QUIET, w - 140);
	lv_obj_set_pos(s, 86, 46);
	/* The round chevron at the right edge; last child, for the toggle. */
	lv_obj_t *chev = lv_obj_create(row);
	lv_obj_remove_style_all(chev);
	lv_obj_set_size(chev, 30, 30);
	lv_obj_set_style_radius(chev, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_border_width(chev, 2, 0);
	lv_obj_set_style_border_color(chev, lv_color_hex(QUIET), 0);
	lv_obj_set_clickable(chev, false);
	lv_obj_align(chev, LV_ALIGN_RIGHT_MID, -6, 0);
	lv_obj_t *ci = lv_image_create(chev);
	lv_image_set_src(ci, &icon_chevron_down);
	lv_obj_set_size(ci, 20, 20);
	lv_image_set_inner_align(ci, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_set_style_image_recolor(ci, lv_color_hex(QUIET), 0);
	lv_obj_set_style_image_recolor_opa(ci, LV_OPA_COVER, 0);
	lv_obj_center(ci);
	return row;
}

#define GROUP_SONGS 64
void activity_open_recents(void) {
	page_begin(0, "Recents");
	lv_obj_t *l = list[0];
	int w = lv_obj_get_content_width(l);
	static library_history_row_t page[64];
	char day[40] = "", shown_day[40] = "";
	/* The group being gathered: consecutive entries of one day and place. */
	static library_history_row_t members[GROUP_SONGS];
	int member_count = 0;
	char gkind[24] = "", gctx[256] = "";
	bool done = false;
	for (int offset = 0; offset < HISTORY_SCAN && !done; offset += 64) {
		int n = library_history_page(offset, 64, page);
		for (int i = 0; i <= n; i++) {
			bool last = i == n;
			if (last && n == 64 && offset + 64 < HISTORY_SCAN) break; /* more to read */
			const library_history_row_t *h = last ? NULL : &page[i];
			char hday[40] = "";
			if (h) day_name(h->played_at, hday, sizeof(hday));
			bool same = h && member_count && !strcmp(hday, day) && !strcmp(h->kind, gkind) &&
						!strcmp(h->context, gctx);
			if (!same && member_count) {
				/* Close the group: its row, then its songs, folded. */
				if (strcmp(day, shown_day)) {
					heading(l, day);
					snprintf(shown_day, sizeof(shown_day), "%s", day);
				}
				char title[300], sub[340], word[24];
				kind_word(gkind, word, sizeof(word));
				const char *played = member_count == 1 ? "song played" : "songs played";
				if (gctx[0]) {
					snprintf(title, sizeof(title), "%s", gctx);
					snprintf(sub, sizeof(sub), "%d %s%s%s", member_count, played, word[0] ? " \xc2\xb7 " : "", word);
				} else {
					snprintf(title, sizeof(title), "%d %s", member_count, played);
					snprintf(sub, sizeof(sub), "%s%s%s", members[0].artist,
							 member_count > 1 && members[1].artist[0] && strcmp(members[1].artist, members[0].artist)
								 ? ", "
								 : "",
							 member_count > 1 && strcmp(members[1].artist, members[0].artist) ? members[1].artist : "");
				}
				lv_obj_t *row = group_row(l, members[0].path, title, sub);
				lv_obj_t *songs = lv_obj_create(l);
				lv_obj_remove_style_all(songs);
				lv_obj_set_size(songs, w, LV_SIZE_CONTENT);
				lv_obj_set_flex_flow(songs, LV_FLEX_FLOW_COLUMN);
				lv_obj_set_style_pad_left(songs, 24, 0);
				lv_obj_set_scrollable(songs, false);
				lv_obj_update_layout(songs); /* the rows size themselves from its width */
				for (int k = 0; k < member_count && row_count < ROWS_MAX; k++) {
					song_row(songs, members[k].path, members[k].name[0] ? members[k].name : members[k].path,
							 members[k].artist, 0, row_count);
				}
				lv_obj_set_hidden(songs, true);
				lv_obj_add_event_cb(row, group_toggle_cb, LV_EVENT_CLICKED, songs);
				member_count = 0;
				if (row_count >= ROWS_MAX) {
					done = true;
					break;
				}
			}
			if (!h) {
				done = true;
				break;
			}
			if (!member_count) {
				snprintf(day, sizeof(day), "%s", hday);
				snprintf(gkind, sizeof(gkind), "%s", h->kind);
				snprintf(gctx, sizeof(gctx), "%s", h->context);
			}
			/* Each song once in its group, newest first. */
			bool dup = false;
			for (int k = 0; k < member_count && !dup; k++) dup = !strcmp(members[k].path, h->path);
			if (!dup && member_count < GROUP_SONGS) members[member_count++] = *h;
		}
		if (n < 64) break;
	}
	if (!row_count) empty_note(l, "Nothing here yet", "Songs you play will show up here.");
	switch_screen(screen[0]);
}

static void artist_clicked_cb(lv_event_t *e) {
	if (!tapped()) return;
	medialist_open_artist(artist_names[(int)(intptr_t)lv_event_get_user_data(e)]);
}

/* ---- Listening stats (Spotify iOS, captured 2026-11-04): "This week" with
 * its dates, tiles for minutes listened, the top artist and the top song,
 * then "Past weeks". Weeks run Monday to Sunday, as Spotify's Nov 2 - 8. A
 * tile or a past week opens that week's ranked artists and songs. ---- */

/* Day numbers are days since 1970 (UTC), as the play counts keep them. */
static int week_start(int day) { return day - (day + 3) % 7; }

static void week_dates(int from, char *out, size_t size) {
	static const char *mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	time_t a = (time_t)from * 86400, b = (time_t)(from + 6) * 86400;
	struct tm ta, tb;
	gmtime_r(&a, &ta);
	gmtime_r(&b, &tb);
	if (ta.tm_mon == tb.tm_mon)
		snprintf(out, size, "%s %d \xe2\x80\x93 %d", mon[ta.tm_mon], ta.tm_mday, tb.tm_mday);
	else
		snprintf(out, size, "%s %d \xe2\x80\x93 %s %d", mon[ta.tm_mon], ta.tm_mday, mon[tb.tm_mon], tb.tm_mday);
}

static int detail_from, detail_to;
static void stats_build(void);
static void week_detail_open(int from, int to);

static void week_clicked_cb(lv_event_t *e) {
	if (!tapped()) return;
	int from = (int)(intptr_t)lv_event_get_user_data(e);
	int today = library_stats_today();
	week_detail_open(from, from + 6 < today ? from + 6 : today);
}

static lv_obj_t *stat_tile(lv_obj_t *parent, const char *label, int w, int from) {
	lv_obj_t *tile = lv_obj_create(parent);
	lv_obj_remove_style_all(tile);
	lv_obj_set_size(tile, w, LV_SIZE_CONTENT);
	lv_obj_set_style_bg_color(tile, lv_color_hex(0x1f1f1f), 0);
	lv_obj_set_style_bg_opa(tile, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(tile, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
	lv_obj_set_style_radius(tile, 8, 0);
	lv_obj_set_style_pad_all(tile, 12, 0);
	lv_obj_set_style_pad_row(tile, 6, 0);
	lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_scrollable(tile, false);
	lv_obj_set_clickable(tile, true);
	lv_obj_set_event_bubble(tile, true);
	lv_obj_add_event_cb(tile, week_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)from);
	lv_obj_t *head = lv_obj_create(tile);
	lv_obj_remove_style_all(head);
	lv_obj_set_size(head, w - 24, 24);
	lv_obj_set_scrollable(head, false);
	lv_obj_set_clickable(head, false);
	lv_obj_t *t = text(head, label, &font_ui_16, QUIET, w - 56);
	lv_obj_align(t, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_t *chev = lv_image_create(head);
	lv_image_set_src(chev, &icon_chevron_right);
	lv_obj_set_style_image_recolor(chev, lv_color_hex(QUIET), 0);
	lv_obj_set_style_image_recolor_opa(chev, LV_OPA_COVER, 0);
	lv_obj_set_size(chev, 20, 20);
	lv_image_set_inner_align(chev, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_align(chev, LV_ALIGN_RIGHT_MID, 0, 0);
	return tile;
}

static void tile_art(lv_obj_t *tile, const char *name, const char *path, int w, bool round) {
	lv_obj_t *n = text(tile, name, &font_ui_22_bold, 0xffffff, w - 24);
	lv_label_set_long_mode(n, LV_LABEL_LONG_DOT);
	art_row_t *r = art(tile, path, w - 24, round);
	(void)r;
}

static void stats_loaded_cb(lv_event_t *e) {
	(void)e;
	/* Back from a week's page: the artwork pool went with it. */
	if (showing != 1) stats_build();
}

static void stats_build(void) {
	page_begin(1, "Listening stats");
	static bool hooked;
	if (!hooked) {
		lv_obj_add_event_cb(screen[1], stats_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
		hooked = true;
	}
	lv_obj_t *l = list[1];
	int w = lv_obj_get_content_width(l);
	int today = library_stats_today();
	int from = week_start(today);
	char dates[48];
	week_dates(from, dates, sizeof(dates));
	lv_obj_t *t = text(l, "This week", &font_ui_24_bold, 0xffffff, w);
	lv_obj_set_height(t, LV_SIZE_CONTENT);
	lv_obj_set_style_pad_top(t, 4, 0);
	lv_obj_t *d = text(l, dates, &font_ui_18, 0xffffff, w);
	lv_obj_set_height(d, LV_SIZE_CONTENT);
	lv_obj_set_style_pad_bottom(d, 14, 0);

	int plays = 0;
	library_stats_range_totals(from, today, &plays, NULL, NULL);
	int past[3], past_count = 0;
	for (int k = 1; k <= 3; k++) {
		int p = 0;
		library_stats_range_totals(from - 7 * k, from - 7 * k + 6, &p, NULL, NULL);
		if (p) past[past_count++] = from - 7 * k;
	}
	if (!plays && !past_count) {
		empty_note(l, "No stats yet", "Play some music and your top artists and songs will show up here.");
		return;
	}

	/* Two columns of tiles, each as tall as what it holds. */
	int cw = (w - 10) / 2;
	lv_obj_t *cols = lv_obj_create(l);
	lv_obj_remove_style_all(cols);
	lv_obj_set_size(cols, w, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(cols, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(cols, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_column(cols, 10, 0);
	lv_obj_set_scrollable(cols, false);
	lv_obj_t *left = lv_obj_create(cols), *right = lv_obj_create(cols);
	lv_obj_t *col[2] = {left, right};
	for (int k = 0; k < 2; k++) {
		lv_obj_remove_style_all(col[k]);
		lv_obj_set_size(col[k], cw, LV_SIZE_CONTENT);
		lv_obj_set_flex_flow(col[k], LV_FLEX_FLOW_COLUMN);
		lv_obj_set_style_pad_row(col[k], 10, 0);
		lv_obj_set_scrollable(col[k], false);
	}
	lv_obj_t *mt = stat_tile(left, "Minutes listened", cw, from);
	char mins[16];
	snprintf(mins, sizeof(mins), "%d", library_listen_minutes(from, today));
	text(mt, mins, &font_ui_64_bold, 0xffffff, cw - 24);
	static library_stat_row_t top[TOP_SONGS > TOP_ARTISTS ? TOP_SONGS : TOP_ARTISTS];
	if (library_stats_range_top_artists(from, today, top, 1)) {
		snprintf(artist_names[0], sizeof(artist_names[0]), "%s", top[0].name);
		tile_art(stat_tile(left, "Top artist", cw, from), top[0].name, top[0].path, cw, true);
	}
	if (library_stats_range_top_songs(from, today, top, 1)) {
		tile_art(stat_tile(right, "Top song", cw, from), top[0].name[0] ? top[0].name : top[0].path, top[0].path,
				 cw, false);
	}

	if (past_count) {
		heading(l, "Past weeks");
		for (int k = 0; k < past_count; k++) {
			library_stat_row_t s;
			int n = library_stats_range_top_songs(past[k], past[k] + 6, &s, 1);
			lv_obj_t *card = lv_obj_create(l);
			lv_obj_remove_style_all(card);
			lv_obj_set_size(card, w, 104);
			lv_obj_set_style_bg_color(card, lv_color_hex(0x1f1f1f), 0);
			lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
			lv_obj_set_style_bg_color(card, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
			lv_obj_set_style_radius(card, 8, 0);
			lv_obj_set_style_margin_bottom(card, 10, 0);
			lv_obj_set_scrollable(card, false);
			lv_obj_set_clickable(card, true);
			lv_obj_set_event_bubble(card, true);
			lv_obj_add_event_cb(card, week_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)past[k]);
			art_row_t *r = art(card, n ? s.path : "", 64, false);
			if (r) lv_obj_align(r->img, LV_ALIGN_LEFT_MID, 14, 0);
			week_dates(past[k], dates, sizeof(dates));
			lv_obj_t *dt = text(card, dates, &font_ui_20_bold, 0xffffff, w - 136);
			lv_obj_set_pos(dt, 92, 14);
			lv_obj_t *sn = text(card, n ? (s.name[0] ? s.name : s.path) : "", &font_ui_16, QUIET, w - 136);
			lv_obj_set_pos(sn, 92, 44);
			char pl[24];
			snprintf(pl, sizeof(pl), "%d %s", n ? s.plays : 0, n && s.plays == 1 ? "play" : "plays");
			lv_obj_t *pc = text(card, pl, &font_ui_16, QUIET, w - 136);
			lv_obj_set_pos(pc, 92, 70);
			lv_obj_t *chev = lv_image_create(card);
			lv_image_set_src(chev, &icon_chevron_right);
			lv_obj_add_style(chev, &theme_style_icon, 0);
			lv_obj_align(chev, LV_ALIGN_RIGHT_MID, -12, 0);
		}
	}
}

void activity_open_stats(void) {
	stats_build();
	if (lv_screen_active() != screen[1]) switch_screen(screen[1]);
}

/* A week's own page: its artists as a carousel, its songs ranked. */
static void week_detail_build(void) {
	char dates[48];
	week_dates(week_start(detail_from), dates, sizeof(dates));
	page_begin(2, dates);
	lv_label_set_text(settingsrow_page_title(screen[2]), dates);
	lv_obj_t *l = list[2];
	static library_stat_row_t top[TOP_SONGS > TOP_ARTISTS ? TOP_SONGS : TOP_ARTISTS];
	int na = library_stats_range_top_artists(detail_from, detail_to, top, TOP_ARTISTS);
	if (na) {
		heading(l, "Top artists");
		/* A carousel running to the screen edge, as Home's shelves do. */
		lv_obj_t *shelf = lv_obj_create(l);
		lv_obj_remove_style_all(shelf);
		/* Full width: the list centres it, so it spans the gutters. */
		lv_obj_set_size(shelf, cfg->screen_width, 220);
		lv_obj_set_style_pad_hor(shelf, cfg->padding, 0);
		lv_obj_set_flex_flow(shelf, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_pad_column(shelf, 16, 0);
		lv_obj_set_scroll_dir(shelf, LV_DIR_HOR);
		lv_obj_set_scroll_snap_x(shelf, LV_SCROLL_SNAP_START);
		lv_obj_set_scrollbar_mode(shelf, LV_SCROLLBAR_MODE_OFF);
		for (int i = 0; i < na; i++) {
			snprintf(artist_names[i], sizeof(artist_names[i]), "%s", top[i].name);
			lv_obj_t *card = lv_obj_create(shelf);
			lv_obj_remove_style_all(card);
			lv_obj_set_size(card, 140, 220);
			lv_obj_set_clickable(card, true);
			lv_obj_set_scrollable(card, false);
			lv_obj_add_flag(card, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
			lv_obj_set_style_opa(card, LV_OPA_70, LV_STATE_PRESSED);
			lv_obj_add_event_cb(card, artist_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
			art_row_t *r = art(card, top[i].path, 140, true);
			if (r) lv_obj_set_pos(r->img, 0, 0);
			char name[280];
			snprintf(name, sizeof(name), "%d. %s", i + 1, top[i].name);
			/* Up to two lines of name, as Home's cards. */
			int lh = lv_font_get_line_height(&font_ui_18);
			lv_obj_t *t = text(card, name, &font_ui_18, 0xffffff, 0);
			lv_obj_set_size(t, 140, 2 * lh);
			lv_label_set_long_mode(t, LV_LABEL_LONG_DOT);
			lv_obj_set_pos(t, 0, 148);
			char sub[32];
			snprintf(sub, sizeof(sub), "%d %s", top[i].plays, top[i].plays == 1 ? "play" : "plays");
			lv_obj_t *s = text(card, sub, &font_ui_16, QUIET, 140);
			lv_obj_set_pos(s, 0, 150 + 2 * lh);
		}
	}
	top_count = library_stats_range_top_songs(detail_from, detail_to, top, TOP_SONGS);
	if (top_count) {
		heading(l, "Top songs");
		for (int i = 0; i < top_count; i++) {
			snprintf(top_paths[i], sizeof(top_paths[i]), "%s", top[i].path);
			char sub[300];
			snprintf(sub, sizeof(sub), "%s%s%d %s", top[i].artist, top[i].artist[0] ? " \xc2\xb7 " : "", top[i].plays,
					 top[i].plays == 1 ? "play" : "plays");
			song_row(l, top[i].path, top[i].name[0] ? top[i].name : top[i].path, sub, i + 1, -(i + 1));
		}
	}
	if (!na && !top_count) empty_note(l, "No plays this week", "Songs you play will show up here.");
}

static void week_detail_open(int from, int to) {
	detail_from = from;
	detail_to = to;
	week_detail_build();
	switch_screen(screen[2]);
}

/* ---- Composers (an R1 addition for classical libraries): everyone the
 * COMPOSER / TCOM tags name, with how many tracks; one opens their tracks,
 * by album, as a list. ---- */
#define COMPOSERS_MAX 300
static char composer_names[COMPOSERS_MAX][128];
static void composer_clicked_cb(lv_event_t *e) {
	if (!tapped()) return;
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	static char paths[500][512];
	int n = library_composer_paths(composer_names[i], paths, 500);
	if (n <= 0) return;
	static const char *list_paths[500], *names[500], *artists[500];
	static char nb[500][128], ab[500][96];
	for (int k = 0; k < n; k++) {
		list_paths[k] = paths[k];
		playlists_track_names(paths[k], nb[k], sizeof(nb[k]), ab[k], sizeof(ab[k]));
		names[k] = nb[k];
		artists[k] = ab[k];
	}
	medialist_open_paths(composer_names[i], list_paths, names, artists, n, NULL);
}

void activity_open_composers(void) {
	page_begin(3, "Composers");
	lv_obj_t *l = list[3];
	int w = lv_obj_get_content_width(l);
	static int counts[COMPOSERS_MAX];
	int n = library_composers(composer_names, counts, COMPOSERS_MAX);
	for (int i = 0; i < n; i++) {
		lv_obj_t *row = lv_obj_create(l);
		lv_obj_remove_style_all(row);
		lv_obj_set_size(row, w, 72);
		lv_obj_set_style_bg_color(row, lv_color_hex(0x1f1f1f), LV_STATE_PRESSED);
		lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
		lv_obj_set_clickable(row, true);
		lv_obj_set_scrollable(row, false);
		lv_obj_set_event_bubble(row, true);
		lv_obj_add_event_cb(row, composer_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
		lv_obj_t *t = text(row, composer_names[i], &font_ui_20, 0xffffff, w - 8);
		lv_obj_set_pos(t, 0, 10);
		char sub[32];
		snprintf(sub, sizeof(sub), counts[i] == 1 ? "1 track" : "%d tracks", counts[i]);
		lv_obj_t *c = text(row, sub, &font_ui_16, QUIET, w - 8);
		lv_obj_set_pos(c, 0, 40);
	}
	if (!n) empty_note(l, "No composers yet", "Composer tags in your files show up here after a scan.");
	switch_screen(screen[3]);
}

void activity_init(gui_config_t *config) {
	cfg = config;
	for (int k = 0; k < COVERLOADER_ACTIVITY_COUNT; k++) slot_owner[k] = -1;
}
