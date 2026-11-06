#include "deviceprofile.h"

#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#include "src/gui/fonts/fonts.h"
#include "src/gui/nowplaying/cover.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/system/core/config.h"

#define DEFAULT_NAME "Sonix R1"
#define NAME_MAX_LEN 40
#define AVATAR_SLOTS 12 // avatars and name labels alive at once
#define PICTURE_SIZES 4 // decoded once per size in use
#define PICK_MAX 80		// images offered from the card

static char sd_root[256];
static char image_path[512];
static bool image_known;

// ---------------------------------------------------------------------------
// name and picture
// ---------------------------------------------------------------------------

void deviceprofile_init(const char *root) { snprintf(sd_root, sizeof(sd_root), "%s", root ? root : ""); }

const char *deviceprofile_name(void) {
	const char *name = config_get("device", "name", DEFAULT_NAME);
	return name && name[0] ? name : DEFAULT_NAME;
}

const char *deviceprofile_image(void) {
	if (image_known) {
		return image_path;
	}
	image_known = true;
	const char *set = config_get("device", "image", NULL);
	if (set) {
		snprintf(image_path, sizeof(image_path), "%s", set);
	} else {
		// No choice made yet: a picture put on the card for the purpose.
		struct stat st;
		snprintf(image_path, sizeof(image_path), "%s/.local/profile.jpg", sd_root);
		if (!sd_root[0] || stat(image_path, &st) != 0) {
			image_path[0] = '\0';
		}
	}
	return image_path;
}

// ---------------------------------------------------------------------------
// what shows them
// ---------------------------------------------------------------------------

static struct {
	lv_obj_t *obj;
	int size; // 0 for a name label
	const lv_font_t *font;
} shown[AVATAR_SLOTS];

static struct {
	int size;
	bool tried;
	bool ok;
	cover_image_t image;
} pictures[PICTURE_SIZES];

static void pictures_forget(void) {
	for (int i = 0; i < PICTURE_SIZES; i++) {
		if (pictures[i].ok) {
			cover_free(&pictures[i].image);
		}
		memset(&pictures[i], 0, sizeof(pictures[i]));
	}
}

// The picture at `size`, decoded the first time it is asked for.
static const lv_image_dsc_t *picture_at(int size) {
	const char *path = deviceprofile_image();
	if (!path[0]) {
		return NULL;
	}
	int free_slot = -1;
	for (int i = 0; i < PICTURE_SIZES; i++) {
		if (pictures[i].tried && pictures[i].size == size) {
			return pictures[i].ok ? &pictures[i].image.dsc : NULL;
		}
		if (!pictures[i].tried && free_slot < 0) {
			free_slot = i;
		}
	}
	if (free_slot < 0) {
		return NULL; // more sizes than anywhere shows
	}
	pictures[free_slot].tried = true;
	pictures[free_slot].size = size;
	pictures[free_slot].ok = cover_load_image_file(path, size, size, COVER_FIT_COVER, &pictures[free_slot].image);
	return pictures[free_slot].ok ? &pictures[free_slot].image.dsc : NULL;
}

static void paint_avatar(lv_obj_t *obj, int size, const lv_font_t *font) {
	lv_obj_clean(obj);
	lv_obj_set_style_radius(obj, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_color(obj, lv_color_hex(0xf573a0), 0);
	lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
	const lv_image_dsc_t *picture = picture_at(size);
	if (picture) {
		// Cut round by the image's own radius: no layer, unlike clip_corner.
		lv_obj_t *im = lv_image_create(obj);
		lv_image_set_src(im, picture);
		lv_obj_set_size(im, size, size);
		lv_obj_set_style_radius(im, LV_RADIUS_CIRCLE, 0);
		lv_obj_set_style_clip_corner(im, false, 0);
		lv_obj_center(im);
		lv_obj_set_clickable(im, false);
		return;
	}
	char initial[2] = {(char)toupper((unsigned char)deviceprofile_name()[0]), 0};
	if (!isalnum((unsigned char)initial[0])) {
		initial[0] = 'S';
	}
	lv_obj_t *l = lv_label_create(obj);
	lv_label_set_text(l, initial);
	lv_obj_set_style_text_font(l, font, 0);
	lv_obj_set_style_text_color(l, lv_color_black(), 0);
	lv_obj_center(l);
}

static void shown_deleted_cb(lv_event_t *e) {
	lv_obj_t *obj = lv_event_get_current_target(e);
	for (int i = 0; i < AVATAR_SLOTS; i++) {
		if (shown[i].obj == obj) {
			shown[i].obj = NULL;
		}
	}
}

static void shown_add(lv_obj_t *obj, int size, const lv_font_t *font) {
	int slot = -1;
	for (int i = 0; i < AVATAR_SLOTS; i++) {
		if (shown[i].obj == obj) {
			shown[i].size = size;
			shown[i].font = font;
			return;
		}
		if (!shown[i].obj && slot < 0) {
			slot = i;
		}
	}
	if (slot < 0) {
		return; // painted once, not kept up to date
	}
	shown[slot].obj = obj;
	shown[slot].size = size;
	shown[slot].font = font;
	lv_obj_add_event_cb(obj, shown_deleted_cb, LV_EVENT_DELETE, NULL);
}

void deviceprofile_avatar(lv_obj_t *obj, int size, const lv_font_t *font) {
	if (!obj) {
		return;
	}
	shown_add(obj, size, font);
	paint_avatar(obj, size, font);
}

void deviceprofile_name_label(lv_obj_t *label) {
	if (!label) {
		return;
	}
	shown_add(label, 0, NULL);
	lv_label_set_text(label, deviceprofile_name());
}

static void (*listeners[4])(void);

void deviceprofile_on_change(void (*cb)(void)) {
	for (int i = 0; i < 4; i++) {
		if (!listeners[i] || listeners[i] == cb) {
			listeners[i] = cb;
			return;
		}
	}
}

static void repaint_all(void) {
	for (int i = 0; i < 4; i++) {
		if (listeners[i]) {
			listeners[i]();
		}
	}
	for (int i = 0; i < AVATAR_SLOTS; i++) {
		if (!shown[i].obj) {
			continue;
		}
		if (shown[i].size) {
			paint_avatar(shown[i].obj, shown[i].size, shown[i].font);
		} else {
			lv_label_set_text(shown[i].obj, deviceprofile_name());
		}
	}
}

void deviceprofile_set_name(const char *name) {
	char clean[NAME_MAX_LEN + 1];
	while (name && isspace((unsigned char)*name)) {
		name++;
	}
	snprintf(clean, sizeof(clean), "%s", name ? name : "");
	for (int n = (int)strlen(clean); n > 0 && isspace((unsigned char)clean[n - 1]); n--) {
		clean[n - 1] = '\0';
	}
	config_set("device", "name", clean[0] ? clean : DEFAULT_NAME);
	config_save();
	repaint_all();
}

void deviceprofile_set_image(const char *path) {
	config_set("device", "image", path ? path : "");
	config_save();
	image_known = false;
	pictures_forget();
	repaint_all();
}

// ---------------------------------------------------------------------------
// the name, with the keyboard
// ---------------------------------------------------------------------------

static lv_obj_t *name_layer, *name_field;

static void name_close(void) {
	if (name_layer) {
		lv_obj_delete(name_layer);
		name_layer = name_field = NULL;
	}
}

static void name_cancel_cb(lv_event_t *e) {
	(void)e;
	name_close();
}

static void name_save_cb(lv_event_t *e) {
	(void)e;
	if (name_field) {
		deviceprofile_set_name(lv_textarea_get_text(name_field));
	}
	name_close();
}

static lv_obj_t *pill(lv_obj_t *parent, const char *text, bool green, lv_event_cb_t cb) {
	lv_obj_t *b = lv_obj_create(parent);
	lv_obj_remove_style_all(b);
	lv_obj_set_size(b, 132, 50);
	lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_color(b, lv_color_hex(0x1ed760), 0);
	lv_obj_set_style_bg_opa(b, green ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
	lv_obj_set_style_border_color(b, lv_color_hex(0x7c7c7c), 0);
	lv_obj_set_style_border_width(b, green ? 0 : 1, 0);
	lv_obj_set_clickable(b, true);
	lv_obj_set_scrollable(b, false);
	lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t *l = lv_label_create(b);
	lv_label_set_text(l, text);
	lv_obj_set_style_text_font(l, &font_ui_20_bold, 0);
	lv_obj_set_style_text_color(l, green ? lv_color_black() : lv_color_white(), 0);
	lv_obj_center(l);
	return b;
}

// Spotify's naming step (the playlist one): a centred question, the name large
// on an underline, Cancel and Save, the keyboard below finishing the same way.
void deviceprofile_edit_name(void) {
	name_close();
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
	name_layer = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(name_layer);
	lv_obj_set_size(name_layer, w, h);
	lv_obj_set_style_bg_color(name_layer, lv_color_hex(0x202124), 0);
	lv_obj_set_style_bg_opa(name_layer, LV_OPA_COVER, 0);
	lv_obj_set_scrollable(name_layer, false);
	lv_obj_set_clickable(name_layer, true);
	lv_obj_t *l = lv_label_create(name_layer);
	lv_label_set_text(l, "Name your device");
	lv_obj_set_width(l, w - 48);
	lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_font(l, &font_ui_24_bold, 0);
	lv_obj_set_style_text_color(l, lv_color_white(), 0);
	lv_obj_align(l, LV_ALIGN_TOP_MID, 0, 40);
	name_field = lv_textarea_create(name_layer);
	lv_obj_remove_style_all(name_field);
	lv_obj_set_size(name_field, w - 64, 58);
	lv_obj_align(name_field, LV_ALIGN_TOP_MID, 0, 100);
	lv_textarea_set_one_line(name_field, true);
	lv_textarea_set_max_length(name_field, NAME_MAX_LEN);
	lv_textarea_set_placeholder_text(name_field, DEFAULT_NAME);
	lv_textarea_set_text(name_field, deviceprofile_name());
	lv_obj_set_style_text_font(name_field, &font_ui_32, 0);
	lv_obj_set_style_text_color(name_field, lv_color_white(), 0);
	lv_obj_set_style_text_align(name_field, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_color(name_field, lv_color_hex(0x8a8a8a), LV_PART_TEXTAREA_PLACEHOLDER);
	lv_obj_set_style_border_side(name_field, LV_BORDER_SIDE_BOTTOM, 0);
	lv_obj_set_style_border_width(name_field, 2, 0);
	lv_obj_set_style_border_color(name_field, lv_color_white(), 0);
	lv_obj_set_style_pad_bottom(name_field, 6, 0);
	keyboard_style_caret(name_field);
	lv_obj_t *row = lv_obj_create(name_layer);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, 132 * 2 + 16, 50);
	lv_obj_align(row, LV_ALIGN_TOP_MID, 0, 184);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 16, 0);
	lv_obj_set_scrollable(row, false);
	pill(row, "Cancel", false, name_cancel_cb);
	pill(row, "Save", true, name_save_cb);
	keyboard_create(name_layer, w, LV_MIN(300, h - 250), name_field, NULL, "Save", name_save_cb, NULL);
}

// ---------------------------------------------------------------------------
// the picture, from the card
// ---------------------------------------------------------------------------

static char (*pick_paths)[512];
static int pick_count;

static bool is_image(const char *name) {
	const char *dot = strrchr(name, '.');
	return dot && (!strcasecmp(dot, ".jpg") || !strcasecmp(dot, ".jpeg") || !strcasecmp(dot, ".png") ||
				   !strcasecmp(dot, ".bmp"));
}

// The images in a few likely folders at the top of the card, not the whole
// card: a walk through every album folder is not a thing to do on a tap.
static void pick_collect(void) {
	static const char *const FOLDERS[] = {"", "Pictures", "pictures", "Images", "images",
										  "Photos", "photos", "DCIM", ".local"};
	if (!pick_paths) {
		pick_paths = malloc(sizeof(*pick_paths) * PICK_MAX);
	}
	pick_count = 0;
	if (!pick_paths) {
		return;
	}
	for (unsigned f = 0; f < sizeof(FOLDERS) / sizeof(FOLDERS[0]) && pick_count < PICK_MAX; f++) {
		char dir[400];
		snprintf(dir, sizeof(dir), "%s%s%s", sd_root, FOLDERS[f][0] ? "/" : "", FOLDERS[f]);
		DIR *d = opendir(dir);
		if (!d) {
			continue;
		}
		struct dirent *ent;
		while ((ent = readdir(d)) && pick_count < PICK_MAX) {
			if (ent->d_name[0] == '.' && strcmp(ent->d_name, "profile.jpg") != 0) {
				continue;
			}
			if (is_image(ent->d_name)) {
				snprintf(pick_paths[pick_count++], 512, "%s/%s", dir, ent->d_name);
			}
		}
		closedir(d);
	}
}

static void pick_chosen_cb(lv_event_t *e) {
	int i = (int)(intptr_t)lv_event_get_user_data(e);
	bottomsheet_close(true);
	if (i < 0) {
		deviceprofile_set_image("");
	} else if (i < pick_count) {
		deviceprofile_set_image(pick_paths[i]);
	}
}

static void pick_row(lv_obj_t *body, int w, const char *title, const char *sub, bool current, int index) {
	lv_obj_t *row = lv_obj_create(body);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, w, sub ? 64 : 56);
	lv_obj_set_style_bg_color(row, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
	lv_obj_set_style_radius(row, 8, 0);
	lv_obj_set_clickable(row, true);
	lv_obj_set_scrollable(row, false);
	lv_obj_add_event_cb(row, pick_chosen_cb, LV_EVENT_CLICKED, (void *)(intptr_t)index);
	lv_obj_t *t = lv_label_create(row);
	lv_label_set_text(t, title);
	lv_label_set_long_mode(t, LV_LABEL_LONG_DOT);
	lv_obj_set_size(t, w - 56, lv_font_get_line_height(&font_ui_20));
	lv_obj_set_style_text_font(t, &font_ui_20, 0);
	lv_obj_set_style_text_color(t, current ? lv_color_hex(0x1ed760) : lv_color_white(), 0);
	lv_obj_set_pos(t, 8, sub ? 6 : 14);
	if (sub) {
		lv_obj_t *s = lv_label_create(row);
		lv_label_set_text(s, sub);
		lv_label_set_long_mode(s, LV_LABEL_LONG_DOT);
		lv_obj_set_size(s, w - 56, lv_font_get_line_height(&font_ui_16));
		lv_obj_set_style_text_font(s, &font_ui_16, 0);
		lv_obj_set_style_text_color(s, lv_color_hex(0xb3b3b3), 0);
		lv_obj_set_pos(s, 8, 36);
	}
	if (current) {
		lv_obj_t *mark = lv_image_create(row);
		lv_image_set_src(mark, &icon_check);
		lv_obj_set_style_image_recolor(mark, lv_color_hex(0x1ed760), 0);
		lv_obj_set_style_image_recolor_opa(mark, LV_OPA_COVER, 0);
		lv_obj_align(mark, LV_ALIGN_RIGHT_MID, -8, 0);
	}
}

void deviceprofile_pick_image(void) {
	pick_collect();
	int h = lv_display_get_vertical_resolution(NULL);
	lv_obj_t *body = bottomsheet_open(h - 80, NULL);
	lv_obj_set_style_pad_hor(body, 16, 0);
	lv_obj_set_style_pad_bottom(body, 16, 0);
	lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(body, 2, 0);
	int w = lv_obj_get_content_width(body);
	lv_obj_t *title = lv_label_create(body);
	lv_label_set_text(title, "Choose a picture");
	lv_obj_set_style_text_font(title, &font_ui_24_bold, 0);
	lv_obj_set_style_text_color(title, lv_color_white(), 0);
	lv_obj_t *note = lv_label_create(body);
	lv_label_set_text(note, "Pictures at the top of the card and in its Pictures, Photos, Images and DCIM folders.");
	lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(note, w);
	lv_obj_set_style_text_font(note, &font_ui_16, 0);
	lv_obj_set_style_text_color(note, lv_color_hex(0xb3b3b3), 0);
	lv_obj_set_style_pad_bottom(note, 8, 0);
	const char *current = deviceprofile_image();
	pick_row(body, w, "No picture", "Your device's initial", !current[0], -1);
	for (int i = 0; i < pick_count; i++) {
		const char *slash = strrchr(pick_paths[i], '/');
		char folder[200];
		snprintf(folder, sizeof(folder), "%.*s", slash ? (int)(slash - pick_paths[i]) : 0, pick_paths[i]);
		const char *where = strncmp(folder, sd_root, strlen(sd_root)) == 0 ? folder + strlen(sd_root) : folder;
		pick_row(body, w, slash ? slash + 1 : pick_paths[i], where[0] ? where + (where[0] == '/') : "Card",
				 !strcmp(current, pick_paths[i]), i);
	}
	if (pick_count == 0) {
		lv_obj_t *none = lv_label_create(body);
		lv_label_set_text(none, "No pictures found. Copy a JPEG or PNG to the card's Pictures folder.");
		lv_label_set_long_mode(none, LV_LABEL_LONG_WRAP);
		lv_obj_set_width(none, w);
		lv_obj_set_style_text_font(none, &font_ui_18, 0);
		lv_obj_set_style_text_color(none, lv_color_white(), 0);
		lv_obj_set_style_pad_top(none, 12, 0);
	}
}
