#include "main_menu.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "src/gui/fonts/fonts.h"
#include "src/gui/library/audiobooks.h"
#include "src/gui/library/browser.h"
#include "src/gui/library/medialist.h"
#include "src/gui/library/music.h"
#include "src/gui/library/playlistpage.h"
#include "src/gui/library/search.h"
#include "src/gui/nowplaying/player.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/settings/settings.h"
#include "src/gui/shell/gridpage.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/shell/morepage.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/gui/streaming/streaming.h"
#include "src/gui/wireless/wifisettings.h"
#include "src/gui/wireless/wireless.h"
#include "src/system/core/lang.h"
#include "src/system/core/config.h"
#include "src/system/discovery/discovery.h"
#include "src/system/discovery/download_queue.h"
#include "src/system/library/library.h"
#include "src/system/net/wifi.h"
#include "src/system/playback/device_state.h"

/* Original Sonix branding and native widgets. Legacy pages retain their own
 * layouts, history and transport owners. Primary pages and the keyboard are
 * created on demand. One bounded catalogue request runs at a time; workers
 * never touch LVGL. Download completion/indexing is owned by the application. */
lv_obj_t *main_menu_screen;
static lv_obj_t *tools_screen;
static gui_config_t *cfg;
static lv_obj_t *page[5], *heading, *nav_icon[3], *nav_label[3];
static lv_obj_t *mini, *mini_title, *mini_artist, *mini_art, *mini_play, *mini_progress;
static int selected;
static cover_image_t mini_image;
static char mini_path[512];
static bool mini_has_art;
static unsigned home_revision;
static long long home_history;

enum { HOME, SEARCH, LIBRARY, DOWNLOADS, PROVIDERS };
enum { FILTER_ALL, FILTER_LOCAL, FILTER_ONLINE };
enum { OPEN_FAVOURITES, OPEN_ALBUMS, OPEN_ARTISTS, OPEN_TRACKS, OPEN_PLAYLISTS,
       OPEN_BROWSE, OPEN_MOST_PLAYED, OPEN_TOOLS, OPEN_DOWNLOADS, OPEN_SETTINGS,
       OPEN_CLASSIC_LIBRARY, OPEN_LOCAL_SEARCH, OPEN_WIFI, OPEN_STREAMING,
       OPEN_DISCOVERY, OPEN_DOWNLOAD_FOLDER, OPEN_PROVIDERS };

#define PAD 20
#define NAV_H 74
#define MINI_H 76
#define HEADER_H 74
#define LOCAL_HITS 18

static lv_color_t ink(void) { return theme()->text_primary; }
static lv_color_t quiet(void) { return theme()->text_secondary; }
static lv_color_t mint(void) { return lv_color_hex(theme_is_dark() ? 0x63dfb2 : 0x087954); }
static bool tapped(void) { return !player_sheet_drag_active() && !switcher_back_drag_active(); }
static int body_height(void) { return cfg->screen_height - cfg->top_bar_height - HEADER_H - NAV_H - MINI_H - 14; }
static void show_page(int tab);

static lv_obj_t *box(lv_obj_t *parent, int w, int h, bool surface) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, w, h);
    lv_obj_add_style(o, surface ? &theme_style_card : &theme_style_screen, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_radius(o, surface ? 12 : 0, 0);
    lv_obj_set_scrollable(o, false);
    return o;
}

static lv_obj_t *label(lv_obj_t *parent, const char *text, const lv_font_t *font, int w, bool dim) {
    lv_obj_t *o = lv_label_create(parent);
    lv_obj_add_style(o, dim ? &theme_style_text_dim : &theme_style_text, 0);
    lv_obj_set_style_text_font(o, font, 0);
    lv_obj_set_width(o, w);
    lv_label_set_text(o, text);
    lv_label_set_long_mode(o, LV_LABEL_LONG_DOT);
    return o;
}

static lv_obj_t *glyph(lv_obj_t *parent, const lv_image_dsc_t *src, int size) {
    lv_obj_t *o = lv_image_create(parent);
    lv_image_set_src(o, src);
    lv_obj_set_size(o, size, size);
    lv_image_set_inner_align(o, LV_IMAGE_ALIGN_STRETCH);
    lv_obj_add_style(o, &theme_style_icon, 0);
    lv_obj_set_style_image_recolor_opa(o, LV_OPA_COVER, 0);
    return o;
}

static lv_obj_t *button(lv_obj_t *parent, const char *title, int w, int h,
                        lv_event_cb_t cb, void *data) {
    lv_obj_t *o = box(parent, w, h, true);
    lv_obj_set_clickable(o, true);
    lv_obj_add_style(o, &theme_style_card_pressed, LV_STATE_PRESSED);
    lv_obj_set_event_bubble(o, true);
    if (title) {
        lv_obj_t *l = label(o, title, &font_ui_18, w - 20, false);
        lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(l);
    }
    lv_obj_add_event_cb(o, cb, LV_EVENT_CLICKED, data);
    return o;
}

static lv_obj_t *column(lv_obj_t *parent, int w, int h) {
    lv_obj_t *o = box(parent, w, h, false);
    lv_obj_set_flex_flow(o, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(o, 12, 0);
    lv_obj_set_scrollable(o, true);
    lv_obj_set_scroll_dir(o, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_AUTO);
    return o;
}

static void section(lv_obj_t *parent, const char *title) {
    label(parent, title, &font_ui_24_bold, cfg->screen_width - 2 * PAD, false);
}

static void action_cb(lv_event_t *e) {
    if (!tapped()) return;
    switch ((intptr_t)lv_event_get_user_data(e)) {
    case OPEN_FAVOURITES: medialist_open(tr("favourites"), LIBRARY_LIST_FAVOURITES, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_ALBUMS: medialist_open(tr("albums"), LIBRARY_LIST_ALBUMS, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_ARTISTS: medialist_open(tr("artists"), LIBRARY_LIST_ARTISTS, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_TRACKS: medialist_open(tr("music_all_tracks"), LIBRARY_LIST_TRACKS, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_PLAYLISTS: playlistpage_open(); break;
    case OPEN_BROWSE: switch_screen(browser_screen); break;
    case OPEN_MOST_PLAYED: medialist_open(tr("music_most_played"), LIBRARY_LIST_MOST_PLAYED, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_TOOLS: switch_screen(tools_screen); break;
    case OPEN_DOWNLOADS: show_page(DOWNLOADS); break;
    case OPEN_SETTINGS: switch_screen(settings_screen); break;
    case OPEN_CLASSIC_LIBRARY: switch_screen(music_screen); break;
    case OPEN_LOCAL_SEARCH: search_open(); break;
    case OPEN_WIFI: switch_screen(wifisettings_screen); break;
    case OPEN_STREAMING: switch_screen(streaming_screen); break;
    case OPEN_DISCOVERY: show_page(SEARCH); break;
    case OPEN_PROVIDERS: show_page(PROVIDERS); break;
    case OPEN_DOWNLOAD_FOLDER: {
        if (!gui_card_available()) { gui_notify_no_card(); break; }
        char path[768];
        snprintf(path, sizeof(path), "%s/Downloads", cfg->sd_root_path);
        if (access(path, R_OK) != 0) { gui_notify_popup("Your downloads will appear here after the first transfer."); break; }
        browser_open_dir_public(path);
        switch_screen(browser_screen);
        break;
    }
    }
}

static lv_obj_t *route(lv_obj_t *parent, const char *title, const char *subtitle,
                       const lv_image_dsc_t *icon, int action) {
    int w = cfg->screen_width - 2 * PAD;
    lv_obj_t *o = button(parent, NULL, w, 78, action_cb, (void *)(intptr_t)action);
    lv_obj_t *im = glyph(o, icon, 30);
    lv_obj_set_pos(im, 16, 24);
    lv_obj_t *t = label(o, title, &font_ui_22_bold, w - 86, false);
    lv_obj_set_height(t, 29);
    lv_obj_set_pos(t, 62, 11);
    lv_obj_t *s = label(o, subtitle, &font_ui_16, w - 86, true);
    lv_obj_set_height(s, 22);
    lv_obj_set_pos(s, 62, 43);
    return o;
}

typedef struct { char path[512], title[256]; } home_item_t;
static home_item_t home_items[6];
static int home_count;

typedef struct {
    char value[512], title[256], path[512], artist[256];
    lv_obj_t *art;
    cover_image_t image;
    bool requested, settled, loaded;
} home_album_t;
static home_album_t home_albums[COVERLOADER_HOME_COUNT];
static int home_album_count;

static void home_art_drop(void) {
    for (int i = 0; i < home_album_count; i++) {
        home_album_t *a = &home_albums[i];
        coverloader_release(COVERLOADER_HOME_BASE + i);
        if (a->loaded) {
            lv_image_set_src(a->art, &icon_album);
            lv_obj_set_size(a->art, 56, 56);
            lv_obj_set_pos(a->art, 44, 38);
            lv_obj_set_style_image_recolor_opa(a->art, LV_OPA_COVER, 0);
            cover_free(&a->image);
        }
        a->requested = a->settled = a->loaded = false;
    }
}

static void home_art_update(void) {
    for (int i = 0; i < home_album_count; i++) {
        home_album_t *a = &home_albums[i];
        if (a->settled || !a->path[0]) continue;
        if (!a->requested) {
            coverloader_request(COVERLOADER_HOME_BASE + i, a->path, 112);
            a->requested = true;
        }
        bool finished = false;
        if (coverloader_take(COVERLOADER_HOME_BASE + i, &a->image, &finished)) {
            lv_image_set_src(a->art, &a->image.dsc);
            lv_obj_set_size(a->art, 112, 112);
            lv_obj_set_pos(a->art, 16, 10);
            lv_obj_set_style_image_recolor_opa(a->art, LV_OPA_TRANSP, 0);
            a->loaded = true;
        }
        if (finished) a->settled = true;
    }
}

static bool home_album_collect(const char *name, const char *path, const char *artist, void *unused) {
    (void)unused;
    if (home_album_count == COVERLOADER_HOME_COUNT) return false;
    home_album_t *a = &home_albums[home_album_count++];
    memset(a, 0, sizeof(*a));
    snprintf(a->value, sizeof(a->value), "%s", name ? name : "");
    library_album_title(a->value, a->title, sizeof(a->title));
    snprintf(a->path, sizeof(a->path), "%s", path ? path : "");
    snprintf(a->artist, sizeof(a->artist), "%s", artist ? artist : "");
    return true;
}

static void home_album_cb(lv_event_t *e) {
    home_album_t *a = lv_event_get_user_data(e);
    if (tapped()) medialist_open(a->title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, a->value);
}

static void home_track_cb(lv_event_t *e) {
    home_item_t *item = lv_event_get_user_data(e);
    if (!tapped()) return;
    if (!gui_card_available()) { gui_notify_no_card(); return; }
    if (access(item->path, R_OK) != 0) { gui_notify_popup("This track is no longer on the card."); return; }
    player_play_file(item->path);
    player_sheet_open(true);
}

static void home_build(void) {
    lv_obj_t *p = page[HOME];
    home_art_drop();
    lv_obj_clean(p);
    home_count = 0;
    section(p, "Your music, within reach");
    lv_obj_t *row = box(p, cfg->screen_width - 2 * PAD, 138, false);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, 12, 0);
    lv_obj_set_scrollable(row, true);
    lv_obj_set_scroll_dir(row, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    const char *titles[] = {"Liked songs", "On repeat", "Playlists", "Albums"};
    const lv_image_dsc_t *icons[] = {&icon_star_filled, &icon_repeat_all, &icon_list_music, &icon_album};
    const int actions[] = {OPEN_FAVOURITES, OPEN_MOST_PLAYED, OPEN_PLAYLISTS, OPEN_ALBUMS};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *c = button(row, NULL, 132, 132, action_cb, (void *)(intptr_t)actions[i]);
        lv_obj_t *im = glyph(c, icons[i], 42);
        lv_obj_set_pos(im, 16, 17);
        lv_obj_set_style_image_recolor(im, i == 0 ? mint() : ink(), 0);
        lv_obj_t *t = label(c, titles[i], &font_ui_18, 110, false);
        lv_obj_set_pos(t, 12, 91);
    }
    home_album_count = 0;
    library_index_t *albums = library_index_open(LIBRARY_LIST_ALBUMS, LIBRARY_FILTER_NONE, NULL,
                                                  LIBRARY_ORDER_ADDED, true);
    if (albums) {
        library_index_window(albums, 0, COVERLOADER_HOME_COUNT, home_album_collect, NULL);
        library_index_close(albums);
    }
    if (home_album_count) {
        section(p, "Recently added albums");
        lv_obj_t *albums_row = box(p, cfg->screen_width - 2 * PAD, 196, false);
        lv_obj_set_flex_flow(albums_row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(albums_row, 12, 0);
        lv_obj_set_scrollable(albums_row, true);
        lv_obj_set_scroll_dir(albums_row, LV_DIR_HOR);
        lv_obj_set_scrollbar_mode(albums_row, LV_SCROLLBAR_MODE_OFF);
        for (int i = 0; i < home_album_count; i++) {
            home_album_t *a = &home_albums[i];
            lv_obj_t *card = button(albums_row, NULL, 144, 190, home_album_cb, a);
            a->art = glyph(card, &icon_album, 56);
            lv_obj_set_pos(a->art, 44, 38);
            lv_obj_t *title = label(card, a->title, &font_ui_18, 124, false);
            lv_obj_set_height(title, 25);
            lv_obj_set_pos(title, 10, 132);
            lv_obj_t *artist = label(card, a->artist, &font_ui_14, 124, true);
            lv_obj_set_height(artist, 22);
            lv_obj_set_pos(artist, 10, 160);
        }
    }
    section(p, "Recently played");
    library_history_item_t history[24];
    int n = library_history_recent(history, 24);
    for (int i = 0; i < n && home_count < 6; ++i) {
        home_item_t item;
        if (!library_history_entry(history[i].id, item.path, sizeof(item.path), item.title, sizeof(item.title))) continue;
        bool duplicate = false;
        for (int j = 0; j < home_count; j++) if (!strcmp(item.path, home_items[j].path)) duplicate = true;
        if (duplicate) continue;
        home_items[home_count] = item;
        lv_obj_t *b = button(p, NULL, cfg->screen_width - 2 * PAD, 64, home_track_cb, &home_items[home_count++]);
        lv_obj_t *im = glyph(b, &icon_music2, 26);
        lv_obj_set_pos(im, 16, 19);
        lv_obj_t *t = label(b, item.title, &font_ui_20, cfg->screen_width - 116, false);
        lv_obj_set_height(t, 28);
        lv_obj_set_pos(t, 58, 19);
    }
    if (!home_count) {
        lv_obj_t *empty = label(p, "Play something you love. Your listening history will appear here.", &font_ui_18,
                               cfg->screen_width - 2 * PAD, true);
        lv_label_set_long_mode(empty, LV_LABEL_LONG_WRAP);
    }
    section(p, "Find your next listen");
    route(p, "Online discovery", "Independent releases and openly licensed audio", &icon_search, OPEN_DISCOVERY);
    route(p, "Browse your card", "Music, folders and downloaded files", &icon_folder, OPEN_BROWSE);
    home_revision = library_revision(LIBRARY_LIST_TRACKS);
    home_history = library_history_revision();
}

static void library_build(void) {
    lv_obj_t *p = page[LIBRARY];
    section(p, "Saved for you");
    route(p, "Liked songs", "Your favourites, available offline", &icon_star_filled, OPEN_FAVOURITES);
    route(p, tr("playlists"), "Create, arrange and import collections", &icon_list_music, OPEN_PLAYLISTS);
    route(p, "Downloads", "Transfer queue and downloaded tracks", &icon_download, OPEN_DOWNLOADS);
    route(p, tr("albums"), "Browse your records", &icon_album, OPEN_ALBUMS);
    route(p, tr("artists"), "The people behind your music", &icon_artist, OPEN_ARTISTS);
    route(p, tr("music_all_tracks"), "Sort and filter your whole collection", &icon_music2, OPEN_TRACKS);
    route(p, "All library views", "Genres, years, album artists and custom tiles", &icon_folder, OPEN_CLASSIC_LIBRARY);
}

/* The worker writes separate storage. Displayed buttons keep stable data
 * until it finishes, then the GUI thread alone rebuilds their bounded list. */
typedef struct { library_search_type_t type; char name[256], path[512]; } local_hit_t;
typedef struct {
    discovery_results_t online;
    local_hit_t local[LOCAL_HITS];
    int local_count;
    char error[192];
} search_result_t;
static search_result_t pending, shown;
static pthread_mutex_t search_lock = PTHREAD_MUTEX_INITIALIZER;
static bool search_busy, search_ready, request_album;
static int request_filter;
static char request_text[256];
static lv_obj_t *search_field, *search_results, *search_status, *filter_buttons[3], *search_keyboard_parent;
static keyboard_t *search_keyboard;
static int search_filter;

static void local_collect(library_search_type_t type, const char *name, const char *path, void *user) {
    search_result_t *r = user;
    if (r->local_count >= LOCAL_HITS) return;
    local_hit_t *hit = &r->local[r->local_count++];
    hit->type = type;
    snprintf(hit->name, sizeof(hit->name), "%s", name ? name : "");
    snprintf(hit->path, sizeof(hit->path), "%s", path ? path : "");
}

static void *search_worker(void *unused) {
    (void)unused;
    memset(&pending, 0, sizeof(pending));
    if (!request_album && request_filter != FILTER_ONLINE)
        library_search(request_text, LOCAL_HITS / 3, local_collect, &pending);
    if (request_album)
        discovery_album(request_text, &pending.online, pending.error, sizeof(pending.error));
    else if (request_filter != FILTER_LOCAL)
        discovery_search(request_text, &pending.online, pending.error, sizeof(pending.error));
    pthread_mutex_lock(&search_lock);
    search_ready = true;
    pthread_mutex_unlock(&search_lock);
    return NULL;
}

static void search_keyboard_visible(bool visible) {
    keyboard_set_visible(search_keyboard, visible);
    lv_obj_set_hidden(search_keyboard_parent, !visible);
    lv_obj_set_hidden(mini, visible);
    int height = visible ? cfg->screen_height - NAV_H - 310 -
                           (cfg->top_bar_height + HEADER_H + 150) - 8 : body_height() - 150;
    lv_obj_set_height(search_results, height > 40 ? height : 40);
}

static void start_search(const char *query, bool album) {
    if (search_busy) { gui_notify_popup("A search is already running."); return; }
    if (!query || !query[0]) { gui_notify_popup("Enter an artist, album or track."); return; }
    snprintf(request_text, sizeof(request_text), "%s", query);
    request_album = album;
    request_filter = search_filter;
    if (!config_get_bool("discovery", "enabled", true)) {
        if (album || request_filter == FILTER_ONLINE) {
            gui_notify_popup("Online discovery is disabled. Enable it in Sources.");
            return;
        }
        request_filter = FILTER_LOCAL;
    }
    search_keyboard_visible(false);
    lv_label_set_text(search_status, album ? "Loading release..." : "Searching...");
    search_ready = false;
    search_busy = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, 256 * 1024);
    pthread_t thread;
    int rc = pthread_create(&thread, &attr, search_worker, NULL);
    pthread_attr_destroy(&attr);
    if (rc != 0) { search_busy = false; lv_label_set_text(search_status, "Could not start search. Try again."); }
}

static void submit_cb(lv_event_t *e) { (void)e; start_search(lv_textarea_get_text(search_field), false); }
static void field_cb(lv_event_t *e) { (void)e; if (!search_busy) search_keyboard_visible(true); }

static void filter_cb(lv_event_t *e) {
    if (!tapped() || search_busy) return;
    search_filter = (int)(intptr_t)lv_event_get_user_data(e);
    for (int i = 0; i < 3; i++) {
        lv_obj_set_style_border_width(filter_buttons[i], i == search_filter ? 2 : 0, 0);
        lv_obj_set_style_border_color(filter_buttons[i], mint(), 0);
    }
    if (lv_textarea_get_text(search_field)[0]) submit_cb(NULL);
}

static void local_hit_cb(lv_event_t *e) {
    local_hit_t *hit = lv_event_get_user_data(e);
    if (!tapped()) return;
    if (!gui_card_available()) { gui_notify_no_card(); return; }
    if (hit->type == LIBRARY_SEARCH_TRACK) {
        player_play_file(hit->path);
        player_sheet_open(true);
    } else if (hit->type == LIBRARY_SEARCH_ALBUM) {
        char title[256];
        library_album_title(hit->name, title, sizeof(title));
        medialist_open(title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, hit->name);
    } else medialist_open_artist(hit->name);
}

static void online_hit_cb(lv_event_t *e) {
    discovery_item_t *item = lv_event_get_user_data(e);
    if (!tapped() || search_busy) return;
    if (item->kind == DISCOVERY_ALBUM) { start_search(item->id, true); return; }
    if (!gui_card_available()) { gui_notify_no_card(); return; }
    char error[192] = "Could not open the download queue.";
    unsigned id = 0;
    if (!download_queue_init(cfg->sd_root_path) || !(id = download_enqueue(item, error, sizeof(error)))) {
        gui_notify_popup(error);
        return;
    }
    /* A repeated explicit download tap may select a restored paused job.
     * Resume that job instead of saying it was added while leaving it idle. */
    download_resume(id);
    show_page(DOWNLOADS);
}

static void search_draw(void) {
    lv_obj_clean(search_results);
    shown = pending;
    /* A preference reorders original source files; it never transcodes or
     * hides alternative formats. Stable insertion keeps their source order. */
    int preference = (int)config_get_int("discovery", "format", 0);
    const char *preferred = preference == 1 ? "mp3" : preference == 2 ? "flac" : "";
    int front = 0;
    if (*preferred) for (int i = 0; i < shown.online.count; i++) {
        if (shown.online.items[i].kind == DISCOVERY_TRACK && !strcmp(shown.online.items[i].format, preferred)) {
            discovery_item_t item = shown.online.items[i];
            memmove(&shown.online.items[front + 1], &shown.online.items[front],
                    (size_t)(i - front) * sizeof(item));
            shown.online.items[front++] = item;
        }
    }
    const char *groups[] = {"Local tracks", "Local albums", "Local artists"};
    for (int kind = 0; kind < 3; kind++) {
        bool heading_drawn = false;
        for (int i = 0; i < shown.local_count; i++) {
            local_hit_t *hit = &shown.local[i];
            if ((int)hit->type != kind) continue;
            if (!heading_drawn) { section(search_results, groups[kind]); heading_drawn = true; }
            char name[256];
            if (kind == LIBRARY_SEARCH_ALBUM) library_album_title(hit->name, name, sizeof(name));
            else snprintf(name, sizeof(name), "%s", hit->name);
            button(search_results, name, cfg->screen_width - 2 * PAD, 62, local_hit_cb, hit);
        }
    }
    if (shown.online.count) section(search_results, request_album ? "Available audio files" : "Online releases");
    for (int i = 0; i < shown.online.count; i++) {
        discovery_item_t *item = &shown.online.items[i];
        lv_obj_t *b = button(search_results, NULL, cfg->screen_width - 2 * PAD, 90, online_hit_cb, item);
        lv_obj_t *title = label(b, item->title, &font_ui_20, cfg->screen_width - 76, false);
        lv_obj_set_height(title, 28);
        lv_obj_set_pos(title, 14, 11);
        char subtitle[220];
        if (item->kind == DISCOVERY_ALBUM) snprintf(subtitle, sizeof(subtitle), "%s  /  Open release", item->artist);
        else snprintf(subtitle, sizeof(subtitle), "%s  /  %.1f MB  /  Download", item->format, (double)item->bytes / 1048576);
        lv_obj_t *sub = label(b, subtitle, &font_ui_16, cfg->screen_width - 76, true);
        lv_obj_set_height(sub, 24);
        lv_obj_set_pos(sub, 14, 48);
    }
    if (shown.error[0]) {
        lv_obj_t *err = label(search_results, shown.error, &font_ui_18, cfg->screen_width - 2 * PAD, true);
        lv_label_set_long_mode(err, LV_LABEL_LONG_WRAP);
    }
    char status[128];
    snprintf(status, sizeof(status), "%d local / %d online%s", shown.local_count, shown.online.count,
             shown.online.truncated ? "  /  Refine for more" : "");
    lv_label_set_text(search_status, status);
    if (!shown.local_count && !shown.online.count && !shown.error[0])
        label(search_results, "No matches. Try an artist or album name.", &font_ui_18, cfg->screen_width - 2 * PAD, true);
    if (shown.local_count >= LOCAL_HITS / 3)
        route(search_results, "Search the full local library", "This overview shows up to six hits per category", &icon_search, OPEN_LOCAL_SEARCH);
}

static void search_build(void) {
    lv_obj_t *p = page[SEARCH];
    lv_obj_set_layout(p, LV_LAYOUT_NONE);
    lv_obj_set_scrollable(p, false);
    search_field = lv_textarea_create(p);
    lv_obj_set_size(search_field, cfg->screen_width - 2 * PAD, 58);
    lv_obj_set_pos(search_field, 0, 0);
    lv_textarea_set_one_line(search_field, true);
    lv_textarea_set_max_length(search_field, 160);
    lv_textarea_set_placeholder_text(search_field, "Artists, albums or songs");
    lv_obj_add_style(search_field, &theme_style_card, 0);
    lv_obj_add_style(search_field, &theme_style_text, 0);
    lv_obj_set_style_text_font(search_field, &font_ui_20, 0);
    keyboard_style_caret(search_field);
    lv_obj_add_event_cb(search_field, field_cb, LV_EVENT_CLICKED, NULL);
    const char *names[] = {"All", "Local", "Online"};
    for (int i = 0; i < 3; i++) {
        filter_buttons[i] = button(p, names[i], 110, 46, filter_cb, (void *)(intptr_t)i);
        lv_obj_set_pos(filter_buttons[i], i * 120, 68);
    }
    lv_obj_set_style_border_width(filter_buttons[0], 2, 0);
    lv_obj_set_style_border_color(filter_buttons[0], mint(), 0);
    lv_obj_t *sources = button(p, "Sources", 80, 46, action_cb, (void *)OPEN_PROVIDERS);
    lv_obj_set_style_text_font(lv_obj_get_child(sources, 0), &font_ui_14, 0);
    lv_obj_set_height(lv_obj_get_child(sources, 0), 20);
    lv_obj_set_pos(sources, 360, 68);
    search_status = label(p, "Local music + Internet Archive", &font_ui_16, cfg->screen_width - 2 * PAD, true);
    lv_obj_set_pos(search_status, 0, 122);
    search_results = column(p, cfg->screen_width - 2 * PAD, body_height() - 150);
    lv_obj_set_pos(search_results, 0, 150);
    section(search_results, "Browse everything");
    route(search_results, "Your full local library", "Tracks, albums and artists", &icon_music2, OPEN_LOCAL_SEARCH);
    route(search_results, "Streaming & podcasts", "Tidal, Qobuz, radio and podcast search", &icon_wifi, OPEN_STREAMING);
    route(search_results, "Wi-Fi settings", "Connect to explore online", &icon_wifi, OPEN_WIFI);
    lv_obj_t *note = label(search_results,
        "Online search finds openly licensed releases. Open one to choose its source format and download. Streaming previews are not supported by this provider.",
        &font_ui_18, cfg->screen_width - 2 * PAD, true);
    lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
    search_keyboard_parent = box(main_menu_screen, cfg->screen_width, 310, false);
    lv_obj_set_pos(search_keyboard_parent, 0, cfg->screen_height - NAV_H - 310);
    search_keyboard = keyboard_create(search_keyboard_parent, cfg->screen_width, 310, search_field,
                                      &icon_search, NULL, submit_cb, NULL);
    search_keyboard_visible(false);
}

static lv_obj_t *provider_enabled_text, *provider_format_text;

static void provider_refresh(void) {
    if (!provider_enabled_text) return;
    lv_label_set_text(provider_enabled_text, config_get_bool("discovery", "enabled", true)
        ? "Online discovery: On" : "Online discovery: Off");
    int format = (int)config_get_int("discovery", "format", 0);
    lv_label_set_text(provider_format_text, format == 1 ? "Prefer MP3 sources" :
        format == 2 ? "Prefer FLAC sources" : "Use source format order");
}

static void provider_setting_cb(lv_event_t *e) {
    if (!tapped()) return;
    int action = (int)(intptr_t)lv_event_get_user_data(e);
    if (action == 0) config_set_bool("discovery", "enabled", !config_get_bool("discovery", "enabled", true));
    else config_set_int("discovery", "format", (config_get_int("discovery", "format", 0) + 1) % 3);
    config_save();
    provider_refresh();
}

static void pause_all_cb(lv_event_t *e) {
    (void)e;
    if (!tapped()) return;
    download_job_t snapshot[DOWNLOAD_MAX_JOBS];
    int n = download_snapshot(snapshot, DOWNLOAD_MAX_JOBS);
    for (int i = 0; i < n; i++) if (snapshot[i].state == DOWNLOAD_RUNNING || snapshot[i].state == DOWNLOAD_QUEUED)
        download_pause(snapshot[i].id);
    gui_notify_popup("Transfers paused. Resume them individually in Downloads.");
}

static void providers_build(void) {
    lv_obj_t *p = page[PROVIDERS];
    section(p, "Internet Archive");
    lv_obj_t *note = label(p, "Release search, album files and authorized downloads. No sign-in required.",
                          &font_ui_18, cfg->screen_width - 2 * PAD, true);
    lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
    lv_obj_t *enabled = button(p, "", cfg->screen_width - 2 * PAD, 58, provider_setting_cb, (void *)0);
    provider_enabled_text = lv_obj_get_child(enabled, 0);
    lv_obj_t *format = button(p, "", cfg->screen_width - 2 * PAD, 58, provider_setting_cb, (void *)1);
    provider_format_text = lv_obj_get_child(format, 0);
    provider_refresh();
    button(p, "Pause all transfers", cfg->screen_width - 2 * PAD, 58, pause_all_cb, NULL);
    note = label(p, "Tap to change a setting. Preferred formats appear first; all available formats remain selectable. "
        "Turning discovery off stops new searches; running transfers can be paused above. Restored transfers always wait for you to resume.",
        &font_ui_18, cfg->screen_width - 2 * PAD, true);
    lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
    section(p, "Provider capabilities");
    note = label(p, "Supported: release search, release files, source metadata, licensed audio downloads. "
        "Not supported: streaming previews, artist pages, online playlists or automatic provider fallback. "
        "Downloads keep the original audio quality and embedded tags, with an attribution sidecar for missing metadata.",
        &font_ui_18, cfg->screen_width - 2 * PAD, true);
    lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
    route(p, "Wi-Fi settings", "Choose a network", &icon_wifi, OPEN_WIFI);
}

static download_job_t jobs[DOWNLOAD_MAX_JOBS];
static lv_obj_t *job_title[DOWNLOAD_MAX_JOBS], *job_status[DOWNLOAD_MAX_JOBS], *job_card[DOWNLOAD_MAX_JOBS];
static int job_count;

static void job_cb(lv_event_t *e) {
    intptr_t index = (intptr_t)lv_event_get_user_data(e);
    if (!tapped() || index < 0 || index >= job_count) return;
    download_job_t *job = &jobs[index];
    if (job->state == DOWNLOAD_DONE) {
        if (!gui_card_available()) { gui_notify_no_card(); return; }
        player_play_file(job->path);
        player_sheet_open(true);
    } else if (job->state == DOWNLOAD_PAUSED || job->state == DOWNLOAD_FAILED) download_resume(job->id);
    else if (job->state == DOWNLOAD_QUEUED || job->state == DOWNLOAD_RUNNING) download_pause(job->id);
}

static void cancel_cb(lv_event_t *e) {
    intptr_t index = (intptr_t)lv_event_get_user_data(e);
    if (tapped() && index >= 0 && index < job_count) download_cancel(jobs[index].id);
}

static void downloads_build(void) {
    lv_obj_t *p = page[DOWNLOADS];
    section(p, "Your download queue");
    lv_obj_t *hint = label(p, "Tap a transfer to pause or resume. Tap a completed track to play.", &font_ui_18,
                           cfg->screen_width - 2 * PAD, true);
    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
    for (int i = 0; i < DOWNLOAD_MAX_JOBS; i++) {
        job_card[i] = button(p, NULL, cfg->screen_width - 2 * PAD, 112, job_cb, (void *)(intptr_t)i);
        job_title[i] = label(job_card[i], "", &font_ui_20, cfg->screen_width - 140, false);
        lv_obj_set_height(job_title[i], 28);
        lv_obj_set_pos(job_title[i], 12, 10);
        job_status[i] = label(job_card[i], "", &font_ui_16, cfg->screen_width - 140, true);
        lv_obj_set_pos(job_status[i], 12, 42);
        lv_label_set_long_mode(job_status[i], LV_LABEL_LONG_WRAP);
        lv_obj_t *cancel = button(job_card[i], "X", 52, 52, cancel_cb, (void *)(intptr_t)i);
        lv_obj_set_event_bubble(cancel, false);
        lv_obj_align(cancel, LV_ALIGN_RIGHT_MID, -8, 0);
        lv_obj_set_hidden(job_card[i], true);
    }
    route(p, "Browse downloaded files", "Open the Downloads folder on your card", &icon_folder, OPEN_DOWNLOAD_FOLDER);
}

static void downloads_update(void) {
    job_count = download_snapshot(jobs, DOWNLOAD_MAX_JOBS);
    for (int i = 0; i < DOWNLOAD_MAX_JOBS; i++) {
        lv_obj_set_hidden(job_card[i], i >= job_count);
        if (i >= job_count) continue;
        download_job_t *job = &jobs[i];
        lv_label_set_text(job_title[i], job->item.title);
        char status[256];
        snprintf(status, sizeof(status), "%s  /  %.1f / %.1f MB\n%s", download_state_name(job->state),
                 (double)job->done / 1048576, (double)job->total / 1048576, job->error);
        if (job->state == DOWNLOAD_RUNNING)
            snprintf(status, sizeof(status), "%.1f / %.1f MB  /  %u KB/s\nTap to pause",
                     (double)job->done / 1048576, (double)job->total / 1048576, job->bytes_per_second / 1024);
        lv_label_set_text(job_status[i], status);
    }
}

static void mini_open_cb(lv_event_t *e) { (void)e; if (tapped()) player_sheet_open(true); }
static void mini_play_cb(lv_event_t *e) { (void)e; if (tapped()) player_key_play_pause(); }

static void mini_drop(void) {
    if (mini_has_art) {
        lv_image_set_src(mini_art, &icon_music2);
        lv_obj_set_style_image_recolor_opa(mini_art, LV_OPA_COVER, 0);
        cover_free(&mini_image);
        mini_has_art = false;
    }
}

static void mini_update(void) {
    device_state_t state;
    device_state_get(&state);
    const char *fallback = strrchr(state.current_file, '/');
    const char *title = state.metadata.title[0] ? state.metadata.title :
                        (state.current_file[0] ? (fallback ? fallback + 1 : state.current_file) : "Choose something to play");
    if (strcmp(lv_label_get_text(mini_title), title)) lv_label_set_text(mini_title, title);
    const char *artist = metadata_shown_artist(&state.metadata);
    if (strcmp(lv_label_get_text(mini_artist), artist)) lv_label_set_text(mini_artist, artist);
    const lv_image_dsc_t *play_icon = state.status == AUDIO_STATUS_PLAYING ? &icon_pause : &icon_play;
    if (state.live && state.status == AUDIO_STATUS_PLAYING) play_icon = &icon_stop;
    if (lv_image_get_src(mini_play) != play_icon) lv_image_set_src(mini_play, play_icon);
    int progress = state.progress_total_secs > 0 ? (int)(state.progress_current_secs * 1000 / state.progress_total_secs) : 0;
    lv_bar_set_value(mini_progress, progress, LV_ANIM_OFF);
    if (strcmp(mini_path, state.current_file)) {
        mini_drop();
        snprintf(mini_path, sizeof(mini_path), "%s", state.current_file);
    }
    if (!mini_has_art && state.current_file[0]) {
        uint64_t id;
        if (player_cover_copy(52, &mini_image, &id)) {
            lv_image_set_src(mini_art, &mini_image.dsc);
            lv_obj_set_style_image_recolor_opa(mini_art, LV_OPA_TRANSP, 0);
            mini_has_art = true;
        }
    }
}

static void poll_cb(lv_timer_t *timer) {
    (void)timer;
    if (search_busy) {
        pthread_mutex_lock(&search_lock);
        bool ready = search_ready;
        pthread_mutex_unlock(&search_lock);
        if (ready) { search_draw(); search_busy = false; }
    }
    if (lv_screen_active() != main_menu_screen || player_sheet_is_open()) return;
    mini_update();
    if (selected == HOME) home_art_update();
    if (selected == DOWNLOADS) downloads_update();
}

static void nav_cb(lv_event_t *e) { if (tapped()) show_page((int)(intptr_t)lv_event_get_user_data(e)); }

static void show_page(int tab) {
    if (tab < HOME || tab > PROVIDERS) return;
    selected = tab;
    if (tab != HOME) home_art_drop();
    if (search_keyboard) search_keyboard_visible(false);
    if (!page[tab]) {
        page[tab] = column(main_menu_screen, cfg->screen_width - 2 * PAD, body_height());
        lv_obj_set_pos(page[tab], PAD, cfg->top_bar_height + HEADER_H);
        if (tab == HOME) home_build();
        if (tab == SEARCH) search_build();
        if (tab == LIBRARY) library_build();
        if (tab == DOWNLOADS) downloads_build();
        if (tab == PROVIDERS) providers_build();
    }
    for (int i = 0; i < 5; i++) if (page[i]) lv_obj_set_hidden(page[i], i != tab);
    const char *titles[] = {"Sonix", "Search", "Your Library", "Downloads", "Discovery"};
    lv_label_set_text(heading, titles[tab]);
    for (int i = 0; i < 3; i++) {
        bool active = i == (tab == DOWNLOADS ? LIBRARY : tab == PROVIDERS ? SEARCH : tab);
        lv_obj_set_style_image_recolor(nav_icon[i], active ? mint() : quiet(), 0);
        lv_obj_set_style_text_color(nav_label[i], active ? ink() : quiet(), 0);
    }
    if (tab == HOME && (home_revision != library_revision(LIBRARY_LIST_TRACKS) || home_history != library_history_revision())) home_build();
    if (tab == DOWNLOADS) downloads_update();
}

static void screen_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_SCREEN_LOADED) {
        show_page(selected);
        mini_update();
    } else {
        if (search_keyboard) search_keyboard_visible(false);
        mini_drop();
        home_art_drop();
    }
}

void main_menu_init(gui_config_t *config) {
    cfg = config;
    /* All six legacy destinations remain reachable without remapping their
     * callbacks or the hardware transport keys. */
    tools_screen = lv_obj_create(NULL);
    const grid_entry_t entries[] = {
        {"music", &icon_menu_music, &music_screen},
        {"streaming", &icon_menu_streaming, &streaming_screen},
        {"wireless", &icon_menu_wireless, &wireless_screen},
        {"audiobooks", &icon_menu_audiobooks, &audiobooks_screen},
        {"more", &icon_menu_more, &morepage_screen},
        {"settings", &icon_menu_settings, &settings_screen},
    };
    gridpage_build(tools_screen, cfg, entries, 6, 2, 3, true);
    switcher_attach_back_gesture(tools_screen);
    lv_obj_add_style(main_menu_screen, &theme_style_screen, 0);
    lv_obj_set_style_pad_all(main_menu_screen, 0, 0);
    lv_obj_set_scrollable(main_menu_screen, false);
    heading = label(main_menu_screen, "Sonix", &font_ui_32, cfg->screen_width - 190, false);
    lv_obj_set_pos(heading, PAD, cfg->top_bar_height + 17);
    lv_obj_t *tools = button(main_menu_screen, "Tools", 82, 48, action_cb, (void *)OPEN_TOOLS);
    lv_obj_set_pos(tools, cfg->screen_width - 174, cfg->top_bar_height + 10);
    lv_obj_t *settings = button(main_menu_screen, NULL, 60, 48, action_cb, (void *)OPEN_SETTINGS);
    lv_obj_set_pos(settings, cfg->screen_width - 80, cfg->top_bar_height + 10);
    lv_obj_t *settings_icon = glyph(settings, &icon_music_settings, 26);
    lv_obj_center(settings_icon);
    mini = button(main_menu_screen, NULL, cfg->screen_width - 24, MINI_H, mini_open_cb, NULL);
    lv_obj_set_pos(mini, 12, cfg->screen_height - NAV_H - MINI_H - 6);
    mini_art = glyph(mini, &icon_music2, 52);
    lv_obj_set_pos(mini_art, 10, 10);
    mini_title = label(mini, "Choose something to play", &font_ui_18, cfg->screen_width - 166, false);
    lv_obj_set_height(mini_title, 25);
    lv_obj_set_pos(mini_title, 74, 12);
    mini_artist = label(mini, "", &font_ui_16, cfg->screen_width - 166, true);
    lv_obj_set_height(mini_artist, 23);
    lv_obj_set_pos(mini_artist, 74, 39);
    lv_obj_t *transport = button(mini, NULL, 54, 58, mini_play_cb, NULL);
    lv_obj_set_event_bubble(transport, false);
    lv_obj_align(transport, LV_ALIGN_RIGHT_MID, -5, -2);
    mini_play = glyph(transport, &icon_play, 28);
    lv_obj_center(mini_play);
    mini_progress = lv_bar_create(mini);
    lv_obj_set_size(mini_progress, cfg->screen_width - 44, 3);
    lv_obj_align(mini_progress, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_bar_set_range(mini_progress, 0, 1000);
    lv_obj_set_style_bg_color(mini_progress, mint(), LV_PART_INDICATOR);
    lv_obj_set_style_shadow_width(mini_progress, 0, 0);
    const char *names[] = {"Home", "Search", "Your Library"};
    const lv_image_dsc_t *icons[] = {&icon_now_playing, &icon_search, &icon_list_music};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *nav = button(main_menu_screen, NULL, cfg->screen_width / 3, NAV_H, nav_cb, (void *)(intptr_t)i);
        lv_obj_set_pos(nav, i * (cfg->screen_width / 3), cfg->screen_height - NAV_H);
        lv_obj_set_style_radius(nav, 0, 0);
        lv_obj_add_style(nav, &theme_style_panel, 0);
        nav_icon[i] = glyph(nav, icons[i], 26);
        lv_obj_align(nav_icon[i], LV_ALIGN_TOP_MID, 0, 9);
        nav_label[i] = label(nav, names[i], &font_ui_16, cfg->screen_width / 3 - 8, false);
        lv_obj_set_style_text_align(nav_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(nav_label[i], LV_ALIGN_BOTTOM_MID, 0, -10);
    }
    lv_obj_add_event_cb(main_menu_screen, screen_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(main_menu_screen, screen_cb, LV_EVENT_SCREEN_UNLOADED, NULL);
    show_page(HOME);
    lv_timer_create(poll_cb, 750, NULL);
}
