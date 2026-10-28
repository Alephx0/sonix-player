#include "src/gui/online/online_album.h"
#include "src/gui/library/homeview.h"
#include "src/gui/library/playlistview.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/system/library/playlists.h"
#include "src/gui/shell/toast.h"
#include "main_menu.h"
#include "ui_probe.h"

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
#include "src/gui/shell/reference_icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/shell/morepage.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/gui/shell/topbar.h"
#include "src/gui/streaming/streaming.h"
#include "src/gui/streaming/podcastpage.h"
#include "src/gui/wireless/wifisettings.h"
#include "src/gui/wireless/wireless.h"
#include "src/system/core/lang.h"
#include "src/system/core/config.h"
#include "src/system/discovery/discovery.h"
#include "src/system/discovery/download_queue.h"
#include "src/system/library/library.h"
#include "src/system/net/wifi.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include "src/gui/library/collectionview.h"

/* Original Sonix branding and native widgets. Legacy pages retain their own
 * layouts, history and transport owners. Primary pages and the keyboard are
 * created on demand. One bounded catalogue request runs at a time; workers
 * never touch LVGL. Download completion/indexing is owned by the application. */
lv_obj_t *main_menu_screen;
static lv_obj_t *tools_screen, *header_tools, *header_settings, *header_add, *header_search;
static gui_config_t *cfg;
static lv_obj_t *page[5], *heading, *nav_icon[3], *nav_label[3], *nav_objects[3];
static lv_obj_t *mini, *mini_title, *mini_artist, *mini_art, *mini_play, *mini_progress;
static int selected;
static cover_image_t mini_image;
static char mini_path[512];
static bool mini_has_art;

enum { HOME, SEARCH, LIBRARY, DOWNLOADS, PROVIDERS };
enum { FILTER_ALL, FILTER_LOCAL, FILTER_ONLINE };
enum { OPEN_FAVOURITES, OPEN_ALBUMS, OPEN_ARTISTS, OPEN_TRACKS, OPEN_PLAYLISTS,
       OPEN_BROWSE, OPEN_MOST_PLAYED, OPEN_TOOLS, OPEN_DOWNLOADS, OPEN_SETTINGS,
       OPEN_CLASSIC_LIBRARY, OPEN_LOCAL_SEARCH, OPEN_WIFI, OPEN_STREAMING,
       OPEN_DISCOVERY, OPEN_DOWNLOAD_FOLDER, OPEN_PROVIDERS, OPEN_GENRES, OPEN_PODCASTS };

#define PAD 20
#define NAV_H 68
#define MINI_H 64
#define HEADER_H 64
#define LOCAL_HITS 30

static lv_color_t ink(void) { return theme()->text_primary; }
static lv_color_t quiet(void) { return theme()->text_secondary; }
static lv_color_t mint(void) { return lv_color_hex(theme_is_dark() ? 0x1ed760 : 0x087954); }
static bool tapped(void) { return !player_sheet_drag_active() && !switcher_back_drag_active(); }
static int body_height(void) { return cfg->screen_height - cfg->top_bar_height - HEADER_H - NAV_H - MINI_H - 14; }
static void show_page(int tab);
static void search_browse(void);

static lv_obj_t *box(lv_obj_t *parent, int w, int h, bool surface) {
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, w, h);
    lv_obj_add_style(o, surface ? &theme_style_card : &theme_style_screen, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_radius(o, surface ? 12 : 0, 0);
    lv_obj_set_scrollable(o, false);
    if (!surface && theme_is_dark()) lv_obj_set_style_bg_color(o, lv_color_hex(0x121212), 0);
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
    case OPEN_GENRES: medialist_open("Genres", LIBRARY_LIST_GENRES, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_PODCASTS: podcastpage_open_followed(); break;
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

static unsigned home_revision;
static void home_build(void) {
    homeview_build(page[HOME],cfg);
    home_revision=library_revision(LIBRARY_LIST_TRACKS);
}
static void home_art_drop(void) {homeview_art_drop();}
static void home_art_update(void) {homeview_art_update();}

static void library_build(void) {
    lv_obj_t *p = page[LIBRARY];
    lv_obj_set_scrollable(p, false);
    lv_obj_t *collections=box(p,cfg->screen_width-2*PAD,body_height(),false);
    collectionview_build(collections,cfg);
}

/* The worker writes separate storage. Displayed buttons keep stable data
 * until it finishes, then the GUI thread alone rebuilds their bounded list. */
typedef library_instant_hit_t local_hit_t;
typedef struct {
    discovery_results_t online;
    local_hit_t local[LOCAL_HITS];
    int local_count;
    char artwork[LOCAL_HITS][512], artist[LOCAL_HITS][256];
    char error[192];
} search_result_t;
static search_result_t pending, shown;
static pthread_mutex_t search_lock = PTHREAD_MUTEX_INITIALIZER;
static bool search_busy, search_ready, request_album;
static bool local_busy, local_ready, local_wanted, online_wanted;
static unsigned local_epoch, online_epoch;
static char local_text[256], online_text[256];
static bool online_album_request, online_artist_request;
static unsigned online_page;
static discovery_results_t online_pending;
static char online_error[192];
static void search_draw(void);
static bool request_artist, browse_artist;
static unsigned request_page = 1, browse_page = 1;
static char browse_query[256], release_artist[128];
static int request_filter;
static char request_text[256];
static lv_obj_t *search_field, *search_results, *search_status, *filter_buttons[3], *search_keyboard_parent;
static keyboard_t *search_keyboard;
static int search_filter;
static bool request_instant, instant_pending;
static uint32_t instant_changed;
static unsigned search_epoch;

static void local_collect(const library_instant_hit_t *item, void *user) {
    search_result_t *r = user;
    if (r->local_count >= LOCAL_HITS) return;
    local_hit_t *hit = &r->local[r->local_count++];
    *hit = *item;
}

static bool search_sample(const char *name,const char *path,const char *artist,void *u) {
    (void)name;int i=*(int *)u;
    snprintf(pending.artwork[i],512,"%s",path?path:"");snprintf(pending.artist[i],256,"%s",artist?artist:"");return false;
}
static void search_resolve_art(void) {
    for(int i=0;i<pending.local_count;i++) {
        local_hit_t *hit=&pending.local[i];
        if(hit->type==LIBRARY_SEARCH_TRACK || hit->type==LIBRARY_SEARCH_LYRICS) {
            memcpy(pending.artwork[i],hit->path,sizeof(hit->path));
            char title[256];playlists_track_names(hit->path,title,sizeof(title),pending.artist[i],256);
        } else {
            library_list_t kind=hit->type==LIBRARY_SEARCH_PLAYLIST?LIBRARY_LIST_PLAYLIST:LIBRARY_LIST_TRACKS;
            library_filter_t filter=hit->type==LIBRARY_SEARCH_ARTIST?LIBRARY_FILTER_ARTIST:hit->type==LIBRARY_SEARCH_ALBUM?LIBRARY_FILTER_ALBUM:LIBRARY_FILTER_NONE;
            library_index_t *ix=library_index_open(kind,filter,hit->name,LIBRARY_ORDER_DEFAULT,false);
            if(ix) { library_index_window(ix,0,1,search_sample,&i);library_index_close(ix); }
        }
    }
}
static lv_obj_t *instant_images[LOCAL_HITS];
static cover_image_t instant_covers[LOCAL_HITS];
static void search_art_drop(void) {
    for(int i=0;i<LOCAL_HITS;i++) {
        coverloader_release(COVERLOADER_INSTANT_BASE+i);
        if(instant_images[i]) lv_image_set_src(instant_images[i],&icon_album);
        cover_free(&instant_covers[i]);instant_images[i]=NULL;
    }
}
static void search_art_update(void) {
    for(int i=0;i<LOCAL_HITS;i++) if(instant_images[i] && !instant_covers[i].pixels) {
        bool done=false;
        if(coverloader_take(COVERLOADER_INSTANT_BASE+i,&instant_covers[i],&done)) lv_image_set_src(instant_images[i],&instant_covers[i].dsc);
    }
}
static void *search_worker(void *unused) {
    (void)unused;
    memset(&pending,0,sizeof(pending));
    library_search_instant(local_text,LOCAL_HITS/5,local_collect,&pending);
    search_resolve_art();
    pthread_mutex_lock(&search_lock);local_ready=true;pthread_mutex_unlock(&search_lock);
    return NULL;
}
static void *online_worker(void *unused) {
    (void)unused;
#ifdef HOST_BUILD
    const char *delay=getenv("SONIX_TEST_ONLINE_DELAY_MS");
    if(delay){int ms=atoi(delay);if(ms>0 && ms<=5000)usleep(ms*1000);}
#endif
    memset(&online_pending,0,sizeof(online_pending));online_error[0]=0;
    const discovery_provider_t *provider=discovery_provider();
    if(online_album_request)provider->album(online_text,online_page,&online_pending,online_error,sizeof(online_error));
    else provider->search(online_text,online_artist_request,online_page,&online_pending,online_error,sizeof(online_error));
    pthread_mutex_lock(&search_lock);search_ready=true;pthread_mutex_unlock(&search_lock);
    return NULL;
}
static bool start_worker(void *(*fn)(void *)) {
    pthread_attr_t attr;pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr,256*1024);
    pthread_t thread;int rc=pthread_create(&thread,&attr,fn,NULL);pthread_attr_destroy(&attr);
    return rc==0;
}
static void dispatch_search(void) {
    if(local_wanted && !local_busy) {
        local_wanted=false;local_epoch=search_epoch;snprintf(local_text,sizeof(local_text),"%s",request_text);
        local_ready=false;local_busy=true;
        if(!start_worker(search_worker)){local_busy=false;lv_label_set_text(search_status,"Could not start local search.");}
    }
    if(online_wanted && !search_busy) {
        online_wanted=false;online_epoch=search_epoch;snprintf(online_text,sizeof(online_text),"%s",request_text);
        online_album_request=request_album;online_artist_request=request_artist;online_page=request_page;
        search_ready=false;search_busy=true;
        if(!start_worker(online_worker)){search_busy=false;snprintf(shown.error,sizeof(shown.error),"Could not start online search.");search_draw();}
    }
}

static void search_keyboard_visible(bool visible) {
    keyboard_set_visible(search_keyboard, visible);
    lv_obj_set_hidden(search_keyboard_parent, !visible);
    lv_obj_set_hidden(mini, visible);
    int height = visible ? cfg->screen_height - NAV_H - 310 -
                           (cfg->top_bar_height + HEADER_H + 150) - 8 : body_height() - 150;
    lv_obj_set_height(search_results, height > 40 ? height : 40);
}

static void start_search_page(const char *query, bool album, bool artist, unsigned page_number) {
    if (!query || !query[0]) { gui_notify_popup("Enter an artist, album or track."); return; }
    char linked_id[128];
    if (!album && !artist && search_filter != FILTER_LOCAL &&
        (strstr(query, "://") || !strncmp(query,"spotify:",8))) {
        if (!discovery_release_link(query,linked_id,sizeof(linked_id))) {
            gui_notify_popup("Supported release links: https://archive.org/details/... ");
            return;
        }
        query=linked_id;
        album=true;
        release_artist[0]=0;
    }
    int filter = (album || artist || page_number > 1) ? FILTER_ONLINE : search_filter;
    if (!config_get_bool("discovery", "enabled", true)) {
        if (album || filter == FILTER_ONLINE) {
            gui_notify_popup("Online discovery is disabled. Enable it in Sources.");
            return;
        }
        filter = FILTER_LOCAL;
    }
    /* Pagination may pass request_text itself. Copy before updating state. */
    char text[sizeof(request_text)];
    snprintf(text, sizeof(text), "%s", query);
    snprintf(request_text, sizeof(request_text), "%s", text);
    request_album = album;
    request_artist = artist;
    request_page = page_number;
    request_filter = filter;
    ++search_epoch;
    instant_pending = false;
    if (!album) {
        snprintf(browse_query, sizeof(browse_query), "%s", text);
        browse_artist = artist;
        browse_page = page_number;
    }
    if(!request_instant)search_keyboard_visible(false);
    if(!album)memset(&shown,0,sizeof(shown));
    local_wanted=!album && !artist && page_number==1 && filter!=FILTER_ONLINE;
    online_wanted=album || filter!=FILTER_LOCAL;
#ifndef HOST_BUILD
    if(online_wanted) {wifi_status_t wifi;wifi_get_status(&wifi);if(wifi.state!=WIFI_STATE_CONNECTED) {
        online_wanted=false;snprintf(shown.error,sizeof(shown.error),"Connect Wi-Fi to search online. Local music is available.");
    }}
#endif
    if(album)lv_label_set_text(search_status,"Loading release...");else search_draw();
    dispatch_search();
}

static void start_search(const char *query, bool album) { request_instant=false;start_search_page(query, album, false, 1); }

static void instant_changed_cb(lv_event_t *e) {
    (void)e;
    search_epoch++;
    instant_changed=lv_tick_get();
    instant_pending=true;
    local_wanted=online_wanted=false;
}

static void start_instant(void) {
    const char *query=lv_textarea_get_text(search_field);
    instant_pending=false;
    if(!query[0]) {
        local_wanted=online_wanted=false;memset(&shown,0,sizeof(shown));
        search_browse();
        lv_label_set_text(search_status,"");
        return;
    }
    request_instant=true;
    start_search_page(query,false,false,1);
}

static void page_cb(lv_event_t *e) {
    if (!tapped()) return;
    int direction = (int)(intptr_t)lv_event_get_user_data(e);
    if ((direction < 0 && request_page <= 1) || (direction > 0 && request_page >= DISCOVERY_MAX_PAGES)) return;
    start_search_page(request_text, request_album, request_artist, request_page + direction);
}

static void results_cb(lv_event_t *e) {
    (void)e;
    if (tapped() && browse_query[0]) start_search_page(browse_query, false, browse_artist, browse_page);
}

static void artist_cb(lv_event_t *e) {
    (void)e;
    if (tapped() && release_artist[0]) {
        switch_screen(main_menu_screen);show_page(SEARCH);request_instant=false;
        start_search_page(release_artist,false,true,1);
    }
}

static void submit_cb(lv_event_t *e) { (void)e; start_search(lv_textarea_get_text(search_field), false); }
static void field_cb(lv_event_t *e) { (void)e; search_keyboard_visible(true); }

static void filter_cb(lv_event_t *e) {
    if (!tapped()) return;
    search_filter = (int)(intptr_t)lv_event_get_user_data(e);
    for (int i = 0; i < 3; i++) {
        lv_obj_set_style_border_width(filter_buttons[i], i == search_filter ? 2 : 1, 0);
        lv_obj_set_style_border_color(filter_buttons[i],i == search_filter ? mint() : lv_color_hex(0x555555), 0);
    }
    if (lv_textarea_get_text(search_field)[0]) submit_cb(NULL);
}

static lv_point_t local_swipe_origin;
static lv_obj_t *local_swipe_row;
static bool local_swipe_taken;
static void local_hit_cb(lv_event_t *e) {
    if (local_swipe_taken && lv_event_get_current_target(e) == local_swipe_row) return;
    local_hit_t *hit = lv_event_get_user_data(e);
    if (!tapped()) return;
    if (!gui_card_available()) { gui_notify_no_card(); return; }
    if (hit->type == LIBRARY_SEARCH_TRACK || hit->type == LIBRARY_SEARCH_LYRICS) {
        player_play_file(hit->path);
        player_sheet_open(true);
    } else if (hit->type == LIBRARY_SEARCH_ALBUM) {
        char title[256];
        library_album_title(hit->name, title, sizeof(title));
        medialist_open(title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, hit->name);
    } else if(hit->type==LIBRARY_SEARCH_PLAYLIST) playlistview_open(hit->name,false);
    else medialist_open_artist(hit->name);
}

static void local_queue_cb(lv_event_t *e) {
    local_hit_t *hit = lv_event_get_user_data(e);
    if (!hit->path[0] || (hit->type != LIBRARY_SEARCH_TRACK && hit->type != LIBRARY_SEARCH_LYRICS)) return;
    lv_indev_t *input = lv_indev_active();
    if (!input) return;
    lv_obj_t *row = lv_event_get_current_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t point; lv_indev_get_point(input, &point);
    if (code == LV_EVENT_PRESSED) {
        local_swipe_row = row; local_swipe_origin = point; local_swipe_taken = false;
        return;
    }
    if (local_swipe_row != row || local_swipe_origin.x <= 32) return;
    int dx = point.x - local_swipe_origin.x, dy = point.y - local_swipe_origin.y;
    if (code == LV_EVENT_PRESSING && dx > 35 && abs(dy) < 35) {
        local_swipe_taken = true;
        lv_obj_set_style_border_width(row, 3, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0x1ed760), 0);
        lv_event_stop_bubbling(e);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_obj_set_style_border_width(row, 0, 0);
        if (local_swipe_taken) lv_event_stop_bubbling(e);
        if (code == LV_EVENT_RELEASED && dx > 75 && abs(dy) < 35 && playlist_priority_add(hit->path)) {
            device_state_queue_changed(); toast_glyph(&icon_list_music, "added_to_the_queue");
        }
    }
}

/* A copied selection outlives asynchronous search result replacement. */
static discovery_item_t online_selection;
static void online_download_cb(lv_event_t *e) {
    (void)e;
    if (!gui_card_available()) { gui_notify_no_card(); return; }
    char error[192] = "Could not open the download queue.";
    unsigned id = 0;
    if (!download_queue_init(cfg->sd_root_path) ||
        !(id = download_enqueue(&online_selection, error, sizeof(error)))) {
        gui_notify_popup(error);
        return;
    }
    download_resume(id);
    bottomsheet_close(false);
    main_menu_open_downloads();
}
static void online_artist_cb(lv_event_t *e) {
    (void)e;
    char artist[128];snprintf(artist,sizeof(artist),"%s",online_selection.artist);
    bottomsheet_close(false);
    switch_screen(main_menu_screen);show_page(SEARCH);
    request_instant=false;start_search_page(artist,false,true,1);
}
static void online_queue_cb(lv_event_t *e) {
    (void)e;bottomsheet_close(false);main_menu_open_downloads();
}
static void online_play_existing_cb(lv_event_t *e) {
    (void)e;download_job_t job;
    char local_title[256];
    if(download_lookup(&online_selection,&job) && access(job.path,R_OK)==0 && library_track_title(job.path,local_title,sizeof(local_title))) {
        bottomsheet_close(false);player_play_file(job.path);
    } else gui_notify_popup("This file is no longer available on the card.");
}
static void online_track_sheet(const discovery_item_t *item) {
    online_selection=*item;
    int w=cfg->screen_width-40;
    lv_obj_t *body=bottomsheet_open(cfg->screen_height-120,NULL);
    lv_obj_t *content=column(body,w,lv_obj_get_height(body)-24);
    lv_obj_set_pos(content,20,12);
    lv_obj_set_style_bg_opa(content,LV_OPA_TRANSP,0);
    lv_obj_t *title=label(content,item->title,&font_ui_24_bold,w,false);
    lv_obj_set_height(title,68);
    lv_obj_t *artist=label(content,item->artist,&font_ui_18,w,true);
    lv_obj_set_height(artist,26);
    lv_obj_t *album=label(content,item->album,&font_ui_18,w,true);
    lv_obj_set_height(album,50);
    char technical[128];snprintf(technical,sizeof(technical),"%s  /  %.1f MB  /  Original source",item->format,(double)item->bytes/1048576);
    lv_obj_t *detail=label(content,technical,&font_ui_18,w,false);
    lv_obj_set_height(detail,28);
    label(content,discovery_provider_name(),&font_ui_18,w,true);
    lv_obj_t *license=label(content,item->license,&font_ui_16,w,true);
    lv_obj_set_height(license,48);
    lv_label_set_long_mode(license,LV_LABEL_LONG_WRAP);
    download_job_t existing;
    bool found=download_lookup(item,&existing);
    char local_title[256];
    bool playable=found && access(existing.path,R_OK)==0 && library_track_title(existing.path,local_title,sizeof(local_title));
    if(playable)button(content,"Play offline copy",w,56,online_play_existing_cb,NULL);
    else if(discovery_item_valid(item) && (!found || existing.state==DOWNLOAD_PAUSED || existing.state==DOWNLOAD_FAILED)) {
        lv_obj_t *download=button(content,found?"Resume download":"Download to microSD",w,56,online_download_cb,NULL);
        lv_obj_set_style_bg_color(download,lv_color_hex(0x167c40),0);
    }
    if(found)label(content,download_state_name(existing.state),&font_ui_18,w,true);
    if(item->artist[0]) button(content,"More by this artist",w,52,online_artist_cb,NULL);
    button(content,"View downloads",w,52,online_queue_cb,NULL);
}
static void online_hit_cb(lv_event_t *e) {
    discovery_item_t *item = lv_event_get_user_data(e);
    if (!tapped()) return;
    if (item->kind == DISCOVERY_ALBUM) {
        snprintf(release_artist, sizeof(release_artist), "%s", item->artist);
        start_search(item->id, true);
        return;
    }
    online_track_sheet(item);
}
#ifdef HOST_BUILD
/* Render a fixture through the production sheet; never a provider/network override. */
void main_menu_probe_online_track(const discovery_item_t *item) {
    switch_screen(main_menu_screen);
    show_page(SEARCH);
    online_track_sheet(item);
}
void main_menu_probe_online_album(const discovery_results_t *results) {
    switch_screen(main_menu_screen);show_page(SEARCH);
    online_album_open(cfg,results,1,online_track_sheet,page_cb,artist_cb);
}
void main_menu_probe_search_text(const char *text) {
    switch_screen(main_menu_screen);show_page(SEARCH);
    lv_textarea_set_text(search_field,text);
}
#endif

static void search_draw(void) {
    search_art_drop();
    lv_obj_clean(search_results);
    lv_obj_scroll_to_y(search_results, 0, LV_ANIM_OFF);
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
    const char *groups[] = {"Songs", "Albums", "Artists", "Playlists", "Lyrics"};
    for (int kind = 0; kind < 5; kind++) {
        bool heading_drawn = false;
        for (int i = 0; i < shown.local_count; i++) {
            local_hit_t *hit = &shown.local[i];
            if ((int)hit->type != kind) continue;
            if (!heading_drawn) { section(search_results, groups[kind]); heading_drawn = true; }
            char name[256];
            if (kind == LIBRARY_SEARCH_ALBUM) library_album_title(hit->name, name, sizeof(name));
            else snprintf(name, sizeof(name), "%s", hit->name);
            int rw=cfg->screen_width-2*PAD;
            lv_obj_t *row=button(search_results,NULL,rw,hit->excerpt[0]?110:84,local_hit_cb,hit);
            lv_obj_set_style_bg_opa(row,LV_OPA_TRANSP,0);
            instant_images[i]=lv_image_create(row);lv_image_set_src(instant_images[i],kind==LIBRARY_SEARCH_ARTIST?&icon_artist:&icon_album);
            lv_obj_set_size(instant_images[i],58,58);lv_obj_set_pos(instant_images[i],2,8);
            lv_image_set_inner_align(instant_images[i],LV_IMAGE_ALIGN_CONTAIN);
            if(shown.artwork[i][0]) coverloader_request(COVERLOADER_INSTANT_BASE+i,shown.artwork[i],58);
            lv_obj_t *title=label(row,name,&font_ui_22,rw-88,false);lv_obj_set_pos(title,76,8);lv_obj_set_height(title,30);
            char subtitle[300];snprintf(subtitle,sizeof(subtitle),"%s%s%s",(const char *[]){"Song","Album","Artist","Playlist","Lyrics"}[kind],shown.artist[i][0]?" \xc2\xb7 ":"",shown.artist[i]);
            lv_obj_t *sub=label(row,subtitle,&font_ui_16,rw-88,true);lv_obj_set_pos(sub,76,42);lv_obj_set_height(sub,24);
            if(hit->excerpt[0]) {
                lv_obj_t *excerpt=label(row,hit->excerpt,&font_ui_16,rw-88,true);lv_obj_set_pos(excerpt,76,72);lv_obj_set_height(excerpt,24);
            }
            lv_obj_add_event_cb(row,local_queue_cb,LV_EVENT_PRESSED,hit);
            lv_obj_add_event_cb(row,local_queue_cb,LV_EVENT_PRESSING,hit);
            lv_obj_add_event_cb(row,local_queue_cb,LV_EVENT_RELEASED,hit);
            lv_obj_add_event_cb(row,local_queue_cb,LV_EVENT_PRESS_LOST,hit);
        }
    }
    if (request_album) {
        lv_obj_t *nav = box(search_results, cfg->screen_width - 2 * PAD, 46, false);
        lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(nav, 12, 0);
        button(nav, "Results", 128, 46, results_cb, NULL);
        if (release_artist[0]) button(nav, "Artist releases", 210, 46, artist_cb, NULL);
    } else if (request_artist) {
        lv_obj_t *artist_name = label(search_results, request_text, &font_ui_22_bold, cfg->screen_width - 2 * PAD, false);
        lv_obj_set_height(artist_name, 30);
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
        else snprintf(subtitle, sizeof(subtitle), "%s  /  %.1f MB  /  Details", item->format, (double)item->bytes / 1048576);
        lv_obj_t *sub = label(b, subtitle, &font_ui_16, cfg->screen_width - 76, true);
        lv_obj_set_height(sub, 24);
        lv_obj_set_pos(sub, 14, 48);
    }
    if (shown.error[0]) {
        lv_obj_t *err = label(search_results, shown.error, &font_ui_18, cfg->screen_width - 2 * PAD, true);
        lv_label_set_long_mode(err, LV_LABEL_LONG_WRAP);
    }
    char status[128];
    snprintf(status,sizeof(status),"%d local / %d online%s",shown.local_count,shown.online.count,
        (online_wanted || (search_busy && online_epoch==search_epoch))?" / Loading online...":"");
    lv_label_set_text(search_status, status);
    if (!local_wanted && !local_busy && !online_wanted && !search_busy && !shown.local_count && !shown.online.count && !shown.error[0])
        label(search_results, shown.online.truncated ? "No eligible releases on this page. Try the next page." :
              "No matches. Try an artist or album name.", &font_ui_18, cfg->screen_width - 2 * PAD, true);
    if (request_page > 1 || shown.online.truncated) {
        lv_obj_t *nav = box(search_results, cfg->screen_width - 2 * PAD, 52, false);
        lv_obj_set_flex_flow(nav, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(nav, 12, 0);
        if (request_page > 1) button(nav, "Previous", 180, 52, page_cb, (void *)(intptr_t)-1);
        if (shown.online.truncated && request_page < DISCOVERY_MAX_PAGES)
            button(nav, "Next page", 180, 52, page_cb, (void *)(intptr_t)1);
    }
    if (shown.local_count >= LOCAL_HITS / 3)
        route(search_results, "Search the full local library", "This overview shows up to six hits per category", &icon_search, OPEN_LOCAL_SEARCH);
}

static char browse_genres[2][256];
static int browse_genre_count;
static bool browse_genre_row(const char *name, const char *path, const char *artist, void *user) {
    (void)path; (void)artist; (void)user;
    if (browse_genre_count == 2) return false;
    snprintf(browse_genres[browse_genre_count++], 256, "%s", name);
    return browse_genre_count < 2;
}
static void browse_genre_cb(lv_event_t *e) {
    const char *genre=lv_event_get_user_data(e);
    if (tapped()) medialist_open(genre, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_GENRE, genre);
}
static lv_obj_t *browse_pair(const char *title) {
    section(search_results,title);
    lv_obj_t *pair=box(search_results,cfg->screen_width-2*PAD,126,false);
    lv_obj_set_flex_flow(pair,LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(pair,12,0);
    return pair;
}
static void browse_tile(lv_obj_t *pair,const char *title,uint32_t color,const lv_image_dsc_t *icon,int action,const char *genre) {
    int w=(cfg->screen_width-2*PAD-12)/2;
    lv_obj_t *tile=button(pair,NULL,w,118,genre?browse_genre_cb:action_cb,genre?(void *)genre:(void *)(intptr_t)action);
    lv_obj_set_style_bg_color(tile,lv_color_hex(color),0);
    lv_obj_set_style_bg_opa(tile,LV_OPA_COVER,0);
    lv_obj_set_style_radius(tile,6,0);
    lv_obj_t *t=label(tile,title,&font_ui_22_bold,w-24,false);
    lv_obj_set_style_text_color(t,lv_color_white(),0);lv_obj_set_pos(t,14,14);
    lv_obj_t *sleeve=box(tile,68,68,true);
    lv_obj_set_style_bg_color(sleeve,lv_color_darken(lv_color_hex(color),LV_OPA_50),0);
    lv_obj_set_style_radius(sleeve,3,0);lv_obj_set_pos(sleeve,w-64,62);
    lv_obj_set_style_transform_rotation(sleeve,180,0);
    lv_obj_t *im=glyph(sleeve,icon,34);lv_obj_center(im);
    lv_obj_set_style_image_recolor(im,lv_color_white(),0);
}
static void search_clear_cb(lv_event_t *e) { (void)e;lv_textarea_set_text(search_field,""); }

static void search_browse(void) {
    search_art_drop();
    lv_obj_clean(search_results);
    browse_genre_count=0;
    library_for_each(LIBRARY_LIST_GENRES,LIBRARY_FILTER_NONE,NULL,browse_genre_row,NULL);
    lv_obj_t *pair=browse_pair(browse_genre_count?"Your genres":"Your music");
    browse_tile(pair,browse_genre_count?browse_genres[0]:"Songs",0x9950ad,&icon_music2,OPEN_TRACKS,browse_genre_count?browse_genres[0]:NULL);
    browse_tile(pair,browse_genre_count>1?browse_genres[1]:"Albums",0x637e25,&icon_album,OPEN_ALBUMS,browse_genre_count>1?browse_genres[1]:NULL);
    pair=browse_pair("Explore your collection");
    browse_tile(pair,"Podcasts",0x2865c7,&icon_podcast_list,OPEN_PODCASTS,NULL);
    browse_tile(pair,"Playlists",0xc44822,&icon_list_music,OPEN_PLAYLISTS,NULL);
    pair=browse_pair("Browse all");
    browse_tile(pair,"Liked Songs",0x8170b3,&icon_heart,OPEN_FAVOURITES,NULL);
    browse_tile(pair,"Artists",0x23375f,&icon_artist,OPEN_ARTISTS,NULL);
    pair=browse_pair("More music");
    browse_tile(pair,"Genres",0x3d8964,&icon_genre,OPEN_GENRES,NULL);
    browse_tile(pair,"Downloads",0xb45b23,&icon_download,OPEN_DOWNLOADS,NULL);
    route(search_results,"Streaming & radio","Your connected music services",&icon_wifi,OPEN_STREAMING);
    route(search_results,"Files & folders","Browse the microSD card",&icon_folder,OPEN_BROWSE);
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
    lv_textarea_set_placeholder_text(search_field, "Music, lyrics or a release link");
    lv_obj_add_style(search_field, &theme_style_card, 0);
    lv_obj_add_style(search_field, &theme_style_text, 0);
    lv_obj_set_style_text_font(search_field, &font_ui_20, 0);
    lv_obj_set_style_bg_color(search_field,lv_color_hex(0x292929),0);
    lv_obj_set_style_text_color(search_field,lv_color_white(),0);
    lv_obj_set_style_radius(search_field,28,0);
    lv_obj_set_style_pad_left(search_field,48,0);lv_obj_set_style_pad_right(search_field,50,0);
    lv_obj_set_style_text_color(search_field,lv_color_hex(0xb3b3b3),LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_t *mag=lv_image_create(p);lv_image_set_src(mag,&icon_search);lv_obj_set_size(mag,26,26);
    lv_image_set_inner_align(mag,LV_IMAGE_ALIGN_CONTAIN);lv_obj_set_pos(mag,14,16);
    lv_obj_t *clear=button(p,"",44,52,search_clear_cb,NULL);lv_obj_set_pos(clear,cfg->screen_width-2*PAD-46,3);
    lv_obj_set_style_bg_opa(clear,0,0);lv_obj_t *x=lv_image_create(clear);lv_image_set_src(x,&icon_close);
    lv_obj_set_size(x,24,24);lv_image_set_inner_align(x,LV_IMAGE_ALIGN_CONTAIN);lv_obj_center(x);
    keyboard_style_caret(search_field);
    lv_obj_add_event_cb(search_field, field_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(search_field, instant_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
    const char *names[] = {"All", "Local", "Online"};
    for (int i = 0; i < 3; i++) {
        filter_buttons[i] = button(p, names[i], 110, 46, filter_cb, (void *)(intptr_t)i);
        lv_obj_set_pos(filter_buttons[i], i * 120, 68);
        lv_obj_set_style_radius(filter_buttons[i],24,0);
        lv_obj_set_style_bg_opa(filter_buttons[i],LV_OPA_TRANSP,0);
        lv_obj_set_style_border_color(filter_buttons[i],lv_color_hex(0x555555),0);
        lv_obj_set_style_border_width(filter_buttons[i],1,0);
    }
    lv_obj_set_style_border_width(filter_buttons[0], 2, 0);
    lv_obj_set_style_border_color(filter_buttons[0], mint(), 0);
    lv_obj_t *sources = button(p, "Sources", 80, 46, action_cb, (void *)OPEN_PROVIDERS);
    lv_obj_set_style_text_font(lv_obj_get_child(sources, 0), &font_ui_14, 0);
    lv_obj_set_height(lv_obj_get_child(sources, 0), 20);
    lv_obj_set_pos(sources, 360, 68);
    search_status = label(p, "", &font_ui_16, cfg->screen_width - 2 * PAD, true);
    lv_obj_set_pos(search_status, 0, 122);
    search_results = column(p, cfg->screen_width - 2 * PAD, body_height() - 150);
    lv_obj_set_pos(search_results, 0, 150);
    search_browse();
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
    if (action == 0) {
        bool enabled=!config_get_bool("discovery","enabled",true);
        config_set_bool("discovery","enabled",enabled);
        if(!enabled){search_epoch++;online_wanted=false;memset(&shown.online,0,sizeof(shown.online));}
    }
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
    lv_obj_t *note = label(p, "Search music or paste an Archive release link. Open a track to choose a download. No sign-in required.",
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
    note = label(p, "Supported: paged release search, direct release links, artist release lists, track details and licensed audio downloads. "
        "Artist lists follow source creator names, which may include several contributors. "
        "Not supported: streaming previews, artist biographies, online playlists or automatic provider fallback. "
        "Downloads keep the original audio quality and embedded tags, with an attribution sidecar for missing metadata.",
        &font_ui_18, cfg->screen_width - 2 * PAD, true);
    lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);
    route(p, "Wi-Fi settings", "Choose a network", &icon_wifi, OPEN_WIFI);
}

static download_job_t jobs[DOWNLOAD_MAX_JOBS];
static lv_obj_t *job_group[DOWNLOAD_MAX_JOBS], *job_cancel[DOWNLOAD_MAX_JOBS];
static lv_obj_t *job_title[DOWNLOAD_MAX_JOBS], *job_status[DOWNLOAD_MAX_JOBS], *job_card[DOWNLOAD_MAX_JOBS];
static int job_count;

static void job_cb(lv_event_t *e) {
    intptr_t index = (intptr_t)lv_event_get_user_data(e);
    if (!tapped() || index < 0 || index >= job_count) return;
    download_job_t *job = &jobs[index];
    if (job->state == DOWNLOAD_DONE) {
        if (!gui_card_available()) { gui_notify_no_card(); return; }
        player_play_file(job->path);
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
        job_group[i]=label(p,"",&font_ui_18,cfg->screen_width-2*PAD,false);
        lv_obj_set_height(job_group[i],52);
        job_card[i] = button(p, NULL, cfg->screen_width - 2 * PAD, 112, job_cb, (void *)(intptr_t)i);
        job_title[i] = label(job_card[i], "", &font_ui_20, cfg->screen_width - 140, false);
        lv_obj_set_height(job_title[i], 28);
        lv_obj_set_pos(job_title[i], 12, 10);
        job_status[i] = label(job_card[i], "", &font_ui_16, cfg->screen_width - 140, true);
        lv_obj_set_pos(job_status[i], 12, 42);
        lv_label_set_long_mode(job_status[i], LV_LABEL_LONG_WRAP);
        lv_obj_t *cancel = job_cancel[i] = button(job_card[i], "X", 52, 52, cancel_cb, (void *)(intptr_t)i);
        lv_obj_set_event_bubble(cancel, false);
        lv_obj_align(cancel, LV_ALIGN_RIGHT_MID, -8, 0);
        lv_obj_set_hidden(job_card[i], true);
    }
    route(p, "Listen offline", "Open your indexed music", &icon_music2, OPEN_TRACKS);
}

static void downloads_update(void) {
    job_count = download_snapshot(jobs, DOWNLOAD_MAX_JOBS);
    /* Keep release members adjacent without altering transfer scheduling. */
    for(int i=1;i<job_count;i++) {
        download_job_t job=jobs[i];int j=i;
        while(j>0 && strcmp(jobs[j-1].item.id,job.item.id)>0){jobs[j]=jobs[j-1];j--;}
        jobs[j]=job;
    }
    for (int i = 0; i < DOWNLOAD_MAX_JOBS; i++) {
        lv_obj_set_hidden(job_card[i], i >= job_count);
        bool group=i<job_count && (!i || strcmp(jobs[i-1].item.id,jobs[i].item.id));
        lv_obj_set_hidden(job_group[i],!group);
        if(group) {
            int total=0,done=0;
            for(int j=i;j<job_count && !strcmp(jobs[i].item.id,jobs[j].item.id);j++) {total++;done+=jobs[j].state==DOWNLOAD_DONE;}
            char title[256];snprintf(title,sizeof(title),"%.180s\n%d of %d downloaded",jobs[i].item.album,done,total);
            lv_label_set_text(job_group[i],title);
        }
        if (i >= job_count) continue;
        download_job_t *job = &jobs[i];
        char local_title[256];
        bool indexed=job->state==DOWNLOAD_DONE && library_track_title(job->path,local_title,sizeof(local_title));
        lv_label_set_text(job_title[i],indexed?local_title:job->item.title);
        lv_obj_set_hidden(job_cancel[i],job->state==DOWNLOAD_DONE || job->state==DOWNLOAD_CANCELLED);
        char status[256];
        snprintf(status, sizeof(status), "%s  /  %.1f / %.1f MB\n%s", download_state_name(job->state),
                 (double)job->done / 1048576, (double)job->total / 1048576, job->error);
        if (job->state == DOWNLOAD_RUNNING)
            snprintf(status, sizeof(status), "%d%% / %s / %u KB/s\nTap to pause",
                     job->total?(int)(100*job->done/job->total):0, job->item.format, job->bytes_per_second / 1024);
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
        lv_obj_set_style_bg_color(mini,lv_color_hex(0x252525),0);
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
            if(mini_image.has_tone) lv_obj_set_style_bg_color(mini,lv_color_darken(lv_color_hex(mini_image.tone),LV_OPA_70),0);
        }
    }
}

static void poll_cb(lv_timer_t *timer) {
    (void)timer;
    search_art_update();
    if(instant_pending && lv_tick_elaps(instant_changed)>=180)start_instant();
    if(local_busy) {
        pthread_mutex_lock(&search_lock);bool ready=local_ready;pthread_mutex_unlock(&search_lock);
        if(ready) {
            local_busy=false;
            if(local_epoch==search_epoch) {
                shown.local_count=pending.local_count;
                memcpy(shown.local,pending.local,sizeof(shown.local));
                memcpy(shown.artwork,pending.artwork,sizeof(shown.artwork));
                memcpy(shown.artist,pending.artist,sizeof(shown.artist));
                search_draw();
            }
        }
    }
    if(search_busy) {
        pthread_mutex_lock(&search_lock);bool ready=search_ready;pthread_mutex_unlock(&search_lock);
        if(ready) {
            search_busy=false;
            if(online_epoch==search_epoch) {
                if(request_album) {
                    /* Keep the previous results and scroll position under the release page. */
                    lv_label_set_text(search_status,online_error);
                    if(online_pending.count && selected==SEARCH &&
                       (lv_screen_active()==main_menu_screen || online_album_active())) {
                        snprintf(release_artist,sizeof(release_artist),"%s",online_pending.items[0].artist);
                        online_album_open(cfg,&online_pending,request_page,online_track_sheet,page_cb,artist_cb);
                    }
                } else {
                    shown.online=online_pending;snprintf(shown.error,sizeof(shown.error),"%s",online_error);
                    search_draw();
                }
            }
        }
    }
    dispatch_search();
    if (player_sheet_is_open()) return;
    if(mini && lv_screen_active()==lv_obj_get_parent(mini)) mini_update();
    if (lv_screen_active() != main_menu_screen) return;
    if (selected == HOME) {
        /* Apply a completed import only at the top of an idle feed. A user
         * browsing a shelf keeps its order until returning to Home. */
        if (home_revision!=library_revision(LIBRARY_LIST_TRACKS) && !library_scan_running()
            && !bottomsheet_active() && lv_obj_get_scroll_y(page[HOME])==0
            && lv_display_get_inactive_time(NULL)>2000) home_build();
        home_art_update();
    }
    if (selected == DOWNLOADS) downloads_update();
}

int main_menu_mount_footer(lv_obj_t *screen) {
    if(!mini || !screen) return 0;
    lv_obj_set_parent(mini,screen);lv_obj_set_hidden(mini,false);lv_obj_move_foreground(mini);
    for(int i=0;i<3;i++) {lv_obj_set_parent(nav_objects[i],screen);lv_obj_move_foreground(nav_objects[i]);}
    mini_update();
    return NAV_H+MINI_H+12;
}
void main_menu_restore_footer(lv_obj_t *screen) {
    if(mini && lv_obj_get_parent(mini)==screen) main_menu_mount_footer(main_menu_screen);
}
static void nav_cb(lv_event_t *e) {
    if(!tapped()) return;
    int tab=(int)(intptr_t)lv_event_get_user_data(e);
    if(lv_screen_active()!=main_menu_screen) {main_menu_mount_footer(main_menu_screen);switch_screen(main_menu_screen);}
    show_page(tab);
}

static void show_page(int tab) {
    if (tab < HOME || tab > PROVIDERS) return;
    selected = tab;
    if(tab<=LIBRARY) {config_set_int("ui","last_tab",tab);config_save();}
    if (tab != HOME) home_art_drop();
    if (search_keyboard) search_keyboard_visible(false);
    if (!page[tab]) {
        page[tab] = column(main_menu_screen, cfg->screen_width - 2 * PAD, body_height());
        lv_obj_set_pos(page[tab], PAD, cfg->top_bar_height + HEADER_H);
        if (tab == SEARCH) search_build();
        if (tab == LIBRARY) library_build();
        if (tab == DOWNLOADS) downloads_build();
        if (tab == PROVIDERS) providers_build();
    }
    for (int i = 0; i < 5; i++) if (page[i]) lv_obj_set_hidden(page[i], i != tab);
    const char *titles[] = {"Sonix", "Search", "Your Library", "Downloads", "Discovery"};
    lv_label_set_text(heading, titles[tab]);
    lv_obj_set_hidden(header_add,tab!=LIBRARY);
    lv_obj_set_hidden(header_search,tab!=LIBRARY);lv_obj_set_hidden(header_tools,tab==LIBRARY);
    lv_obj_set_hidden(header_settings,tab==LIBRARY);
    for (int i = 0; i < 3; i++) {
        bool active = i == (tab == DOWNLOADS ? LIBRARY : tab == PROVIDERS ? SEARCH : tab);
        lv_obj_set_style_image_recolor(nav_icon[i], active ? ink() : quiet(), 0);
        lv_obj_set_style_text_color(nav_label[i], active ? ink() : quiet(), 0);
    }
    if (tab == HOME) home_build();
    if (tab == DOWNLOADS) downloads_update();
    if(tab==LIBRARY) collectionview_refresh();
}

void main_menu_open_downloads(void) {
    if(lv_screen_active()!=main_menu_screen) switch_screen(main_menu_screen);
    show_page(DOWNLOADS);
}

static void screen_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_SCREEN_LOADED) {
        main_menu_mount_footer(main_menu_screen);
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
    if(theme_is_dark()) {lv_obj_set_style_bg_color(main_menu_screen,lv_color_hex(0x121212),0);topbar_tint_on_screen(main_menu_screen,true,lv_color_hex(0x121212),lv_color_white());}
    heading = label(main_menu_screen, "Sonix", &font_ui_28, cfg->screen_width - 140, false);
    lv_obj_set_pos(heading, PAD, cfg->top_bar_height + 17);
    lv_obj_t *tools = button(main_menu_screen, NULL, 48, 48, action_cb, (void *)OPEN_TOOLS);
    header_tools=tools;
    lv_obj_set_style_bg_opa(tools,LV_OPA_TRANSP,0);
    lv_obj_center(glyph(tools,&icon_ellipsis_vertical,26));
    lv_obj_set_pos(tools, cfg->screen_width - 122, cfg->top_bar_height + 10);
    lv_obj_t *settings = button(main_menu_screen, NULL, 60, 48, action_cb, (void *)OPEN_SETTINGS);
    header_settings=settings;
    lv_obj_set_style_bg_opa(settings,LV_OPA_TRANSP,0);
    lv_obj_set_pos(settings, cfg->screen_width - 72, cfg->top_bar_height + 10);
    lv_obj_t *settings_icon = glyph(settings, &icon_music_settings, 26);
    lv_obj_center(settings_icon);
    header_search=button(main_menu_screen,NULL,48,48,collectionview_search,NULL);
    lv_obj_set_pos(header_search,cfg->screen_width-132,cfg->top_bar_height+10);
    lv_obj_set_style_bg_opa(header_search,LV_OPA_TRANSP,0);lv_obj_center(glyph(header_search,&icon_search,26));
    header_add=button(main_menu_screen,NULL,52,48,collectionview_create_menu,NULL);
    lv_obj_set_pos(header_add,cfg->screen_width-72,cfg->top_bar_height+10);
    lv_obj_set_style_bg_opa(header_add,LV_OPA_TRANSP,0);
    lv_obj_center(glyph(header_add,&icon_plus,28));
    mini = button(main_menu_screen, NULL, cfg->screen_width - 24, MINI_H, mini_open_cb, NULL);
    lv_obj_set_pos(mini, 12, cfg->screen_height - NAV_H - MINI_H - 6);
    lv_obj_set_style_bg_color(mini,lv_color_hex(0x252525),0);
    lv_obj_set_style_radius(mini,6,0);
    mini_art = glyph(mini, &icon_music2, 44);
    lv_obj_set_pos(mini_art, 8, 8);
    mini_title = label(mini, "Choose something to play", &font_ui_18, cfg->screen_width - 166, false);
    lv_obj_set_height(mini_title, 25);
    lv_obj_set_pos(mini_title, 64, 8);
    mini_artist = label(mini, "", &font_ui_16, cfg->screen_width - 166, true);
    lv_obj_set_height(mini_artist, 23);
    lv_obj_set_pos(mini_artist, 64, 33);
    lv_obj_t *transport = button(mini, NULL, 54, 58, mini_play_cb, NULL);
    lv_obj_set_event_bubble(transport, false);
    lv_obj_set_style_bg_opa(transport,LV_OPA_TRANSP,0);
    lv_obj_align(transport, LV_ALIGN_RIGHT_MID, -5, -2);
    mini_play = glyph(transport, &icon_play, 28);
    lv_obj_center(mini_play);
    mini_progress = lv_bar_create(mini);
    lv_obj_set_size(mini_progress, cfg->screen_width - 44, 3);
    lv_obj_align(mini_progress, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_bar_set_range(mini_progress, 0, 1000);
    lv_obj_set_style_bg_color(mini_progress, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_shadow_width(mini_progress, 0, 0);
    const char *names[] = {"Home", "Search", "Your Library"};
    const lv_image_dsc_t *icons[] = {&sonix_icon_home, &icon_search, &sonix_icon_library};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *nav = button(main_menu_screen, NULL, cfg->screen_width / 3, NAV_H, nav_cb, (void *)(intptr_t)i);
        nav_objects[i]=nav;
        lv_obj_set_pos(nav, i * (cfg->screen_width / 3), cfg->screen_height - NAV_H);
        lv_obj_set_style_radius(nav, 0, 0);
        lv_obj_add_style(nav, &theme_style_panel, 0);
        if(theme_is_dark()) lv_obj_set_style_bg_color(nav,lv_color_hex(0x121212),0);
        nav_icon[i] = glyph(nav, icons[i], 26);
        lv_obj_align(nav_icon[i], LV_ALIGN_TOP_MID, 0, 9);
        nav_label[i] = label(nav, names[i], &font_ui_16, cfg->screen_width / 3 - 8, false);
        lv_obj_set_style_text_align(nav_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(nav_label[i], LV_ALIGN_BOTTOM_MID, 0, -10);
    }
    lv_obj_add_event_cb(main_menu_screen, screen_cb, LV_EVENT_SCREEN_LOADED, NULL);
    lv_obj_add_event_cb(main_menu_screen, screen_cb, LV_EVENT_SCREEN_UNLOADED, NULL);
    int last=(int)config_get_int("ui","last_tab",HOME);
    show_page(last>=HOME && last<=LIBRARY?last:HOME);
    lv_timer_create(poll_cb, 100, NULL);
    ui_probe_init();
}
