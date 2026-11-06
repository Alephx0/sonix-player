#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "lvgl/lvgl.h"

#include "src/gui/settings/appearance.h"
#include "src/gui/settings/ccsettings.h"
#include "src/gui/settings/lastfmsettings.h"
#include "src/gui/settings/musicsettings.h"
#include "src/gui/wireless/wireless.h"
#include "src/gui/shell/confirm.h"
#include "src/gui/settings/devoptions.h"
#include "src/gui/settings/language.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/settings/kblayoutpage.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/nowplaying/player.h"
#include "src/gui/settings/powersettings.h"
#include "src/gui/settings/remap.h"
#include "src/gui/settings/screensaver.h"
#include "src/gui/shell/settingsrow.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/system/library/library.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/settings/systempage.h"
#include "src/gui/settings/storagepage.h"
#include "src/gui/shell/deviceprofile.h"
#include "src/gui/shell/theme.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/reference_icons.h"
#include "src/system/bluetooth/bluetooth.h"
#include "src/system/net/wifi.h"
#include "src/gui/settings/timeset.h"
#include "src/system/core/config.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include "src/system/audio/headset.h"
#include "src/system/core/lang.h"
#include "src/system/device/power.h"
#include "src/system/device/screenshot.h"
#include "src/system/device/system.h"

lv_obj_t *settings_screen;

static void clock_clicked_cb(lv_event_t *e) {
	(void)e;
	timeset_show();
}

// In-line remote keys on the headphone cable: play/pause and the two skips.
// Under More, since it is touched when headphones change, not daily.
static lv_obj_t *headset_switch;

static void headset_toggled_cb(lv_event_t *e) {
	(void)e;
	headset_set_controls_enabled(lv_obj_has_state(headset_switch, LV_STATE_CHECKED));
}

// Firmware update lives on the System page; see systempage.c.

// ---------------------------------------------------------------------------
// Screen: the panel brightness, a continuous slider written straight to the
// backlight (power_set_brightness), like the stock player's brightness bar.
// The chosen level is saved and handed to power_init() on the next boot.
// ---------------------------------------------------------------------------

static lv_obj_t *screensettings_screen;
static lv_obj_t *othersettings_screen;
static lv_obj_t *keyboard_screen;
static lv_obj_t *brightness_slider;
static lv_obj_t *brightness_value;
static lv_obj_t *screen_off_value;
static lv_obj_t *screen_off_slider;
static lv_obj_t *doubletap_switch;
static lv_obj_t *screensaver_switch;
static lv_obj_t *screensaver_pills;
static lv_obj_t *screensaver_album_pill;
static lv_obj_t *screensaver_images_pill;
static lv_obj_t *rotate_switch;

static void doubletap_toggle_cb(lv_event_t *e) {
	(void)e;
	bool on = lv_obj_has_state(doubletap_switch, LV_STATE_CHECKED);
	config_set_int("screen", "double_tap_wake", on ? 1 : 0);
	config_save();
	power_set_double_tap_wake(on);
}

// The two pills under the switch, and which of them is lit. Hidden with the
// screensaver off: "album or pictures" means nothing when there is no
// screensaver to show either.
static void screensaver_refresh(void) {
	if (!screensaver_switch) {
		return;
	}
	bool on = screensaver_get_enabled();
	if (on) {
		lv_obj_add_state(screensaver_switch, LV_STATE_CHECKED);
		lv_obj_set_hidden(screensaver_pills, false);
	} else {
		lv_obj_remove_state(screensaver_switch, LV_STATE_CHECKED);
		lv_obj_set_hidden(screensaver_pills, true);
	}
	screensaver_source_t source = screensaver_source();
	settingsrow_pill_active(screensaver_album_pill, source == SCREENSAVER_SOURCE_ALBUM);
	settingsrow_pill_active(screensaver_images_pill, source == SCREENSAVER_SOURCE_IMAGES);
}

static void screensaver_toggle_cb(lv_event_t *e) {
	(void)e;
	// Just the setting: the screensaver belongs to the next wake, not to the
	// moment the switch is flipped.
	screensaver_set_enabled(lv_obj_has_state(screensaver_switch, LV_STATE_CHECKED));
	screensaver_refresh();
}

static void screensaver_source_cb(lv_event_t *e) {
	if (player_sheet_drag_active() || switcher_back_drag_active()) {
		return;
	}
	screensaver_source_t chosen = (screensaver_source_t)(intptr_t)lv_event_get_user_data(e);

	// Pictures is refused rather than accepted-and-blank: the folder is judged
	// now, while the user is looking at the answer, instead of at a wake hours
	// later with nothing to show and no way to tell why.
	if (chosen == SCREENSAVER_SOURCE_IMAGES && !screensaver_has_images()) {
		confirm_notice("settings_no_pictures_to_show", "screensaver_empty_note");
		screensaver_set_source(SCREENSAVER_SOURCE_ALBUM);
		screensaver_refresh();
		return;
	}

	screensaver_set_source(chosen);
	screensaver_refresh();
}

// Rotate screen: the picture turns 180 degrees and the touch panel with it.
// The skip keys do not: the button remap page is where a user who wants them
// the other way round says so.
static void rotate_toggle_cb(lv_event_t *e) {
	(void)e;
	bool on = lv_obj_has_state(rotate_switch, LV_STATE_CHECKED);

	display_set_rotated(on);

	config_set_int("screen", "rotate_180", on ? 1 : 0);
	config_save();

	if (on && !display_rotation_supported()) {
		gui_notify_popup("settings_rotation_unavailable");
	}
}

// The lowest level offered. The stock player never writes below 5 of its
// Blanking this panel powers its touch controller down with it, so the only
// way back is the power key. Thirty seconds is the default; "never" is there
// for anyone who would rather not be caught out by it.
//
// The setting is written under [power], where it has always lived: it is read
// by power.c, and moving the key would lose everybody's choice.
static const struct {
	int seconds;
	const char *label;
} SCREEN_OFF[] = {
	{0, "power_never"},		 {15, "power_15_seconds"}, {30, "power_30_seconds"},
	{60, "power_1_minute"},	 {120, "power_2_minutes"}, {300, "power_5_minutes"},
};
#define SCREEN_OFF_COUNT ((int)(sizeof(SCREEN_OFF) / sizeof(SCREEN_OFF[0])))
#define SCREEN_OFF_DEFAULT 30

static int screen_off_index(void) {
	int seconds = (int)config_get_int("power", "screen_off_seconds", SCREEN_OFF_DEFAULT);
	for (int i = 0; i < SCREEN_OFF_COUNT; i++) {
		if (SCREEN_OFF[i].seconds == seconds) {
			return i;
		}
	}
	return 2; // the 30 second default
}

void settings_apply_screen_off(void) {
	int seconds = SCREEN_OFF[screen_off_index()].seconds;
	power_set_screen_off_timeout((uint32_t)seconds * 1000);
	power_set_screen_off_enabled(seconds > 0);
}

static void screen_off_changed_cb(lv_event_t *e) {
	(void)e;
	int index = (int)lv_slider_get_value(screen_off_slider);
	config_set_int("power", "screen_off_seconds", SCREEN_OFF[index].seconds);
	config_save();
	settings_apply_screen_off();
	lv_label_set_text(screen_off_value, tr(SCREEN_OFF[index].label));
}

// The dimmest backlight level on the slider: 0 would put the backlight out.
#define BRIGHTNESS_FLOOR 1

static void brightness_show_value(long value, long max) {
	lv_label_set_text_fmt(brightness_value, "%d%%", (int)((value * 100 + max / 2) / max));
}

// Live while dragging: the panel follows the finger.
static void brightness_changed_cb(lv_event_t *e) {
	(void)e;
	long value = lv_slider_get_value(brightness_slider);
	power_set_brightness(value);
	brightness_show_value(value, lv_slider_get_max_value(brightness_slider));
}

// Persisted once, when the finger lifts -- not on every pixel of the drag.
static void brightness_released_cb(lv_event_t *e) {
	(void)e;
	config_set_int("screen", "brightness", (int)lv_slider_get_value(brightness_slider));
	config_save();
}

// The panel's real range is only known after power_init() has run, which is
// after the GUI is built -- so the slider syncs itself every time the page
// comes up.
static void screensettings_loaded_cb(lv_event_t *e) {
	(void)e;
	long max = power_get_max_brightness();
	if (max <= 0) {
		max = 100;
	}
	lv_slider_set_range(brightness_slider, BRIGHTNESS_FLOOR, (int32_t)max);

	long value = power_get_brightness();
	if (value < BRIGHTNESS_FLOOR) {
		value = max;
	}
	lv_slider_set_value(brightness_slider, (int32_t)value, LV_ANIM_OFF);
	brightness_show_value(value, max);
}

// The slider's fill re-reads the palette after a theme/accent change.
static void refresh_brightness_slider(void) {
	theme_apply_slider_knob(brightness_slider);
	if (brightness_slider) {
		lv_obj_set_style_bg_color(brightness_slider, theme()->accent, LV_PART_INDICATOR);
		lv_obj_set_style_bg_color(brightness_slider, theme()->text_secondary, LV_PART_MAIN);
	}
}

static void build_screen_page(gui_config_t *cfg) {
	screensettings_screen = lv_obj_create(NULL);
	lv_obj_t *container = settingsrow_page(screensettings_screen, cfg, "settings_screen");

	// The brightness card: name up top, the current percent on the right, a
	// full-width slider underneath.
	lv_obj_t *card = lv_obj_create(container);
	lv_obj_set_width(card, lv_pct(100));
	lv_obj_set_height(card, 140);
	lv_obj_add_style(card, &theme_style_card, 0);
	lv_obj_set_style_radius(card, 12, 0);
	lv_obj_set_style_border_width(card, 0, 0);
	lv_obj_set_style_shadow_width(card, 0, 0);
	lv_obj_set_style_pad_hor(card, 20, 0);
	lv_obj_set_style_pad_ver(card, 14, 0);
	if (theme_reference()) {
		// A flat row like the others in the reference layout.
		lv_obj_set_style_bg_opa(card, LV_OPA_TRANSP, 0);
		lv_obj_set_style_pad_hor(card, 4, 0);
		lv_obj_set_height(card, 112);
	}
	lv_obj_set_scrollable(card, false);

	lv_obj_t *label = lv_label_create(card);
	lv_label_set_text(label, tr("brightness"));
	lv_obj_add_style(label, &theme_style_text, 0);
	lv_obj_set_style_text_font(label, &font_ui_24, 0);
	lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

	brightness_value = lv_label_create(card);
	lv_obj_add_style(brightness_value, &theme_style_text_dim, 0);
	lv_obj_set_style_text_font(brightness_value, &font_ui_24, 0);
	lv_obj_align(brightness_value, LV_ALIGN_TOP_RIGHT, 0, 0);
	lv_label_set_text(brightness_value, "");

	// The Adwaita slider: thin round trough, accent fill, white knob.
	brightness_slider = lv_slider_create(card);
	lv_obj_set_width(brightness_slider, lv_pct(100));
	lv_obj_set_height(brightness_slider, 10);
	lv_obj_align(brightness_slider, LV_ALIGN_BOTTOM_MID, 0, -14);
	lv_slider_set_range(brightness_slider, BRIGHTNESS_FLOOR, 100);

	lv_obj_set_style_radius(brightness_slider, LV_RADIUS_CIRCLE, LV_PART_MAIN);
	lv_obj_set_style_bg_color(brightness_slider, theme()->text_secondary, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(brightness_slider, LV_OPA_40, LV_PART_MAIN);

	lv_obj_set_style_radius(brightness_slider, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(brightness_slider, theme()->accent, LV_PART_INDICATOR);

	lv_obj_set_style_radius(brightness_slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
	lv_obj_set_style_bg_color(brightness_slider, lv_color_white(), LV_PART_KNOB);
	theme_apply_slider_knob(brightness_slider);
	lv_obj_set_style_pad_all(brightness_slider, 8, LV_PART_KNOB);
	lv_obj_set_style_shadow_width(brightness_slider, 8, LV_PART_KNOB);
	lv_obj_set_style_shadow_opa(brightness_slider, LV_OPA_30, LV_PART_KNOB);
	lv_obj_set_style_shadow_color(brightness_slider, lv_color_black(), LV_PART_KNOB);
	lv_obj_set_style_shadow_offset_y(brightness_slider, 1, LV_PART_KNOB);

	// A slim trough is a hard touch target; let the finger land anywhere in
	// the card's lower half.
	lv_obj_set_ext_click_area(brightness_slider, 24);

	lv_obj_add_event_cb(brightness_slider, brightness_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(brightness_slider, brightness_released_cb, LV_EVENT_RELEASED, NULL);

	// How long the panel waits before it goes dark. Straight under the
	// brightness: both are about the panel, and the one that switches it off is
	// the first thing looked for after the one that sets how bright it is.
	settingsrow_slider(container, "power_screen_off", SCREEN_OFF_COUNT, &screen_off_value, &screen_off_slider,
					   screen_off_changed_cb);
	lv_slider_set_value(screen_off_slider, screen_off_index(), LV_ANIM_OFF);
	lv_label_set_text(screen_off_value, tr(SCREEN_OFF[screen_off_index()].label));

	// Double-tap to wake: the touch controller's gesture mode. With the
	// screen off, two taps light it back up -- the way the stock player's
	// option works, through the same sysfs switch.
	// Not on the R1, whose touch controller sleeps with the panel.
	if (power_double_tap_wake_supported()) {
		settingsrow_toggle(container, "settings_double_tap_to_wake", &doubletap_switch, doubletap_toggle_cb);
		if (config_get_int("screen", "double_tap_wake", 0)) {
			lv_obj_add_state(doubletap_switch, LV_STATE_CHECKED);
		}
	}

	// The screensaver: a picture, the track and the time over everything else
	// the moment the panel lights back up. A swipe up puts it away. The pills
	// say where the picture comes from -- the record being played, or the
	// card's own Screensaver folder.
	settingsrow_toggle_pills(container, "screensaver", screensaver_toggle_cb, &screensaver_switch,
							 &screensaver_pills);
	screensaver_album_pill =
		settingsrow_pill(screensaver_pills, "screensaver_source_album", SCREENSAVER_SOURCE_ALBUM, screensaver_source_cb);
	screensaver_images_pill =
		settingsrow_pill(screensaver_pills, "screensaver_source_images", SCREENSAVER_SOURCE_IMAGES, screensaver_source_cb);
	screensaver_refresh();

	// Upside down, for holding the player the other way up.
	settingsrow_toggle(container, "settings_rotate_screen", &rotate_switch, rotate_toggle_cb);
	if (config_get_int("screen", "rotate_180", 0)) {
		lv_obj_add_state(rotate_switch, LV_STATE_CHECKED);
	}

	lv_obj_add_event_cb(screensettings_screen, screensettings_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
	switcher_attach_back_gesture(screensettings_screen);
	theme_register_refresh(refresh_brightness_slider);
	// The pills carry hand-set colours, so a theme change has to repaint them.
	theme_register_refresh(screensaver_refresh);
}

// The developer options row is kept in a global because it appears and
// disappears: it is always in the list, but only visible once unlocked.
static lv_obj_t *devoptions_row;

void settings_refresh_devoptions(void) {
	if (!devoptions_row) {
		return;
	}
	if (systempage_devoptions_unlocked()) {
		lv_obj_set_hidden(devoptions_row, false);
	} else {
		lv_obj_set_hidden(devoptions_row, true);
	}
}

// ---------------------------------------------------------------------------
// More: the things that fit nowhere else
//
// The in-line remote keys and the remapping of the three side buttons: both
// touched rarely enough not to earn a row on the main settings page.
// ---------------------------------------------------------------------------

// Keyboard type, QWERTY or T9, as pills (the same shape used for the clock
// position). The setting applies to every keyboard in the player and takes
// effect immediately via keyboard_refresh_type().
static lv_obj_t *kbtype_buttons[2]; // 0 = QWERTY, 1 = T9

static void refresh_kbtype_buttons(void) {
	int active = config_get_int("other", "keyboard_t9", 0) != 0 ? 1 : 0;
	for (int i = 0; i < 2; i++) {
		if (!kbtype_buttons[i]) {
			continue;
		}
		bool on = i == active;
		lv_obj_set_style_bg_color(kbtype_buttons[i], on ? theme()->accent : theme()->surface_pressed, 0);
		lv_obj_set_style_text_color(lv_obj_get_child(kbtype_buttons[i], 0),
									on ? theme()->accent_ink : theme()->text_primary, 0);
	}
}

static void kbtype_pick_cb(lv_event_t *e) {
	int t9 = (int)(intptr_t)lv_event_get_user_data(e);
	config_set_int("other", "keyboard_t9", t9);
	config_save();
	keyboard_refresh_type();
	refresh_kbtype_buttons();
}

static lv_obj_t *make_kbtype_pill(lv_obj_t *parent, const char *text, int t9) {
	lv_obj_t *btn = lv_btn_create(parent);
	lv_obj_set_size(btn, LV_SIZE_CONTENT, 64);
	lv_obj_set_style_pad_hor(btn, 22, 0);
	lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0); // the Adwaita pill
	lv_obj_set_style_shadow_width(btn, 0, 0);
	lv_obj_set_style_border_width(btn, 0, 0);
	lv_obj_add_event_cb(btn, kbtype_pick_cb, LV_EVENT_CLICKED, (void *)(intptr_t)t9);

	lv_obj_t *label = lv_label_create(btn);
	lv_label_set_text(label, text);
	lv_obj_set_style_text_font(label, &font_ui_22, 0);
	lv_obj_center(label);
	return btn;
}

// Screenshots, on the keys shown under the switch.
static lv_obj_t *screenshot_switch;

// Volume steps: how far one press of a volume key moves the volume, from
// VOLUME_STEP_MIN to VOLUME_STEP_MAX percent. Position 0 of the slider is the
// smallest step.
static lv_obj_t *volume_step_slider;
static lv_obj_t *volume_step_value;

static int volume_step_setting(void) {
	int step = (int)config_get_int("other", "volume_step", VOLUME_STEP_MIN);
	if (step < VOLUME_STEP_MIN) {
		return VOLUME_STEP_MIN;
	}
	return step > VOLUME_STEP_MAX ? VOLUME_STEP_MAX : step;
}

static void volume_step_changed_cb(lv_event_t *e) {
	(void)e;
	int step = VOLUME_STEP_MIN + (int)lv_slider_get_value(volume_step_slider);
	if (step == volume_step_setting()) {
		return;
	}
	config_set_int("other", "volume_step", step);
	config_save();
	input_set_volume_step(step);
	lv_label_set_text_fmt(volume_step_value, "%d", step);
}

static void screenshot_toggled_cb(lv_event_t *e) {
	(void)e;
	screenshot_set_enabled(lv_obj_has_state(screenshot_switch, LV_STATE_CHECKED));
}

static void build_screenshot_card(lv_obj_t *container);

static void build_other_page(gui_config_t *cfg) {
	othersettings_screen = lv_obj_create(NULL);
	lv_obj_t *container = settingsrow_page(othersettings_screen, cfg, "settings_keys");

	settingsrow_toggle(container, "settings_in_line_remote", &headset_switch, headset_toggled_cb);
	if (headset_controls_enabled()) {
		lv_obj_add_state(headset_switch, LV_STATE_CHECKED);
	}

	settingsrow_add(container, "remap_buttons", NULL, switch_screen_cb, remap_screen);

	settingsrow_slider(container, "settings_volume_steps", VOLUME_STEP_MAX - VOLUME_STEP_MIN + 1, &volume_step_value,
					   &volume_step_slider, volume_step_changed_cb);
	int step = volume_step_setting();
	lv_slider_set_value(volume_step_slider, step - VOLUME_STEP_MIN, LV_ANIM_OFF);
	lv_label_set_text_fmt(volume_step_value, "%d", step);
	input_set_volume_step(step);

	// Screenshots: the option carries the key combination as a fixed subtitle.
	// Without it the switch would enable something without saying how to use it.
	build_screenshot_card(container);

	switcher_attach_back_gesture(othersettings_screen);
}

static void build_keyboard_page(gui_config_t *cfg) {
	keyboard_screen = lv_obj_create(NULL);
	lv_obj_t *container = settingsrow_page(keyboard_screen, cfg, "settings_keyboard");

	// Which alphabets the keyboard can be laid out in, above the shape of the
	// keys: the letters matter more than whether they come ten to a row or
	// three to a key.
	settingsrow_add(container, "keyboard_layout", NULL, switch_screen_cb, kblayoutpage_screen);

	// Keyboard type: the two pills on a card, like the MSEB radius control.
	lv_obj_t *kb_card = lv_obj_create(container);
	lv_obj_set_width(kb_card, lv_pct(100));
	lv_obj_set_height(kb_card, LV_SIZE_CONTENT);
	lv_obj_add_style(kb_card, &theme_style_card, 0);
	lv_obj_set_style_radius(kb_card, 12, 0);
	lv_obj_set_style_border_width(kb_card, 0, 0);
	lv_obj_set_style_shadow_width(kb_card, 0, 0);
	lv_obj_set_style_pad_all(kb_card, 16, 0);
	lv_obj_set_style_pad_row(kb_card, 12, 0);
	lv_obj_set_scrollable(kb_card, false);
	lv_obj_set_flex_flow(kb_card, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(kb_card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

	lv_obj_t *kb_title = lv_label_create(kb_card);
	lv_label_set_text(kb_title, tr("settings_keyboard_type"));
	lv_obj_add_style(kb_title, &theme_style_text, 0);
	lv_obj_set_style_text_font(kb_title, &font_ui_24, 0);

	lv_obj_t *kb_pills = lv_obj_create(kb_card);
	lv_obj_set_width(kb_pills, lv_pct(100));
	lv_obj_set_height(kb_pills, LV_SIZE_CONTENT);
	lv_obj_set_style_bg_opa(kb_pills, 0, 0);
	lv_obj_set_style_border_width(kb_pills, 0, 0);
	lv_obj_set_style_pad_all(kb_pills, 0, 0);
	lv_obj_set_style_pad_column(kb_pills, 8, 0);
	lv_obj_set_scrollable(kb_pills, false);
	lv_obj_set_flex_flow(kb_pills, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(kb_pills, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

	kbtype_buttons[0] = make_kbtype_pill(kb_pills, "QWERTY", 0);
	kbtype_buttons[1] = make_kbtype_pill(kb_pills, "T9", 1);
	refresh_kbtype_buttons();
	theme_register_refresh(refresh_kbtype_buttons);

	switcher_attach_back_gesture(keyboard_screen);
}

static void build_screenshot_card(lv_obj_t *container) {
	lv_obj_t *shot_card = settingsrow_toggle(container, "devoptions_enable_screenshots", &screenshot_switch, screenshot_toggled_cb);
	lv_obj_t *shot_name = lv_obj_get_child(shot_card, 0);
	lv_obj_t *shot_toggle = lv_obj_get_child(shot_card, 1);

	lv_obj_t *shot_hint = lv_label_create(shot_card);
	lv_label_set_text(shot_hint, tr("devoptions_screenshot_keys"));
	lv_obj_set_width(shot_hint, lv_pct(100));
	lv_obj_set_style_pad_right(shot_hint, lv_obj_get_style_pad_right(shot_name, LV_PART_MAIN), 0);
	lv_obj_add_style(shot_hint, &theme_style_text_dim, 0);
	lv_obj_set_style_text_font(shot_hint, &font_ui_18, 0);

	// Name on top, keys at the bottom, in a column the card grows around: the
	// keys take two lines in a long language or with large text, and a row of
	// fixed height would draw them over the name. The row's own height stays
	// the least it is, and the switch keeps to the middle of it.
	int32_t row_height = lv_obj_get_style_height(shot_card, LV_PART_MAIN);
	lv_obj_set_height(shot_card, LV_SIZE_CONTENT);
	lv_obj_set_style_min_height(shot_card, row_height, 0);
	lv_obj_set_flex_flow(shot_card, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(shot_card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(shot_card, 2, 0);
	lv_obj_set_ignore_layout(shot_toggle, true);

	if (screenshot_enabled()) {
		lv_obj_add_state(screenshot_switch, LV_STATE_CHECKED);
	}
}

/* ---- Spotify's Settings: categories, each opening a page of its rows ---- */

static lv_obj_t *root_container;
static lv_obj_t *autoplay_switch;

static lv_obj_t *album_shuffle_switch;
static void album_shuffle_cb(lv_event_t *e) {
	(void)e;
	config_set_bool("player", "shuffle_albums", lv_obj_has_state(album_shuffle_switch, LV_STATE_CHECKED));
	config_save();
	if (playlist_shuffle_enabled()) {
		playlist_reshuffle(); /* what is still to come, dealt the new way */
		device_state_queue_changed();
	}
}

static void autoplay_toggled_cb(lv_event_t *e) {
	(void)e;
	device_state_set_autoplay(lv_obj_has_state(autoplay_switch, LV_STATE_CHECKED));
}

typedef struct {
	const char *name;
	lv_obj_t *screen;
	lv_event_cb_t cb; /* instead of opening a screen */
} hub_row_t;

/* A category page: the rows of what used to be the root's groups. */
static lv_obj_t *hub_container;
static lv_obj_t *hub_page(gui_config_t *cfg, const char *title, const hub_row_t *rows, int n, lv_obj_t **dev_row) {
	lv_obj_t *screen = lv_obj_create(NULL);
	lv_obj_t *container = settingsrow_page(screen, cfg, title);
	hub_container = container;
	settingsrow_title_corner_slots(settingsrow_page_title(screen), cfg, 0);
	switcher_attach_back_gesture(screen);
	for (int i = 0; i < n; i++) {
		lv_obj_t *row = rows[i].cb ? settingsrow_add(container, rows[i].name, NULL, rows[i].cb, NULL)
								   : settingsrow_add(container, rows[i].name, NULL, switch_screen_cb, rows[i].screen);
		if (dev_row && rows[i].screen == devoptions_screen) {
			*dev_row = row;
		}
	}
	return screen;
}

/* ---- Quick tiles: Spotify's Data saver / Offline mode / Private session.
 * The R1 has no data plan and is always offline, so its two radios take
 * those places; Private session is Spotify's. ---- */
enum { QUICK_WIFI, QUICK_BLUETOOTH, QUICK_PRIVATE, QUICK_COUNT };
static struct {
	lv_obj_t *tile, *icon, *name, *state;
} quick[QUICK_COUNT];

static bool quick_available(int kind) {
	return kind == QUICK_WIFI ? wifi_available() : kind == QUICK_BLUETOOTH ? bluetooth_available() : true;
}

static bool quick_on(int kind) {
	return kind == QUICK_WIFI		 ? wifi_get_enabled()
		   : kind == QUICK_BLUETOOTH ? bluetooth_get_enabled()
									 : device_state_private_session();
}

static void quick_paint(void) {
	for (int k = 0; k < QUICK_COUNT; k++) {
		if (!quick[k].tile) {
			continue;
		}
		bool avail = quick_available(k);
		bool on = avail && quick_on(k);
		lv_color_t c = on ? lv_color_hex(0x1ed760) : lv_color_white();
		lv_obj_set_style_image_recolor(quick[k].icon, c, 0);
		lv_obj_set_style_text_color(quick[k].name, c, 0);
		lv_obj_set_style_text_color(quick[k].state, on ? c : lv_color_hex(0xb3b3b3), 0);
		lv_label_set_text(quick[k].state, !avail ? "Not available" : on ? "On" : "Off");
		lv_obj_set_style_opa(quick[k].tile, avail ? LV_OPA_COVER : LV_OPA_50, 0);
	}
}

static void quick_clicked_cb(lv_event_t *e) {
	int kind = (int)(intptr_t)lv_event_get_user_data(e);
	if (!quick_available(kind)) {
		return;
	}
	bool on = !quick_on(kind);
	if (kind == QUICK_WIFI) {
		wifi_set_enabled(on);
	} else if (kind == QUICK_BLUETOOTH) {
		bluetooth_set_enabled(on);
	} else {
		device_state_set_private_session(on);
	}
	quick_paint();
}

static void quick_loaded_cb(lv_event_t *e) {
	(void)e;
	quick_paint();
}

static void quick_tiles(lv_obj_t *parent) {
	static const char *names[QUICK_COUNT] = {"Wi-Fi", "Bluetooth", "Private session"};
	const void *icons[QUICK_COUNT] = {&icon_wifi, &icon_bluetooth, &icon_eye_off};
	lv_obj_t *row = lv_obj_create(parent);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, lv_pct(100), 94);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 8, 0);
	lv_obj_set_style_margin_bottom(row, 6, 0);
	lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_remove_flag(row, LV_OBJ_FLAG_CLICKABLE);
	for (int k = 0; k < QUICK_COUNT; k++) {
		lv_obj_t *t = lv_btn_create(row);
		lv_obj_set_height(t, 94);
		lv_obj_set_flex_grow(t, 1);
		lv_obj_set_style_bg_color(t, lv_color_hex(0x242424), 0);
		lv_obj_set_style_bg_opa(t, LV_OPA_COVER, 0);
		lv_obj_set_style_radius(t, 8, 0);
		lv_obj_set_style_border_width(t, 0, 0);
		lv_obj_set_style_shadow_width(t, 0, 0);
		lv_obj_set_style_pad_all(t, 0, 0);
		lv_obj_add_event_cb(t, quick_clicked_cb, LV_EVENT_CLICKED, (void *)(intptr_t)k);
		quick[k].tile = t;
		quick[k].icon = lv_image_create(t);
		lv_image_set_src(quick[k].icon, icons[k]);
		lv_obj_set_size(quick[k].icon, 28, 28);
		lv_image_set_inner_align(quick[k].icon, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_style_image_recolor_opa(quick[k].icon, LV_OPA_COVER, 0);
		lv_obj_align(quick[k].icon, LV_ALIGN_TOP_MID, 0, 10);
		quick[k].name = lv_label_create(t);
		lv_label_set_text(quick[k].name, names[k]);
		lv_obj_set_style_text_font(quick[k].name, &font_ui_16, 0);
		lv_obj_align(quick[k].name, LV_ALIGN_TOP_MID, 0, 44);
		quick[k].state = lv_label_create(t);
		lv_obj_set_style_text_font(quick[k].state, &font_ui_16, 0);
		lv_obj_align(quick[k].state, LV_ALIGN_TOP_MID, 0, 66);
	}
	quick_paint();
	lv_obj_add_event_cb(settings_screen, quick_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
}

/* ---- Search settings: Spotify's magnifier in the Settings header opens a
 * page that finds a setting by its name or by what it is about. ---- */
typedef struct {
	const char *name;  /* shown (a translation key or plain text) */
	const char *where; /* the category it lives in */
	const char *about; /* other words it answers to */
	lv_obj_t *screen;
	lv_event_cb_t cb;
} find_entry_t;
#define FIND_MAX 32
static find_entry_t finds[FIND_MAX];
static int find_count;
static lv_obj_t *find_screen, *find_field, *find_list, *find_empty;
static keyboard_t *find_keyboard;
static gui_config_t *find_cfg;

static void find_add(const char *name, const char *where, const char *about, lv_obj_t *screen, lv_event_cb_t cb) {
	if (find_count < FIND_MAX) {
		finds[find_count++] = (find_entry_t){name, where, about, screen, cb};
	}
}

/* Case-insensitive, ASCII only: the setting names are. */
static bool find_contains(const char *hay, const char *needle) {
	size_t n = strlen(needle);
	for (const char *h = hay; *h; h++) {
		size_t i = 0;
		while (i < n && h[i] && tolower((unsigned char)h[i]) == tolower((unsigned char)needle[i])) {
			i++;
		}
		if (i == n) {
			return true;
		}
	}
	return n == 0;
}

static void find_open_cb(lv_event_t *e) {
	find_entry_t *f = lv_event_get_user_data(e);
	keyboard_set_visible(find_keyboard, false);
	if (f->cb) {
		f->cb(e);
	} else if (f->screen) {
		switch_screen(f->screen);
	}
}

static void find_refresh(void) {
	const char *q = lv_textarea_get_text(find_field);
	while (*q == ' ') {
		q++;
	}
	lv_obj_clean(find_list);
	int shown = 0;
	if (q[0]) {
		for (int i = 0; i < find_count; i++) {
			find_entry_t *f = &finds[i];
			if (f->screen == devoptions_screen && !systempage_devoptions_unlocked()) {
				continue;
			}
			if (find_contains(tr(f->name), q) || find_contains(f->where, q) || find_contains(f->about, q)) {
				settingsrow_category(find_list, NULL, f->name, f->where, find_open_cb, f);
				shown++;
			}
		}
	}
	lv_obj_set_hidden(find_empty, shown > 0);
	lv_label_set_text(lv_obj_get_child(find_empty, 0), q[0] ? "No settings found" : "Search settings");
	lv_label_set_text(lv_obj_get_child(find_empty, 1),
					  q[0] ? "Try another word" : "Find settings by name or description");
}

static void find_changed_cb(lv_event_t *e) {
	(void)e;
	find_refresh();
}

static void find_accept_cb(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(find_keyboard, false);
}

static void find_field_clicked_cb(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(find_keyboard, true);
}

static void find_loaded_cb(lv_event_t *e) {
	(void)e;
	keyboard_set_visible(find_keyboard, true);
}

static void find_build(void) {
	gui_config_t *cfg = find_cfg;
	find_screen = lv_obj_create(NULL);
	lv_obj_add_style(find_screen, &theme_style_screen, 0);
	settingsrow_title(find_screen, cfg, "Search");
	int top = settingsrow_content_top(cfg);
	find_field = lv_textarea_create(find_screen);
	lv_textarea_set_one_line(find_field, true);
	lv_textarea_set_placeholder_text(find_field, "Search settings");
	lv_obj_set_size(find_field, cfg->screen_width - 2 * cfg->padding, 56);
	lv_obj_align(find_field, LV_ALIGN_TOP_LEFT, cfg->padding, top);
	lv_obj_set_scrollbar_mode(find_field, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_bg_color(find_field, lv_color_hex(0x2a2a2a), 0);
	lv_obj_set_style_bg_opa(find_field, LV_OPA_COVER, 0);
	lv_obj_set_style_radius(find_field, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_border_width(find_field, 1, 0);
	lv_obj_set_style_border_color(find_field, lv_color_hex(0x3e3e3e), 0);
	lv_obj_set_style_shadow_width(find_field, 0, 0);
	lv_obj_set_style_pad_ver(find_field, 14, 0);
	lv_obj_set_style_pad_left(find_field, 52, 0);
	lv_obj_set_style_pad_right(find_field, 16, 0);
	lv_obj_set_style_text_font(find_field, &font_ui_20, 0);
	lv_obj_set_style_text_color(find_field, lv_color_white(), 0);
	keyboard_style_caret(find_field);
	lv_obj_add_event_cb(find_field, find_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(find_field, find_field_clicked_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t *glass = lv_image_create(find_screen);
	lv_image_set_src(glass, &icon_search);
	lv_obj_add_style(glass, &theme_style_icon, 0);
	lv_obj_set_style_image_opa(glass, LV_OPA_70, 0);
	lv_obj_set_size(glass, 26, 26);
	lv_image_set_inner_align(glass, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_align(glass, LV_ALIGN_TOP_LEFT, cfg->padding + 16, top + (56 - 26) / 2);

	int list_top = top + 56 + 8;
	find_list = lv_obj_create(find_screen);
	lv_obj_remove_style_all(find_list);
	lv_obj_set_size(find_list, cfg->screen_width, cfg->screen_height - list_top - 316);
	lv_obj_align(find_list, LV_ALIGN_TOP_LEFT, 0, list_top);
	lv_obj_set_style_pad_hor(find_list, cfg->padding, 0);
	lv_obj_set_flex_flow(find_list, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(find_list, 2, 0);

	find_empty = lv_obj_create(find_screen);
	lv_obj_remove_style_all(find_empty);
	lv_obj_set_size(find_empty, cfg->screen_width, 70);
	lv_obj_align(find_empty, LV_ALIGN_TOP_MID, 0, list_top + 40);
	lv_obj_remove_flag(find_empty, LV_OBJ_FLAG_CLICKABLE);
	for (int i = 0; i < 2; i++) {
		lv_obj_t *l = lv_label_create(find_empty);
		lv_obj_set_width(l, lv_pct(100));
		lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_add_style(l, i ? &theme_style_text_dim : &theme_style_text, 0);
		lv_obj_set_style_text_font(l, i ? &font_ui_16 : &font_ui_20_bold, 0);
		lv_obj_align(l, LV_ALIGN_TOP_MID, 0, i * 30);
	}

	find_keyboard = keyboard_create(find_screen, cfg->screen_width, 316, find_field, &icon_search, NULL,
									find_accept_cb, NULL);
	lv_obj_add_event_cb(find_screen, find_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
	switcher_attach_back_gesture(find_screen);
	find_refresh();
}

static void find_clicked_cb(lv_event_t *e) {
	(void)e;
	if (!find_screen) {
		find_build();
	}
	lv_textarea_set_text(find_field, "");
	find_refresh();
	switch_screen(find_screen);
}

lv_obj_t *settings_search_screen(void) { return find_screen; }

static void search_button(gui_config_t *cfg) {
	lv_obj_t *btn = lv_btn_create(settings_screen);
	lv_obj_set_size(btn, 56, 56);
	lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -cfg->padding, cfg->top_bar_height + cfg->padding);
	lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_color(btn, lv_color_hex(0x1f1f1f), 0);
	lv_obj_set_style_bg_opa(btn, LV_OPA_90, 0);
	lv_obj_set_style_border_width(btn, 0, 0);
	lv_obj_set_style_shadow_width(btn, 0, 0);
	lv_obj_add_event_cb(btn, find_clicked_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t *icon = lv_image_create(btn);
	lv_image_set_src(icon, &icon_search);
	lv_obj_add_style(icon, &theme_style_icon, 0);
	lv_obj_center(icon);
}

/* ---- Library health: an R1 page (Spotify has no files to look after).
 * Read-only counts of what makes a card library hard to browse, each opening
 * a few examples; nothing is changed or deleted from here. ---- */
static lv_obj_t *health_screen, *health_list;
static const char *const HEALTH_NAMES[LIBRARY_HEALTH_KINDS] = {"Missing artist or album", "No genre",
																"Possible duplicates", "Lossy files"};
static void health_examples_cb(lv_event_t *e) {
	int kind = (int)(intptr_t)lv_event_get_user_data(e);
	static char paths[20][512];
	int n = library_health_examples(kind, paths, 20);
	lv_obj_t *body = bottomsheet_open(560, NULL);
	lv_obj_set_style_pad_hor(body, 20, 0);
	lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(body, 6, 0);
	lv_obj_t *t = lv_label_create(body);
	lv_label_set_text(t, HEALTH_NAMES[kind]);
	lv_obj_set_style_text_font(t, &font_ui_22_bold, 0);
	lv_obj_set_style_text_color(t, lv_color_white(), 0);
	for (int i = 0; i < n; i++) {
		/* The file name over its folder: enough to find it on the card. */
		char *slash = strrchr(paths[i], '/');
		lv_obj_t *name = lv_label_create(body);
		lv_label_set_text(name, slash ? slash + 1 : paths[i]);
		lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
		lv_obj_set_width(name, lv_pct(100));
		lv_obj_set_style_text_font(name, &font_ui_18, 0);
		lv_obj_set_style_text_color(name, lv_color_white(), 0);
		if (slash) {
			*slash = '\0';
			char *up = strrchr(paths[i], '/');
			lv_obj_t *dir = lv_label_create(body);
			lv_label_set_text(dir, up ? up + 1 : paths[i]);
			lv_label_set_long_mode(dir, LV_LABEL_LONG_DOT);
			lv_obj_set_width(dir, lv_pct(100));
			lv_obj_set_style_text_font(dir, &font_ui_14, 0);
			lv_obj_set_style_text_color(dir, lv_color_hex(0xb3b3b3), 0);
		}
	}
}
static void health_loaded_cb(lv_event_t *e) {
	(void)e;
	lv_obj_clean(health_list);
	lv_obj_t *note = lv_label_create(health_list);
	lv_label_set_text(note, "What could make your library harder to browse. Nothing here changes your files.");
	lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(note, lv_pct(100));
	lv_obj_add_style(note, &theme_style_text_dim, 0);
	lv_obj_set_style_text_font(note, settingsrow_note_font(), 0);
	lv_obj_set_style_pad_bottom(note, 8, 0);
	for (int k = 0; k < LIBRARY_HEALTH_KINDS; k++) {
		int n = library_health_count(k);
		char sub[64];
		snprintf(sub, sizeof(sub), n == 0 ? "None" : n == 1 ? "1 song" : "%d songs", n);
		settingsrow_category(health_list, NULL, HEALTH_NAMES[k], sub, health_examples_cb, (void *)(intptr_t)k);
	}
}
static void health_build(gui_config_t *cfg) {
	health_screen = lv_obj_create(NULL);
	health_list = settingsrow_page(health_screen, cfg, "Library health");
	settingsrow_title_corner_slots(settingsrow_page_title(health_screen), cfg, 0);
	switcher_attach_back_gesture(health_screen);
	lv_obj_add_event_cb(health_screen, health_loaded_cb, LV_EVENT_SCREEN_LOADED, NULL);
}

/* The device's card at the top of Settings, as the account's heads iOS and
 * Spotify settings: its picture and name, and the way to change them. */
static void profile_card_cb(lv_event_t *e) {
	(void)e;
	switch_screen(sysinfo_screen);
}
static void profile_card(lv_obj_t *parent) {
	lv_obj_t *card = lv_obj_create(parent);
	lv_obj_remove_style_all(card);
	lv_obj_set_size(card, lv_pct(100), 84);
	lv_obj_set_style_radius(card, 12, 0);
	lv_obj_set_style_bg_color(card, lv_color_hex(0x242424), 0);
	lv_obj_set_style_bg_color(card, lv_color_hex(0x333333), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
	lv_obj_set_scrollable(card, false);
	lv_obj_set_clickable(card, true);
	lv_obj_add_event_cb(card, profile_card_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t *av = lv_obj_create(card);
	lv_obj_remove_style_all(av);
	lv_obj_set_size(av, 60, 60);
	lv_obj_set_pos(av, 12, 12);
	lv_obj_set_clickable(av, false);
	deviceprofile_avatar(av, 60, &font_ui_24_bold);
	lv_obj_update_layout(parent);
	int text_w = lv_obj_get_content_width(parent) - 110;
	lv_obj_t *name = lv_label_create(card);
	lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
	lv_obj_set_size(name, text_w, lv_font_get_line_height(&font_ui_24_bold));
	lv_obj_set_style_text_font(name, &font_ui_24_bold, 0);
	lv_obj_set_style_text_color(name, lv_color_white(), 0);
	lv_obj_set_pos(name, 86, 14);
	deviceprofile_name_label(name);
	lv_obj_t *sub = lv_label_create(card);
	lv_label_set_text(sub, "Name, picture and device info");
	lv_label_set_long_mode(sub, LV_LABEL_LONG_DOT);
	lv_obj_set_size(sub, text_w, lv_font_get_line_height(&font_ui_16));
	lv_obj_set_style_text_font(sub, &font_ui_16, 0);
	lv_obj_set_style_text_color(sub, lv_color_hex(0xb3b3b3), 0);
	lv_obj_set_pos(sub, 86, 48);
	lv_obj_t *chev = lv_image_create(card);
	lv_image_set_src(chev, &icon_chevron_right);
	lv_obj_set_style_image_recolor(chev, lv_color_hex(0xb3b3b3), 0);
	lv_obj_set_style_image_recolor_opa(chev, LV_OPA_COVER, 0);
	lv_obj_align(chev, LV_ALIGN_RIGHT_MID, -12, 0);
}

void settings_build_categories(gui_config_t *cfg) {
	if (!root_container) {
		return;
	}
	lv_obj_t *c = root_container;
	find_cfg = cfg;
	const hub_row_t display[] = {{"appearance", appearance_screen, NULL},
								 {"settings_screen", screensettings_screen, NULL},
								 {"language", language_screen, NULL},
								 {"settings_keyboard", keyboard_screen, NULL}};
	const hub_row_t playback[] = {{"musicsettings_crossfade_gapless", musicsettings_fade_screen(), NULL},
								  {"musicsettings_playback_queue", musicsettings_playback_screen(), NULL},
								  {"musicsettings_now_playing", musicsettings_nowplaying_screen(), NULL}};
	health_build(cfg);
	const hub_row_t library[] = {{"musicsettings_scan_options", musicsettings_scan_screen(), NULL},
								 {"musicsettings_library", musicsettings_library_screen(), NULL},
								 {"Library health", health_screen, NULL},
								 {"lastfm", lastfmsettings_screen(), NULL}};
	const hub_row_t controls[] = {{"settings_keys", othersettings_screen, NULL},
								  {"control_centre", ccsettings_screen, NULL}};
	storagepage_init(cfg);
	const hub_row_t device[] = {{"date_and_time", NULL, clock_clicked_cb},
								{"Storage", storagepage_screen, NULL},
								{"power", powersettings_screen, NULL},
								{"system", systempage_screen, NULL},
								{"developer_options", devoptions_screen, NULL}};
#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))
	lv_obj_t *display_page = hub_page(cfg, "Content and display", display, N(display), NULL);
	lv_obj_t *playback_page = hub_page(cfg, "Playback", playback, N(playback), NULL);
	/* Spotify's Autoplay switch, with its explanation under it. */
	settingsrow_toggle(hub_container, "Autoplay", &autoplay_switch, autoplay_toggled_cb);
	lv_obj_t *note = lv_label_create(hub_container);
	lv_label_set_text(note, "Keep listening to similar songs from your library when your music ends.");
	lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(note, lv_pct(100));
	lv_obj_add_style(note, &theme_style_text_dim, 0);
	lv_obj_set_style_text_font(note, settingsrow_note_font(), 0);
	lv_obj_set_style_pad_hor(note, 4, 0);
	if (device_state_autoplay()) {
		lv_obj_add_state(autoplay_switch, LV_STATE_CHECKED);
	}
	/* Album shuffle (an R1 addition): shuffle whole albums, each in order. */
	settingsrow_toggle(hub_container, "Shuffle by album", &album_shuffle_switch, album_shuffle_cb);
	lv_obj_t *anote = lv_label_create(hub_container);
	lv_label_set_text(anote, "Shuffle plays whole albums in track order, the albums in random order.");
	lv_label_set_long_mode(anote, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(anote, lv_pct(100));
	lv_obj_add_style(anote, &theme_style_text_dim, 0);
	lv_obj_set_style_text_font(anote, settingsrow_note_font(), 0);
	lv_obj_set_style_pad_hor(anote, 4, 0);
	if (config_get_bool("player", "shuffle_albums", false)) {
		lv_obj_add_state(album_shuffle_switch, LV_STATE_CHECKED);
	}
	lv_obj_t *library_page = hub_page(cfg, "Your Library", library, N(library), NULL);
	lv_obj_t *controls_page = hub_page(cfg, "Controls", controls, N(controls), NULL);
	lv_obj_t *device_page = hub_page(cfg, "Device", device, N(device), &devoptions_row);

	/* What Search settings can find, category by category. */
	const struct {
		const hub_row_t *rows;
		int n;
		const char *where;
	} groups[] = {{display, N(display), "Content and display"}, {playback, N(playback), "Playback"},
				  {library, N(library), "Your Library"},		{controls, N(controls), "Controls"},
				  {device, N(device), "Device"}};
#undef N
	for (unsigned g = 0; g < sizeof(groups) / sizeof(groups[0]); g++) {
		for (int i = 0; i < groups[g].n; i++) {
			find_add(groups[g].rows[i].name, groups[g].where, "", groups[g].rows[i].screen, groups[g].rows[i].cb);
		}
	}
	find_add("Autoplay", "Playback", "similar songs end continue", playback_page, NULL);
	find_add("Audio quality", "Settings",
			 "equalizer eq replaygain volume normalization dac bit-perfect gain filter mseb crossfeed balance",
			 musicsettings_screen, NULL);
	find_add("Apps and devices", "Settings", "wi-fi wifi bluetooth airplay transfer dlna headphones",
			 wireless_screen, NULL);
	find_add("Private session", "Settings", "history recents stats scrobble", settings_screen, NULL);
	find_add("Update from SD card", "Device", "firmware update upgrade upt system", systempage_screen, NULL);

	profile_card(c);
	quick_tiles(c);
	search_button(cfg);
	/* Spotify's order: what you see, how it plays, the sound, then the rest. */
	settingsrow_category(c, &sonix_icon_music, "Content and display",
						 "Appearance \xc2\xb7 Screen \xc2\xb7 Language \xc2\xb7 Keyboard", switch_screen_cb, display_page);
	settingsrow_category(c, &sonix_icon_volume, "Playback", "Crossfade \xc2\xb7 Gapless \xc2\xb7 Queue \xc2\xb7 Autoplay",
						 switch_screen_cb, playback_page);
	settingsrow_category(c, &sonix_icon_stats, "Audio quality",
						 "Equalizer \xc2\xb7 Normalization \xc2\xb7 Bit-perfect \xc2\xb7 DAC", switch_screen_cb,
						 musicsettings_screen);
	settingsrow_category(c, &sonix_icon_library, "Your Library", "Scan \xc2\xb7 Artists and genres \xc2\xb7 Last.fm",
						 switch_screen_cb, library_page);
	settingsrow_category(c, &sonix_icon_speaker, "Apps and devices",
						 "Wi-Fi \xc2\xb7 Bluetooth \xc2\xb7 AirPlay \xc2\xb7 Transfer", switch_screen_cb, wireless_screen);
	settingsrow_category(c, &sonix_icon_controls, "Controls",
						 "Buttons \xc2\xb7 Volume steps \xc2\xb7 Control centre", switch_screen_cb, controls_page);
	settingsrow_category(c, &sonix_icon_gear, "Device", "Date and time \xc2\xb7 Storage \xc2\xb7 Power \xc2\xb7 System",
						 switch_screen_cb, device_page);
	settings_refresh_devoptions();
}

void settings_init(gui_config_t *cfg) {
	lv_obj_t *container = settingsrow_page(settings_screen, cfg, "settings");

	build_screen_page(cfg);
	build_other_page(cfg);
	build_keyboard_page(cfg);

	// The reference layout lists Spotify's categories instead, once every page
	// they open exists: settings_build_categories(), called after the music
	// settings are built.
	if (theme_reference()) {
		root_container = container;
		return;
	}

	// In groups, like Music: how it looks, how it is driven, the device.
	settingsrow_heading(container, "settings_section_personal");
	settingsrow_add(container, "appearance", NULL, switch_screen_cb, appearance_screen);
	settingsrow_add(container, "settings_screen", NULL, switch_screen_cb, screensettings_screen);
	language_bind_menu_row(settingsrow_add(container, "language", NULL, switch_screen_cb, language_screen));

	settingsrow_heading(container, "settings_section_controls");
	settingsrow_add(container, "settings_keys", NULL, switch_screen_cb, othersettings_screen);
	settingsrow_add(container, "control_centre", NULL, switch_screen_cb, ccsettings_screen);
	settingsrow_add(container, "settings_keyboard", NULL, switch_screen_cb, keyboard_screen);

	settingsrow_heading(container, "settings_section_device");
	settingsrow_add(container, "date_and_time", NULL, clock_clicked_cb, NULL);
	settingsrow_add(container, "power", NULL, switch_screen_cb, powersettings_screen);
	settingsrow_add(container, "system", NULL, switch_screen_cb, systempage_screen);
	// Hidden until the build number in System > Info is tapped five times.
	// Built regardless, so unlocking is a visibility change rather than a
	// rebuild of the page.
	devoptions_row = settingsrow_add(container, "developer_options", NULL, switch_screen_cb, devoptions_screen);
	settings_refresh_devoptions();
}
