#include "ui_probe.h"
#ifdef HOST_BUILD
#include "bottomsheet.h"
#include "lvgl/lvgl.h"
#include "main_menu.h"
#include "src/gui/library/medialist.h"
#include "src/gui/library/playlistview.h"
#include "src/gui/nowplaying/player.h"
#include "src/system/discovery/discovery.h"
extern void main_menu_probe_online_track(const discovery_item_t *item);
#include "src/system/core/config.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char directory[768];
static int nodes;
static void string(FILE *f, const char *s) {
	fputc('"', f);
	for (const unsigned char *c = (const unsigned char *)s; *c; c++) {
		if (*c == '"' || *c == '\\')
			fprintf(f, "\\%c", *c);
		else if (*c < 32)
			fprintf(f, "\\u%04x", *c);
		else
			fputc(*c, f);
	}
	fputc('"', f);
}
static void walk(FILE *f, lv_obj_t *o, int parent, bool hidden) {
	if (!o || nodes >= 2048)
		return;
	int id = nodes++;
	hidden = hidden || lv_obj_is_hidden(o);
	lv_area_t r;
	lv_obj_get_coords(o, &r);
	if (id)
		fputc(',', f);
	fprintf(f, "{\"id\":%d,\"parent\":%d,\"hidden\":%s,\"clickable\":%s,\"rect\":[%d,%d,%d,%d],\"type\":", id, parent, hidden ? "true" : "false", lv_obj_is_clickable(o) ? "true" : "false", r.x1, r.y1, r.x2 + 1, r.y2 + 1);
	bool label = lv_obj_check_type(o, &lv_label_class);
	string(f, label ? "label" : lv_obj_check_type(o, &lv_image_class) ? "image" : "object");
	if (label) {
		fputs(",\"text\":", f);
		string(f, lv_label_get_text(o));
	}
	fputc('}', f);
	uint32_t count = lv_obj_get_child_count(o);
	for (uint32_t i = 0; i < count; i++)
		walk(f, lv_obj_get_child(o, i), id, hidden);
}
static void probe_poll(lv_timer_t *timer) {
	(void)timer;
	char path[1024], command[1024];
	snprintf(path, sizeof(path), "%s/request", directory);
	FILE *f = fopen(path, "r");
	if (!f)
		return;
	if (!fgets(command, sizeof(command), f))
		command[0] = 0;
	fclose(f);
	unlink(path);
	command[strcspn(command, "\r\n")] = 0;
	/* Setup only; test interactions are delivered through SDL input. */
	if (!strncmp(command, "online-fixture ", 15)) {
        /* Offline metadata fixture; the real parser and real sheet render it. */
        FILE *fixture=fopen(command+15,"r");
        if(fixture) {
            char json[8192];size_t len=fread(json,1,sizeof(json)-1,fixture);json[len]=0;fclose(fixture);
            discovery_results_t *results=calloc(1,sizeof(*results));char error[192];
            if(results && discovery_parse_album("sonix-ui-fixture",json,results,error,sizeof(error)) && results->count)
                main_menu_probe_online_track(&results->items[0]);
            free(results);
        }
    } else if (!strncmp(command, "playlist ", 9))
		playlistview_open(command + 9, false);
	else if (!strncmp(command, "album ", 6)) {
		char album[512];
		const char *value = command + 6;
		if (library_track_album_value(value, album, sizeof(album)))
			value = album;
		medialist_open(value, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, value);
	}
	lv_obj_update_layout(lv_screen_active());
	lv_obj_update_layout(lv_layer_top());
	device_state_t state;
	device_state_get(&state);
	snprintf(path, sizeof(path), "%s/response.tmp", directory);
	f = fopen(path, "w");
	if (!f)
		return;
	fputs("{\"request\":", f);
	string(f, command);
	fprintf(f, ",\"lvgl\":\"%d.%d.%d\",\"route\":", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
	string(f, lv_screen_active() == medialist_tracks_screen ? "collection" : lv_screen_active() == main_menu_screen ? "main" : "other");
	fprintf(f, ",\"modal\":%s,\"status\":%d,\"seconds\":%.3f,\"shuffle\":%s,\"repeat\":%d,\"priority\":%d,\"tab\":%d,\"file\":", bottomsheet_active() ? "true" : "false", state.status, state.progress_current_secs, playlist_shuffle_enabled() ? "true" : "false", playlist_repeat_mode(), playlist_priority_count(), (int)config_get_int("ui", "last_tab", 0));
	string(f, state.current_file);
	fprintf(f, ",\"now_playing_open\":%s", player_sheet_is_open() ? "true" : "false");
	fprintf(f, ",\"library_filter\":%d,\"library_sort\":%d,\"library_grid\":%d", (int)config_get_int("collections", "filter", 0), (int)config_get_int("collections", "sort", 0), (int)config_get_int("collections", "grid", 0));
	fputs(",\"queue\":[", f);
	for (int i = 0; i < playlist_count() && i < 512; i++) {
		char track[512] = "";
		playlist_path_at(i, track, sizeof(track));
		if (i)
			fputc(',', f);
		string(f, track);
	}
	fputs("],\"nodes\":[", f);
	nodes = 0;
	walk(f, lv_screen_active(), -1, false);
	walk(f, lv_layer_top(), -1, false);
	fprintf(f, "],\"node_count\":%d}", nodes);
	fclose(f);
	char dest[1024];
	snprintf(dest, sizeof(dest), "%s/response.json", directory);
	rename(path, dest);
}
void ui_probe_init(void) {
	const char *dir = getenv("SONIX_UI_PROBE_DIR");
	if (!dir || !*dir)
		return;
	snprintf(directory, sizeof(directory), "%s", dir);
	lv_timer_create(probe_poll, 50, NULL);
}
#else
void ui_probe_init(void) {}
#endif
