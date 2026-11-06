#include "outputprofile.h"

#include <stdio.h>

#include "lvgl/lvgl.h"
#include "src/gui/settings/musicsettings.h"
#include "src/gui/shell/toast.h"
#include "src/system/audio/eq.h"
#include "src/system/bluetooth/bluetooth.h"
#include "src/system/core/config.h"

/* Sound per output (an R1 addition; Spotify has one sound for everything).
 * Wired IEMs and Bluetooth headphones rarely want the same tuning, so when
 * the output changes the switches and graphic EQ bands in use are kept for
 * the output being left, under [output_wired] / [output_bluetooth], and the
 * ones last kept for the new output are put back. A new output with nothing
 * kept yet simply carries on with what was playing. */

enum { OUT_WIRED, OUT_BLUETOOTH };
static int current = -1;
static int forced = -1;

static int output_now(void) {
	if (forced >= 0) {
		return forced;
	}
	return bluetooth_audio_active() ? OUT_BLUETOOTH : OUT_WIRED;
}

static const char *section(int out) { return out == OUT_BLUETOOTH ? "output_bluetooth" : "output_wired"; }

static void keep(int out) {
	const char *s = section(out);
	config_set_bool(s, "eq", eq_get_enabled());
	config_set_bool(s, "peq", peq_get_enabled());
	config_set_bool(s, "mseb", mseb_get_enabled());
	config_set_bool(s, "crossfeed", crossfeed_get_enabled());
	for (int i = 0; i < EQ_BANDS; i++) {
		char key[8];
		snprintf(key, sizeof(key), "b%d", i);
		config_set_int(s, key, eq_get_band_tenths(i));
	}
	config_set_bool(s, "kept", true);
	config_save();
}

static bool restore(int out) {
	const char *s = section(out);
	if (!config_get_bool(s, "kept", false)) {
		return false;
	}
	for (int i = 0; i < EQ_BANDS; i++) {
		char key[8];
		snprintf(key, sizeof(key), "b%d", i);
		eq_set_band_tenths(i, config_get_int(s, key, eq_get_band_tenths(i)));
	}
	musicsettings_set_eq_enabled(config_get_bool(s, "eq", eq_get_enabled()));
	peq_set_enabled(config_get_bool(s, "peq", peq_get_enabled()));
	musicsettings_set_mseb_enabled(config_get_bool(s, "mseb", mseb_get_enabled()));
	crossfeed_set_enabled(config_get_bool(s, "crossfeed", crossfeed_get_enabled()));
	return true;
}

static void tick_cb(lv_timer_t *t) {
	(void)t;
	int now = output_now();
	if (current < 0) {
		current = now;
		return;
	}
	if (now == current) {
		return;
	}
	keep(current);
	current = now;
	if (restore(now)) {
		toast_plain(now == OUT_BLUETOOTH ? "Sound for Bluetooth" : "Sound for wired");
	}
}

void outputprofile_init(void) { lv_timer_create(tick_cb, 1000, NULL); }

const char *outputprofile_name(void) { return output_now() == OUT_BLUETOOTH ? "Bluetooth" : "Wired"; }

void outputprofile_force(int output) { forced = output; }
