#include "storagepage.h"

#include <dirent.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include "src/gui/fonts/fonts.h"
#include "src/gui/shell/settingsrow.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/system/library/audiobookdb.h"
#include "src/system/library/library.h"

/* Storage, as the iPhone's: one bar for the card split by what fills it, a
 * line per kind under it, and the R1's own memory (the system) apart, since
 * nothing on the card is the system. Spotify's Settings > Storage is about its
 * cache, which this player does not have.
 *
 * The card is walked on a thread of its own each time the page opens, gently
 * (a pause every few hundred files, so playback from the same card keeps its
 * share); the page shows the last result while the new one is worked out. */

enum { KIND_MUSIC, KIND_AUDIOBOOKS, KIND_PHOTOS, KIND_BOOKS, KIND_PLAYER, KIND_OTHER, KINDS };
static const char *const KIND_NAMES[KINDS] = {"Music", "Audiobooks", "Photos", "Books", "Player data", "Other"};
static const char *const KIND_NOTES[KINDS] = {"Songs in your library", "The Audiobooks folder",
											  "Pictures on the card", "EPUB, PDF and text",
											  "Library index, covers, lyrics", "Everything else"};
static const uint32_t KIND_COLOURS[KINDS] = {0x1ed760, 0xffa42b, 0x509bf5, 0xaf2896, 0xf573a0, 0x9a9a9a};
#define FREE_COLOUR 0x3a3a3a
#define DEPTH_MAX 24

lv_obj_t *storagepage_screen;
static lv_obj_t *list;
static lv_timer_t *poll;
static const char *root;

// The walk. Written by the thread, read by the page once `done` is set.
static struct {
	uint64_t bytes[KINDS];
	unsigned files;
} walk_now, walk_last;
static bool walking, done, have_last;
static pthread_t walker;

static bool ext_in(const char *name, const char *const *list) {
	const char *dot = strrchr(name, '.');
	if (!dot) {
		return false;
	}
	for (; *list; list++) {
		if (!strcasecmp(dot + 1, *list)) {
			return true;
		}
	}
	return false;
}

static int kind_of(const char *name, int top_kind) {
	static const char *const AUDIO[] = {"flac", "mp3", "m4a", "aac", "alac", "ogg", "opus", "wav", "aif", "aiff",
										"ape", "wv", "dsf", "dff", "wma", "mka", "cue", "m4b", NULL};
	static const char *const IMAGES[] = {"jpg", "jpeg", "png", "bmp", "gif", "webp", "heic", NULL};
	static const char *const BOOKS[] = {"epub", "pdf", "txt", "mobi", "fb2", "azw3", NULL};
	if (top_kind >= 0) {
		return top_kind;
	}
	const char *dot = strrchr(name, '.');
	if (dot && !strcasecmp(dot, ".m4b"))
		return KIND_AUDIOBOOKS;
	if (ext_in(name, AUDIO)) {
		return KIND_MUSIC;
	}
	if (ext_in(name, IMAGES)) {
		return KIND_PHOTOS;
	}
	if (ext_in(name, BOOKS)) {
		return KIND_BOOKS;
	}
	return KIND_OTHER;
}

// `top_kind` is forced for a whole folder (the Audiobooks folder, the player's
// hidden ones), -1 to go by each file's kind.
static void walk_dir(char *path, size_t len, size_t cap, int depth, int top_kind) {
	DIR *d = opendir(path);
	if (!d) {
		return;
	}
	struct dirent *ent;
	while ((ent = readdir(d))) {
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
			continue;
		}
		size_t n = strlen(ent->d_name);
		if (len + 1 + n + 1 > cap) {
			continue;
		}
		path[len] = '/';
		memcpy(path + len + 1, ent->d_name, n + 1);
		struct stat st;
		if (lstat(path, &st) == 0) {
			if (S_ISDIR(st.st_mode)) {
				int kind = top_kind;
				if (depth == 0 && kind < 0) {
					if (!strcmp(ent->d_name, ".local") || !strcmp(ent->d_name, ".hiby")) {
						kind = KIND_PLAYER; // .local, .hiby: the player's own files
					} else if (!strcasecmp(ent->d_name, AUDIOBOOKDB_FOLDER)) {
						kind = KIND_AUDIOBOOKS;
					}
				}
				if (depth < DEPTH_MAX) {
					walk_dir(path, len + 1 + n, cap, depth + 1, kind);
				}
			} else if (S_ISREG(st.st_mode)) {
				walk_now.bytes[kind_of(ent->d_name, top_kind)] += (uint64_t)st.st_size;
				if (++walk_now.files % 256 == 0) {
					usleep(2000);
				}
			}
		}
		path[len] = '\0';
	}
	closedir(d);
}

static void *walk_thread(void *arg) {
	(void)arg;
	static char path[1024];
	memset(&walk_now, 0, sizeof(walk_now));
	snprintf(path, sizeof(path), "%s", root);
	walk_dir(path, strlen(path), sizeof(path), 0, -1);
	__atomic_store_n(&done, true, __ATOMIC_RELEASE);
	return NULL;
}

static void walk_start(void) {
	if (walking || !root || !root[0]) {
		return;
	}
	__atomic_store_n(&done, false, __ATOMIC_RELEASE);
	walking = true;
	if (pthread_create(&walker, NULL, walk_thread, NULL) != 0) {
		walking = false;
		return;
	}
	pthread_detach(walker);
}

// ---------------------------------------------------------------------------
// the page
// ---------------------------------------------------------------------------

static void size_text(uint64_t bytes, char *out, size_t size) {
	double b = (double)bytes;
	if (b >= 1e9) {
		snprintf(out, size, "%.1f GB", b / 1e9);
	} else if (b >= 1e6) {
		snprintf(out, size, "%.0f MB", b / 1e6);
	} else {
		snprintf(out, size, "%.0f KB", b / 1e3);
	}
}

static lv_obj_t *text(lv_obj_t *parent, const char *s, const lv_font_t *font, bool dim) {
	lv_obj_t *l = lv_label_create(parent);
	lv_label_set_text(l, s);
	lv_obj_set_style_text_font(l, font, 0);
	lv_obj_add_style(l, dim ? &theme_style_text_dim : &theme_style_text, 0);
	return l;
}

// One bar in segments, in proportion, the free part last.
static void bar(lv_obj_t *parent, const uint64_t *parts, const uint32_t *colours, int n, uint64_t total) {
	lv_obj_t *b = lv_obj_create(parent);
	lv_obj_remove_style_all(b);
	lv_obj_set_size(b, lv_pct(100), 18);
	lv_obj_set_style_radius(b, 6, 0);
	lv_obj_set_style_clip_corner(b, true, 0);
	lv_obj_set_style_bg_color(b, lv_color_hex(FREE_COLOUR), 0);
	lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
	lv_obj_set_flex_flow(b, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(b, 0, 0);
	lv_obj_set_scrollable(b, false);
	lv_obj_update_layout(parent);
	int w = lv_obj_get_content_width(b), used = 0;
	for (int i = 0; i < n && total; i++) {
		if (!parts[i]) {
			continue;
		}
		int px = (int)((double)parts[i] / (double)total * w);
		if (px > w - used) px = w - used;
		if (px <= 0) continue;
		used += px;
		lv_obj_t *s = lv_obj_create(b);
		lv_obj_remove_style_all(s);
		lv_obj_set_size(s, px, lv_pct(100));
		lv_obj_set_style_bg_color(s, lv_color_hex(colours[i]), 0);
		lv_obj_set_style_bg_opa(s, LV_OPA_COVER, 0);
	}
}

// A line of the legend: a dot of the kind's colour, its name and note, its size.
static void legend_row(lv_obj_t *parent, uint32_t colour, const char *name, const char *note, const char *value) {
	lv_obj_t *row = lv_obj_create(parent);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, lv_pct(100), 60);
	lv_obj_set_scrollable(row, false);
	lv_obj_t *dot = lv_obj_create(row);
	lv_obj_remove_style_all(dot);
	lv_obj_set_size(dot, 14, 14);
	lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_color(dot, lv_color_hex(colour), 0);
	lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
	lv_obj_align(dot, LV_ALIGN_LEFT_MID, 4, 0);
	lv_obj_t *t = text(row, name, &font_ui_20, false);
	lv_obj_set_width(t, lv_pct(64));
	lv_label_set_long_mode(t, LV_LABEL_LONG_DOT);
	lv_obj_set_pos(t, 30, 6);
	lv_obj_t *s = text(row, note, &font_ui_16, true);
	lv_obj_set_width(s, lv_pct(64));
	lv_label_set_long_mode(s, LV_LABEL_LONG_DOT);
	lv_obj_set_pos(s, 30, 34);
	lv_obj_t *v = text(row, value, &font_ui_18, true);
	lv_obj_align(v, LV_ALIGN_RIGHT_MID, -4, 0);
}

static void card_section(void) {
	struct statvfs vfs;
	if (!root || statvfs(root, &vfs) != 0 || !vfs.f_blocks) {
		text(list, "No card", &font_ui_24_bold, false);
		return;
	}
	uint64_t total = (uint64_t)vfs.f_blocks * vfs.f_frsize, avail = (uint64_t)vfs.f_bavail * vfs.f_frsize;
	char used_s[32], total_s[32], line[96];
	size_text(total - avail, used_s, sizeof(used_s));
	size_text(total, total_s, sizeof(total_s));
	lv_obj_t *h = text(list, "Card", &font_ui_24_bold, false);
	lv_obj_set_width(h, lv_pct(100));
	lv_obj_set_style_pad_top(h, 4, 0);
	snprintf(line, sizeof(line), "%s of %s used", used_s, total_s);
	lv_obj_set_width(text(list, line, &font_ui_18, true), lv_pct(100));

	if (!have_last) {
		bar(list, NULL, NULL, 0, total);
		lv_obj_t *wait = text(list, "Working out what fills the card\xE2\x80\xA6", &font_ui_18, true);
		lv_obj_set_style_pad_top(wait, 8, 0);
		return;
	}
	// The kinds add up to what the files take; the filesystem's own overhead
	// and anything unreadable go to Other, so the bar matches "used".
	uint64_t parts[KINDS], sum = 0;
	memcpy(parts, walk_last.bytes, sizeof(parts));
	for (int i = 0; i < KINDS; i++) {
		sum += parts[i];
	}
	if (total - avail > sum) {
		parts[KIND_OTHER] += total - avail - sum;
	}
	bar(list, parts, KIND_COLOURS, KINDS, total);
	if (walking) {
		lv_obj_t *wait = text(list, "Updating\xE2\x80\xA6", &font_ui_16, true);
		lv_obj_set_style_pad_top(wait, 4, 0);
	}
	for (int i = 0; i < KINDS; i++) {
		char v[32];
		size_text(parts[i], v, sizeof(v));
		legend_row(list, KIND_COLOURS[i], KIND_NAMES[i], KIND_NOTES[i], v);
	}
	char v[32];
	size_text(avail, v, sizeof(v));
	legend_row(list, FREE_COLOUR, "Free", "Room for more", v);
}

// The R1's own memory: the system and the player, read-only to the user.
static void system_section(void) {
	struct statvfs vfs;
	if (statvfs("/", &vfs) != 0 || !vfs.f_blocks) {
		return;
	}
	uint64_t total = (uint64_t)vfs.f_blocks * vfs.f_frsize, avail = (uint64_t)vfs.f_bavail * vfs.f_frsize;
	char used_s[32], total_s[32], line[96];
	size_text(total - avail, used_s, sizeof(used_s));
	size_text(total, total_s, sizeof(total_s));
	lv_obj_t *h = text(list, "System", &font_ui_24_bold, false);
	lv_obj_set_width(h, lv_pct(100));
	lv_obj_set_style_pad_top(h, 18, 0);
	snprintf(line, sizeof(line), "%s of %s internal memory used", used_s, total_s);
	lv_obj_set_width(text(list, line, &font_ui_18, true), lv_pct(100));
	const uint64_t parts[1] = {total - avail};
	const uint32_t colours[1] = {0xe9e9e9};
	bar(list, parts, colours, 1, total);
	legend_row(list, 0xe9e9e9, "OS", "The R1's firmware and this player", used_s);
}

static void render(void) {
	int32_t y = lv_obj_get_scroll_y(list);
	lv_obj_clean(list);
	card_section();
	system_section();
	lv_obj_t *note = text(list, "Nothing here deletes files. Sizes are worked out from the files on the card.",
						  settingsrow_note_font(), true);
	lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(note, lv_pct(100));
	lv_obj_set_style_pad_top(note, 12, 0);
	lv_obj_update_layout(list);
	lv_obj_scroll_to_y(list, y, LV_ANIM_OFF);
}

static void poll_cb(lv_timer_t *t) {
	(void)t;
	if (!__atomic_load_n(&done, __ATOMIC_ACQUIRE)) {
		return;
	}
	__atomic_store_n(&done, false, __ATOMIC_RELEASE);
	walking = false;
	walk_last = walk_now;
	have_last = true;
	lv_timer_pause(poll);
	if (lv_screen_active() == storagepage_screen) {
		render();
	}
}

static void loaded_cb(lv_event_t *e) {
	(void)e;
	walk_start();
	render();
	lv_timer_resume(poll);
}

void storagepage_init(gui_config_t *cfg) {
	root = cfg->sd_root_path;
	storagepage_screen = lv_obj_create(NULL);
	list = settingsrow_page(storagepage_screen, cfg, "Storage");
	settingsrow_title_corner_slots(settingsrow_page_title(storagepage_screen), cfg, 0);
	switcher_attach_back_gesture(storagepage_screen);
	lv_obj_add_event_cb(storagepage_screen, loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
	poll = lv_timer_create(poll_cb, 300, NULL);
	lv_timer_pause(poll);
}
