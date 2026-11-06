#include "src/gui/library/homeview.h"
#include "src/gui/library/playlistview.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/rowswipe.h"
#include "src/system/library/playlists.h"
#include "src/system/library/collections.h"
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
#include "src/gui/shell/deviceprofile.h"
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
#include "src/gui/wireless/wifisettings.h"
#include "src/gui/wireless/wireless.h"
#include "src/system/core/lang.h"
#include "src/system/core/config.h"
#include "src/system/library/library.h"
#include "src/system/net/wifi.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include "src/gui/library/collectionview.h"
#include "src/gui/library/activity.h"
#include "src/gui/settings/systempage.h"

/* Original Sonix branding and native widgets. Legacy pages retain their own
 * layouts, history and transport owners. Primary pages and the keyboard are
 * created on demand. One bounded local-index request runs at a time; workers never touch LVGL. */
lv_obj_t *main_menu_screen;
static lv_obj_t *tools_screen, *header_tools, *header_settings, *header_add, *header_search, *header_profile;
static gui_config_t *cfg;
static lv_obj_t *page[3], *heading, *nav_icon[3], *nav_label[3], *nav_objects[3];
static lv_obj_t *mini, *mini_title, *mini_artist, *mini_art, *mini_play, *mini_progress;
static int selected;
static bool tab_save_pending;
static uint32_t tab_changed;
static cover_image_t mini_image;
static char mini_path[512];
static bool mini_has_art;

enum { HOME, SEARCH, LIBRARY };

enum { OPEN_FAVOURITES, OPEN_ALBUMS, OPEN_ARTISTS, OPEN_TRACKS, OPEN_PLAYLISTS,
       OPEN_BROWSE, OPEN_MOST_PLAYED, OPEN_TOOLS, OPEN_SETTINGS,
       OPEN_CLASSIC_LIBRARY, OPEN_LOCAL_SEARCH, OPEN_WIFI, OPEN_GENRES, OPEN_COMPOSERS };

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
    case OPEN_COMPOSERS: activity_open_composers(); break;
    case OPEN_FAVOURITES: medialist_open(tr("favourites"), LIBRARY_LIST_FAVOURITES, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_ALBUMS: medialist_open(tr("albums"), LIBRARY_LIST_ALBUMS, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_ARTISTS: medialist_open(tr("artists"), LIBRARY_LIST_ARTISTS, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_TRACKS: medialist_open(tr("music_all_tracks"), LIBRARY_LIST_TRACKS, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_PLAYLISTS: if (theme_reference()) main_menu_open_library(1); else playlistpage_open(); break;
    case OPEN_BROWSE: switch_screen(browser_screen); break;
    case OPEN_MOST_PLAYED: medialist_open(tr("music_most_played"), LIBRARY_LIST_MOST_PLAYED, LIBRARY_FILTER_NONE, NULL); break;
    case OPEN_TOOLS: switch_screen(tools_screen); break;
    case OPEN_SETTINGS: switch_screen(settings_screen); break;
    case OPEN_CLASSIC_LIBRARY: switch_screen(music_screen); break;
    case OPEN_LOCAL_SEARCH: search_open(); break;
    case OPEN_WIFI: switch_screen(wifisettings_screen); break;

    }
}

/* The profile menu, as Spotify's: the round avatar beside every tab title
 * slides a drawer in from the left with the personal pages (Listening stats,
 * Recents, Settings) and, under a divider, the R1's own destinations. Tap the
 * dimmed page or drag the drawer left to close it. */
/* 432 on the 480 px screen; narrower at Display size Large (384 px), where a
 * fixed 432 ran off the right edge and left no dimmed page to tap. */
#define DRAWER_W (LV_MIN(432, (int)cfg->screen_width - 48))
static lv_obj_t *drawer_scrim, *drawer_panel;
static int drawer_drag_x;
static bool drawer_dragging;
typedef struct { const char *title; const lv_image_dsc_t *icon; lv_obj_t **target; void (*open)(void); } drawer_entry_t;
static const drawer_entry_t drawer_entries[] = {
    {"Listening stats", &sonix_icon_stats, NULL, activity_open_stats},
    {"Recents", &sonix_icon_recents, NULL, activity_open_recents},
    {"Settings", &sonix_icon_gear, &settings_screen, NULL},
    {NULL, NULL, NULL, NULL},
    {"Devices", &sonix_icon_speaker, &wireless_screen, NULL},
    {"Audiobooks", &sonix_icon_book, &audiobooks_screen, NULL},
    {"Browse music", &sonix_icon_music, &music_screen, NULL},
    {"More", &sonix_icon_grid, &morepage_screen, NULL},
};
bool main_menu_drawer_open(void) { return drawer_panel != NULL; }
static void drawer_x_anim(void *o, int32_t v) { lv_obj_set_style_translate_x(o, v, 0); }
static void scrim_opa_anim(void *o, int32_t v) { lv_obj_set_style_bg_opa(o, (lv_opa_t)v, 0); }
static void drawer_deleted(lv_anim_t *a) {
    (void)a;
    if (drawer_scrim) lv_obj_delete(drawer_scrim);
    drawer_scrim = drawer_panel = NULL;
}
static void drawer_close(bool animate) {
    if (!drawer_panel) return;
    lv_anim_delete(drawer_panel, drawer_x_anim);
    lv_anim_delete(drawer_scrim, scrim_opa_anim);
    if (!animate) { lv_obj_delete(drawer_scrim); drawer_scrim = drawer_panel = NULL; return; }
    lv_obj_set_clickable(drawer_scrim, false);
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, drawer_panel); lv_anim_set_exec_cb(&a, drawer_x_anim);
    lv_anim_set_values(&a, lv_obj_get_style_translate_x(drawer_panel, 0), -DRAWER_W);
    lv_anim_set_duration(&a, 180); lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_completed_cb(&a, drawer_deleted); lv_anim_start(&a);
    lv_anim_t b; lv_anim_init(&b);
    lv_anim_set_var(&b, drawer_scrim); lv_anim_set_exec_cb(&b, scrim_opa_anim);
    lv_anim_set_values(&b, lv_obj_get_style_bg_opa(drawer_scrim, 0), 0);
    lv_anim_set_duration(&b, 180); lv_anim_start(&b);
}
static void drawer_scrim_cb(lv_event_t *e) { if (lv_event_get_target(e) == drawer_scrim) drawer_close(true); }
static void drawer_pick_cb(lv_event_t *e) {
    if (drawer_dragging) return;
    const drawer_entry_t *d = lv_event_get_user_data(e);
    drawer_close(false);
    if (d->open) d->open();
    else if (d->target && *d->target) switch_screen(*d->target);
}
/* Dragging the drawer left follows the finger; past a third it closes. Rows
 * bubble their presses here, so the drag can start anywhere on the drawer. */
static void drawer_drag_cb(lv_event_t *e) {
    lv_indev_t *in = lv_indev_active();
    if (!in || !drawer_panel) return;
    lv_point_t p; lv_indev_get_point(in, &p);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) { drawer_drag_x = p.x; drawer_dragging = false; return; }
    int dx = p.x - drawer_drag_x;
    if (code == LV_EVENT_PRESSING) {
        if (!drawer_dragging && dx < -12) {
            drawer_dragging = true;
            lv_obj_t *t = lv_event_get_target(e);
            if (t != drawer_panel) lv_obj_remove_state(t, LV_STATE_PRESSED);
        }
        if (drawer_dragging) lv_obj_set_style_translate_x(drawer_panel, dx < 0 ? dx : 0, 0);
    } else if (drawer_dragging) {
        if (dx < -DRAWER_W / 3) drawer_close(true);
        else lv_obj_set_style_translate_x(drawer_panel, 0, 0);
    }
}
static lv_obj_t *drawer_item(lv_obj_t *parent, int h, int y) {
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, DRAWER_W, h);
    lv_obj_set_pos(row, 0, y);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
    lv_obj_set_clickable(row, true);
    lv_obj_set_scrollable(row, false);
    lv_obj_set_event_bubble(row, true);
    return row;
}
static void drawer_row(lv_obj_t *parent, const drawer_entry_t *d, int y) {
    lv_obj_t *row = drawer_item(parent, 60, y);
    lv_obj_add_event_cb(row, drawer_pick_cb, LV_EVENT_CLICKED, (void *)d);
    lv_obj_t *im = lv_image_create(row);
    lv_image_set_src(im, d->icon); lv_obj_set_size(im, 28, 28);
    lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
    lv_obj_set_style_image_recolor(im, lv_color_white(), 0);
    lv_obj_set_style_image_recolor_opa(im, LV_OPA_COVER, 0);
    lv_obj_align(im, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_t *t = label(row, d->title, &font_ui_20, DRAWER_W - 90, false);
    lv_obj_set_style_text_color(t, lv_color_white(), 0);
    lv_obj_align(t, LV_ALIGN_LEFT_MID, 68, 0);
}
static void drawer_divider(lv_obj_t *parent, int y) {
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_remove_style_all(line);
    lv_obj_set_size(line, DRAWER_W - 40, 1);
    lv_obj_set_pos(line, 20, y);
    lv_obj_set_style_bg_color(line, lv_color_hex(0x3a3a3a), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
}
/* The device's avatar: its picture, or Spotify's default -- the initial on a
 * coloured disc (deviceprofile.h; chosen in Settings > About). */
static void avatar(lv_obj_t *o, int size, const lv_font_t *font) { deviceprofile_avatar(o, size, font); }
static void device_info_cb(lv_event_t *e) {
    (void)e;
    if (drawer_dragging) return;
    drawer_close(false);
    switch_screen(sysinfo_screen); /* where the name and picture are */
}
static void device_menu_cb(lv_event_t *e) {
    (void)e;
    if (!tapped() || drawer_panel) return;
    int h = cfg->screen_height;
    drawer_scrim = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(drawer_scrim);
    lv_obj_set_size(drawer_scrim, cfg->screen_width, h);
    lv_obj_set_style_bg_color(drawer_scrim, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(drawer_scrim, 0, 0);
    lv_obj_set_clickable(drawer_scrim, true);
    lv_obj_add_event_cb(drawer_scrim, drawer_scrim_cb, LV_EVENT_CLICKED, NULL);
    drawer_panel = lv_obj_create(drawer_scrim);
    lv_obj_remove_style_all(drawer_panel);
    lv_obj_set_size(drawer_panel, DRAWER_W, h);
    lv_obj_set_style_bg_color(drawer_panel, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_bg_opa(drawer_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_translate_x(drawer_panel, -DRAWER_W, 0);
    lv_obj_set_clickable(drawer_panel, true);
    lv_obj_set_scrollable(drawer_panel, false);
    lv_obj_add_event_cb(drawer_panel, drawer_drag_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(drawer_panel, drawer_drag_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(drawer_panel, drawer_drag_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(drawer_panel, drawer_drag_cb, LV_EVENT_PRESS_LOST, NULL);
    /* Header: Spotify's profile card (iOS, 2026-11-04 captures) -- the
     * avatar, the device's name and a way to its details on a raised,
     * rounded card, the rows straight under it. */
    int y = cfg->top_bar_height + 16;
    lv_obj_t *head = drawer_item(drawer_panel, 80, y);
    lv_obj_set_width(head, DRAWER_W - 24);
    lv_obj_set_x(head, 12);
    lv_obj_set_style_radius(head, 12, 0);
    lv_obj_set_style_bg_color(head, lv_color_hex(0x242424), 0);
    lv_obj_set_style_bg_opa(head, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(head, lv_color_hex(0x333333), LV_STATE_PRESSED);
    lv_obj_add_event_cb(head, device_info_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *av = lv_obj_create(head);
    lv_obj_remove_style_all(av);
    lv_obj_set_size(av, 56, 56);
    lv_obj_set_pos(av, 12, 12);
    lv_obj_set_clickable(av, false);
    avatar(av, 56, &font_ui_24_bold);
    lv_obj_t *name = label(head, "", &font_ui_24_bold, DRAWER_W - 130, false);
    deviceprofile_name_label(name);
    lv_obj_set_style_text_color(name, lv_color_white(), 0);
    lv_obj_set_pos(name, 82, 14);
    lv_obj_t *sub = label(head, "View device info", &font_ui_16, DRAWER_W - 130, true);
    lv_obj_set_pos(sub, 82, 48);
    y += 80 + 16;
    int n = (int)(sizeof(drawer_entries) / sizeof(drawer_entries[0]));
    for (int i = 0; i < n; i++) {
        if (!drawer_entries[i].title) {
            /* Between Spotify's personal pages and the R1's own. */
            drawer_divider(drawer_panel, y + 8);
            y += 17;
            continue;
        }
        drawer_row(drawer_panel, &drawer_entries[i], y);
        y += 60;
    }
    lv_anim_t a; lv_anim_init(&a);
    lv_anim_set_var(&a, drawer_panel); lv_anim_set_exec_cb(&a, drawer_x_anim);
    lv_anim_set_values(&a, -DRAWER_W, 0); lv_anim_set_duration(&a, 220);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out); lv_anim_start(&a);
    lv_anim_t b; lv_anim_init(&b);
    lv_anim_set_var(&b, drawer_scrim); lv_anim_set_exec_cb(&b, scrim_opa_anim);
    lv_anim_set_values(&b, 0, LV_OPA_60); lv_anim_set_duration(&b, 220); lv_anim_start(&b);
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

/* Home is rebuilt only when something its shelves show has changed. It used
 * to be rebuilt on every return to it -- every Back, every tab tap -- and on
 * the R1 each rebuild was every shelf's queries and widgets again on the
 * interface thread (2026-11-05). Kept, it also keeps its scroll position. */
static void page_ensure(int tab);
static unsigned home_revision;
static bool home_built;
static unsigned home_signature(void) { return library_home_revision()+collections_revision(); }
static void home_build(void) {
    uint32_t started=lv_tick_get();
    homeview_build(page[HOME],cfg);
    home_revision=home_signature();
    home_built=true;
    printf("home: feed built in %u ms\n",(unsigned)lv_tick_elaps(started));
}
static void home_build_if_stale(void) {
    if(!home_built || home_revision!=home_signature()) home_build();
}
static void home_art_update(void) {homeview_art_update();}

static void library_build(void) {
    lv_obj_t *p = page[LIBRARY];
    lv_obj_set_scrollable(p, false);
    lv_obj_t *collections=box(p,cfg->screen_width-2*PAD,body_height(),false);
    collectionview_build(collections,cfg);
}

/* A single bounded local-index worker never touches LVGL. */
typedef library_instant_hit_t local_hit_t;
typedef struct {
    local_hit_t local[LOCAL_HITS];
    int local_count;
    char artwork[LOCAL_HITS][512], artist[LOCAL_HITS][256];
} search_result_t;
static search_result_t pending, shown;
static pthread_mutex_t search_lock = PTHREAD_MUTEX_INITIALIZER;
static bool local_busy, local_ready, local_wanted;
static unsigned local_epoch, search_epoch;
static char local_text[256], request_text[256], shown_text[256];
static lv_obj_t *search_field, *search_results, *search_status, *filter_buttons[6], *search_keyboard_parent;
static keyboard_t *search_keyboard;
static int search_filter;
static bool instant_pending;
static uint32_t instant_changed;
static void search_draw(void);

/* With a chip chosen the worker asks for that kind alone and the whole
 * LOCAL_HITS budget goes to it, as Spotify's chips narrow the results to a
 * longer list of one kind; -1 is every kind, LOCAL_HITS/5 each. */
static int local_type=-1, shown_type=-1;
static void local_collect(const library_instant_hit_t *item, void *user) {
    search_result_t *r = user;
    if (r->local_count >= LOCAL_HITS) return;
    if (local_type >= 0 && (int)item->type != local_type) return;
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
        } else if(hit->type==LIBRARY_SEARCH_ARTIST) {
            snprintf(pending.artwork[i],512,"%s",hit->path);
        } else {
            library_list_t kind=hit->type==LIBRARY_SEARCH_PLAYLIST?LIBRARY_LIST_PLAYLIST:LIBRARY_LIST_TRACKS;
            library_filter_t filter=hit->type==LIBRARY_SEARCH_ARTIST?LIBRARY_FILTER_ARTIST:hit->type==LIBRARY_SEARCH_ALBUM?LIBRARY_FILTER_ALBUM:LIBRARY_FILTER_NONE;
            library_index_t *ix=library_index_open(kind,filter,hit->name,LIBRARY_ORDER_DEFAULT,false);
            if(ix) { library_index_window(ix,0,1,search_sample,&i);library_index_close(ix); }
        }
    }
}
/* Browse as it was drawn: kept while the library is the same, so clearing the
 * field or coming back to it does not draw every tile again. */
static bool browse_shown;
static unsigned browse_revision;
static lv_obj_t *instant_images[LOCAL_HITS];
static cover_image_t instant_covers[LOCAL_HITS];
static void search_art_drop(void) {
    for(int i=0;i<LOCAL_HITS;i++) {
        coverloader_release(COVERLOADER_INSTANT_BASE+i);
        if(instant_images[i])lv_image_set_src(instant_images[i],&icon_album);
        cover_free(&instant_covers[i]);instant_images[i]=NULL;
    }
}
static void search_art_update(void) {
    for(int i=0;i<LOCAL_HITS;i++)if(instant_images[i]&&!instant_covers[i].pixels) {
        bool done=false;
        if(coverloader_take(COVERLOADER_INSTANT_BASE+i,&instant_covers[i],&done)) {
            lv_image_set_src(instant_images[i],&instant_covers[i].dsc);
            lv_obj_set_style_image_recolor_opa(instant_images[i],LV_OPA_TRANSP,0);
            lv_image_set_inner_align(instant_images[i],LV_IMAGE_ALIGN_CONTAIN);
        }
    }
}
static void *search_worker(void *unused) {
    (void)unused;
    memset(&pending,0,sizeof(pending));
    library_search_instant(local_text,local_type>=0?LOCAL_HITS:LOCAL_HITS/5,local_collect,&pending);
    search_resolve_art();
    pthread_mutex_lock(&search_lock);local_ready=true;pthread_mutex_unlock(&search_lock);
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
        local_type=search_filter?search_filter-1:-1;
        local_ready=false;local_busy=true;
        if(!start_worker(search_worker)){local_busy=false;lv_label_set_text(search_status,"Could not start local search.");}
    }
}
/* Spotify's browse page has no chips; they appear with a query. Without
 * them the tiles start under the field. */
static lv_obj_t *search_chips;
static int search_top=150;
static void search_keyboard_visible(bool visible);
static void search_chips_show(bool on) {
    if(!search_chips || lv_obj_is_hidden(search_chips)==!on) return;
    lv_obj_set_hidden(search_chips,!on);lv_obj_set_hidden(search_status,!on);
    search_top=on?150:68;lv_obj_set_pos(search_results,-PAD,search_top);
    search_keyboard_visible(!lv_obj_is_hidden(search_keyboard_parent));
}
static void start_instant(void);
static void search_keyboard_visible(bool visible) {
    /* Leaving an empty field shows Browse again, whatever emptied it. */
    if(!visible && search_field && !lv_textarea_get_text(search_field)[0] && search_chips && !lv_obj_is_hidden(search_chips)) {
        ++search_epoch;start_instant();
    }
    int top=search_top;
    keyboard_set_visible(search_keyboard,visible);
    lv_obj_set_hidden(search_keyboard_parent,!visible);lv_obj_set_hidden(mini,visible);
    int height=visible?cfg->screen_height-NAV_H-310-(cfg->top_bar_height+HEADER_H+top)-8:body_height()-top;
    lv_obj_set_height(search_results,height>40?height:40);
}
/* The clear button is there only once something is typed, as in Spotify;
 * empty, the field has the whole bar and the placeholder is not cut short. */
static lv_obj_t *search_clear_btn;
static void search_clear_sync(void) {
    if(!search_field || !search_clear_btn) return;
    bool empty=!lv_textarea_get_text(search_field)[0];
    lv_obj_set_hidden(search_clear_btn,empty);
    lv_obj_set_style_pad_right(search_field,empty?0:46,0);
}
static void instant_changed_cb(lv_event_t *e) {
    (void)e;++search_epoch;instant_changed=lv_tick_get();instant_pending=true;local_wanted=false;
    search_clear_sync();
}
static void start_instant(void) {
    instant_pending=false;
    snprintf(request_text,sizeof(request_text),"%s",lv_textarea_get_text(search_field));
    if(!request_text[0]) {local_wanted=false;memset(&shown,0,sizeof(shown));search_browse();lv_label_set_text(search_status,"");search_chips_show(false);return;}
    search_chips_show(true);
    local_wanted=true;lv_label_set_text(search_status,"Searching your music...");dispatch_search();
}
static void submit_cb(lv_event_t *e) {(void)e;start_instant();search_keyboard_visible(false);}
static void field_cb(lv_event_t *e) {(void)e;search_keyboard_visible(true);}
static void filter_cb(lv_event_t *e) {
    if(!tapped())return;
    int chosen=(int)(intptr_t)lv_event_get_user_data(e);search_filter=search_filter==chosen?0:chosen;
    for(int i=0;i<6;i++) {
        lv_obj_set_style_bg_opa(filter_buttons[i],i==search_filter?LV_OPA_COVER:LV_OPA_TRANSP,0);
        lv_obj_set_style_bg_color(filter_buttons[i],mint(),0);
        lv_obj_set_style_text_color(lv_obj_get_child(filter_buttons[i],0),i==search_filter?lv_color_black():ink(),0);
    }
    /* A different kind asks the index again rather than filtering the few
     * rows already shown. */
    if(lv_textarea_get_text(search_field)[0]){++search_epoch;start_instant();}
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
        /* Spotify continues a searched song with autoplay; offline, the song's
         * own album is the honest continuation, so it plays from that track in
         * the album context (never a filesystem folder). Without an indexed
         * album the song plays alone. Search playback stays in preview. */
        char album[256];
        library_index_t *ix=NULL;int start=-1;
        if(library_track_album_value(hit->path,album,sizeof(album)) && album[0]) {
            ix=library_index_open(LIBRARY_LIST_TRACKS,LIBRARY_FILTER_ALBUM,album,LIBRARY_ORDER_DEFAULT,false);
            if(ix) start=library_index_find_path(ix,hit->path);
        }
        if(ix && start>=0) {
            /* The handle is taken whether or not playback starts. */
            if(device_state_play_index(ix,start)) {playlist_set_album(album);player_refresh_now_playing();}
        } else {
            if(ix) library_index_close(ix);
            player_play_file(hit->path);
        }
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
    /* The shared row swipe: the same strip and threshold as every song list. */
    static const rowswipe_spec_t spec = {&icon_list_plus, ROWSWIPE_GREEN, NULL, 0};
    int committed = 0;
    bool swiping = rowswipe_event(e, row, &spec, &committed);
    if (code == LV_EVENT_PRESSED) {
        local_swipe_row = row; local_swipe_origin = point; local_swipe_taken = false;
        return;
    }
    if (local_swipe_row != row) return;
    if (swiping) { local_swipe_taken = true; lv_event_stop_bubbling(e); }
    if (committed > 0 && playlist_priority_add(hit->path)) {
        device_state_queue_changed(); toast_glyph(&icon_list_music, "added_to_the_queue");
    }
}

/* A group that filled its instant slots continues in the shared collection
 * list of every match ("See all songs"), the same list, rows and menus as the
 * rest of the library. Playlists and lyrics have no full search list yet. */
static void see_all_cb(lv_event_t *e) {
    if (!tapped()) return;
    int kind=(int)(intptr_t)lv_event_get_user_data(e);
    static const library_list_t lists[]={LIBRARY_LIST_TRACKS,LIBRARY_LIST_ALBUMS,LIBRARY_LIST_ARTISTS};
    char title[200];snprintf(title,sizeof(title),"\xe2\x80\x9c%.180s\xe2\x80\x9d",shown_text);
    medialist_open(title,lists[kind],LIBRARY_FILTER_SEARCH,shown_text);
}
static void see_all_row(int kind) {
    static const char *names[]={"See all songs","See all albums","See all artists"};
    int rw=cfg->screen_width-2*PAD;
    lv_obj_t *row=button(search_results,NULL,rw,56,see_all_cb,(void *)(intptr_t)kind);
    lv_obj_set_style_bg_opa(row,LV_OPA_TRANSP,0);
    lv_obj_t *t=label(row,names[kind],&font_ui_20_bold,rw-40,false);
    lv_obj_set_pos(t,2,14);
    lv_obj_t *chevron=glyph(row,&icon_chevron_right,24);
    lv_obj_align(chevron,LV_ALIGN_RIGHT_MID,-4,0);
    lv_obj_set_style_image_recolor(chevron,lv_color_hex(0xb3b3b3),0);
    lv_obj_set_style_image_recolor_opa(chevron,LV_OPA_COVER,0);
}

static void search_draw(void) {
    browse_shown=false;
    search_art_drop();lv_obj_clean(search_results);lv_obj_scroll_to_y(search_results,0,LV_ANIM_OFF);
    int count=0;
    const char *groups[] = {"Songs", "Albums", "Artists", "Playlists", "Lyrics"};
    for (int kind = 0; kind < 5; kind++) {
        if(search_filter && kind!=search_filter-1)continue;
        bool heading_drawn = false;
        int in_group = 0;
        for (int i = 0; i < shown.local_count; i++) {
            local_hit_t *hit = &shown.local[i];
            if ((int)hit->type != kind) continue;
            count++;in_group++;
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
        if(in_group>=(shown_type>=0?LOCAL_HITS:LOCAL_HITS/5) && kind<=LIBRARY_SEARCH_ARTIST) see_all_row(kind);
    }
    /* Spotify shows no result count, and a capped instant count would be
     * wrong for a long list; the line only reports transient states. */
    lv_label_set_text(search_status,"");
    if(!count)label(search_results,"No matches. Try a song, artist or lyric.",&font_ui_18,cfg->screen_width-2*PAD,true);
}
/* The one search of the Spotify-style experience, for pages that offer one. */
void main_menu_open_search(void) {
    switch_screen(main_menu_screen);show_page(SEARCH);
}
/* Your Library on one chip; the playlists live there, not on a page of their own. */
void main_menu_open_library(int filter) {
    collectionview_set_filter(filter);switch_screen(main_menu_screen);show_page(LIBRARY);
}
#ifdef HOST_BUILD
void main_menu_probe_search_text(const char *text) {
    switch_screen(main_menu_screen);show_page(SEARCH);lv_textarea_set_text(search_field,text);++search_epoch;start_instant();search_keyboard_visible(false);
}
#endif

/* Search's browse page, as Spotify lays it out: your top genres first, in a
 * carousel of tiles that runs to the screen edge, then "Browse all" -- one grid
 * of coloured tiles, each with a sleeve tilted into its corner. The sleeves are
 * real covers from the library where there is one, loaded through the instant
 * results' slots (the two never show at once). */
#define BROWSE_GENRES 10
static char browse_genres[BROWSE_GENRES][256];
static int browse_genre_count;
static int browse_art_slot;
static const uint32_t TILE_COLORS[] = {0xe8115b, 0x006450, 0x8400e7, 0x1e3264, 0xdc148c, 0x477d95,
                                       0xaf2896, 0x509bf5, 0xba5d07, 0x148a08, 0xe13300, 0x7358ff};
static void browse_genre_cb(lv_event_t *e) {
    const char *genre=lv_event_get_user_data(e);
    if (tapped()) medialist_open(genre, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_GENRE, genre);
}
/* A track of that list to take the sleeve from. */
typedef struct {
    char genres[BROWSE_GENRES][256], genre_art[BROWSE_GENRES][512], tile_art[6][512];
    int count;
    unsigned revision;
} browse_data_t;
static browse_data_t browse_data, browse_pending;
static bool browse_busy, browse_valid;
static int browse_ready;
static bool browse_collect(const char *name,const char *path,const char *artist,void *unused) {
    (void)path;(void)artist;(void)unused;
    if(browse_pending.count>=BROWSE_GENRES)return false;
    snprintf(browse_pending.genres[browse_pending.count++],256,"%s",name);
    return browse_pending.count<BROWSE_GENRES;
}
static void *browse_worker(void *unused) {
    (void)unused;
    library_for_each(LIBRARY_LIST_GENRES,LIBRARY_FILTER_NONE,NULL,browse_collect,NULL);
    for(int n=0;n<browse_pending.count;n++)
        library_sample_path(LIBRARY_LIST_TRACKS,LIBRARY_FILTER_GENRE,browse_pending.genres[n],0,browse_pending.genre_art[n],512);
    for(int n=0;n<6;n++)
        library_sample_path(n==5?LIBRARY_LIST_MOST_PLAYED:LIBRARY_LIST_TRACKS,LIBRARY_FILTER_NONE,NULL,n,browse_pending.tile_art[n],512);
    __atomic_store_n(&browse_ready,1,__ATOMIC_RELEASE);return NULL;
}
static void browse_poll(void) {
    if(browse_busy && __atomic_load_n(&browse_ready,__ATOMIC_ACQUIRE)) {
        browse_busy=false;browse_data=browse_pending;browse_valid=true;
        browse_shown=false;
        if(search_field && !lv_textarea_get_text(search_field)[0])search_browse();
    }
}
static void browse_tile(lv_obj_t *parent, int w, int h, const char *title, uint32_t color, const lv_image_dsc_t *icon,
                        int action, const char *genre, const char *art) {
    lv_obj_t *tile=button(parent,NULL,w,h,genre?browse_genre_cb:action_cb,genre?(void *)genre:(void *)(intptr_t)action);
    lv_obj_set_style_bg_color(tile,lv_color_hex(color),0);
    lv_obj_set_style_bg_opa(tile,LV_OPA_COVER,0);
    lv_obj_set_style_radius(tile,8,0); /* no corner clipping: a layer per tile per frame on the R1 */
    lv_obj_set_style_bg_color(tile,lv_color_darken(lv_color_hex(color),LV_OPA_20),LV_STATE_PRESSED);
    lv_obj_t *t=label(tile,title,&font_ui_22_bold,w-28,false);
    lv_label_set_long_mode(t,LV_LABEL_LONG_WRAP);
    lv_obj_set_height(t,LV_SIZE_CONTENT);
    lv_obj_set_style_max_height(t,2*font_ui_22_bold.line_height,0);
    lv_obj_set_style_text_color(t,lv_color_white(),0);lv_obj_set_pos(t,14,12);
    /* The sleeve: tilted 25 degrees, half out of the tile's corner, shadowed. */
    lv_obj_t *im=lv_image_create(tile);
    lv_obj_set_size(im,78,78);
    lv_obj_set_pos(im,w-62,h-64);
    lv_obj_set_style_radius(im,4,0);
    lv_obj_set_style_bg_color(im,lv_color_darken(lv_color_hex(color),LV_OPA_40),0);
    lv_obj_set_style_bg_opa(im,LV_OPA_COVER,0);
    lv_image_set_src(im,icon);
    lv_image_set_inner_align(im,LV_IMAGE_ALIGN_CENTER);
    lv_obj_set_style_image_recolor(im,lv_color_white(),0);
    lv_obj_set_style_image_recolor_opa(im,LV_OPA_COVER,0);
    lv_obj_set_style_shadow_width(im,14,0);
    lv_obj_set_style_shadow_color(im,lv_color_black(),0);
    lv_obj_set_style_shadow_opa(im,LV_OPA_50,0);
    lv_obj_set_style_transform_pivot_x(im,39,0);
    lv_obj_set_style_transform_pivot_y(im,39,0);
    lv_obj_set_style_transform_rotation(im,250,0);
    if (icon == &icon_heart) {
        /* Liked Songs keeps its own cover: the heart on the violet gradient. */
        lv_obj_set_style_bg_color(im,lv_color_hex(0x450af5),0);
        lv_obj_set_style_bg_grad_color(im,lv_color_hex(0xc4efd9),0);
        lv_obj_set_style_bg_grad_dir(im,LV_GRAD_DIR_VER,0);
        lv_image_set_src(im,NULL);
        lv_obj_t *heart=lv_image_create(im);
        lv_image_set_src(heart,&icon_heart);lv_obj_set_size(heart,36,36);
        lv_image_set_inner_align(heart,LV_IMAGE_ALIGN_CONTAIN);
        lv_obj_set_style_image_recolor(heart,lv_color_white(),0);
        lv_obj_set_style_image_recolor_opa(heart,LV_OPA_COVER,0);
        lv_obj_center(heart);
    }
    if (art && art[0] && browse_art_slot < LOCAL_HITS) {
        int k = browse_art_slot++;
        instant_images[k] = im;
        coverloader_request(COVERLOADER_INSTANT_BASE + k, art, 78);
    }
}
static lv_obj_t *browse_heading(const char *title) {
    lv_obj_t *l = label(search_results, title, &font_ui_24_bold, cfg->screen_width - 2 * PAD, false);
    lv_obj_set_style_margin_top(l, 6, 0);
    return l;
}
/* The field has a length limit, and with one LVGL empties it without sending
 * VALUE_CHANGED: the clear button starts the empty search itself, which puts
 * Browse back. */
static void search_clear_cb(lv_event_t *e) { (void)e;lv_textarea_set_text(search_field,"");++search_epoch;start_instant(); }

static void search_browse(void) {
    if (browse_shown && browse_revision == library_revision(LIBRARY_LIST_TRACKS)) return;
    browse_shown = true;
    browse_revision = library_revision(LIBRARY_LIST_TRACKS);
    search_art_drop();
    lv_obj_clean(search_results);
    browse_art_slot = 0;
    if(!browse_busy && (!browse_valid || browse_data.revision!=browse_revision)) {
        memset(&browse_pending,0,sizeof(browse_pending));browse_pending.revision=browse_revision;
        __atomic_store_n(&browse_ready,0,__ATOMIC_RELEASE);
        browse_busy=start_worker(browse_worker);
    }
    browse_genre_count=browse_data.count;
    memcpy(browse_genres,browse_data.genres,sizeof(browse_genres));
    char art[512];
    if (browse_genre_count) {
        /* Your top genres: a carousel; the next tile peeks in at the edge and a
         * sideways drag flicks through, each tile resting at the gutter. */
        browse_heading("Your top genres");
        lv_obj_t *row = box(search_results, cfg->screen_width, 118, false);
        lv_obj_set_style_translate_x(row, -PAD, 0);
        lv_obj_set_style_pad_hor(row, PAD, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(row, 12, 0);
        lv_obj_set_scrollable(row, true);
        lv_obj_set_scroll_dir(row, LV_DIR_HOR);
        lv_obj_set_scroll_snap_x(row, LV_SCROLL_SNAP_START);
        lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
        for (int i = 0; i < browse_genre_count; i++) {
            snprintf(art,sizeof(art),"%s",browse_data.genre_art[i]);
            browse_tile(row, 196, 112, browse_genres[i], TILE_COLORS[i % 12], &icon_genre, 0, browse_genres[i], art);
        }
    }
    browse_heading("Browse all");
    int tw = (cfg->screen_width - 2 * PAD - 12) / 2;
    lv_obj_t *grid = box(search_results, cfg->screen_width - 2 * PAD, LV_SIZE_CONTENT, false);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_column(grid, 12, 0);
    lv_obj_set_style_pad_row(grid, 12, 0);
    static const struct { const char *title; uint32_t color; int action; int sample; } TILES[] = {
        {"All songs", 0x1e3264, OPEN_TRACKS, 0},      {"Liked Songs", 0x8400e7, OPEN_FAVOURITES, -1},
        {"Albums", 0xe8115b, OPEN_ALBUMS, 1},         {"Artists", 0x477d95, OPEN_ARTISTS, 2},
        {"Playlists", 0x006450, OPEN_PLAYLISTS, 3},   {"Genres", 0xba5d07, OPEN_GENRES, 4},
        {"Most played", 0xaf2896, OPEN_MOST_PLAYED, 5}, {"Files & folders", 0x509bf5, OPEN_BROWSE, -1},
        /* An R1 addition for classical libraries: who wrote it. */
        {"Composers", 0x7d4b32, OPEN_COMPOSERS, -1},
    };
    static const lv_image_dsc_t *const ICONS[] = {&icon_music2, &icon_heart, &icon_album, &icon_artist,
                                                  &icon_list_music, &icon_genre, &icon_music2, &icon_folder,
                                                  &icon_music_note};
    for (int i = 0; i < 9; i++) {
        art[0] = 0;
        if (TILES[i].sample >= 0)
            snprintf(art,sizeof(art),"%s",browse_data.tile_art[TILES[i].sample]);
        browse_tile(grid, tw, 112, TILES[i].title, TILES[i].color, ICONS[i], TILES[i].action, NULL, art);
    }
}

static void search_build(void) {
    lv_obj_t *p = page[SEARCH];
    lv_obj_set_layout(p, LV_LAYOUT_NONE);
    lv_obj_set_scrollable(p, false);
    lv_obj_t *search_frame=box(p,cfg->screen_width-2*PAD,58,true);
    /* Spotify's Search bar (iOS, captured 2026-11-04): white, slightly
     * rounded, black glyph and text, "What do you want to listen to?". */
    lv_obj_set_pos(search_frame,0,0);lv_obj_set_style_radius(search_frame,8,0);
    lv_obj_set_style_bg_color(search_frame,lv_color_white(),0);
    lv_obj_set_style_border_width(search_frame,0,0);
    search_field = lv_textarea_create(p);
    /* The text viewport itself excludes both icons. Padding alone lets a
     * horizontally scrolled URL paint underneath the search glyph. */
    lv_obj_set_size(search_field, cfg->screen_width - 2 * PAD - 52, 54);
    lv_obj_set_pos(search_field, 48, 2);
    lv_textarea_set_one_line(search_field, true);
    lv_textarea_set_max_length(search_field, 160);
    lv_textarea_set_placeholder_text(search_field, "What do you want to listen to?");
    lv_obj_add_style(search_field, &theme_style_card, 0);
    lv_obj_add_style(search_field, &theme_style_text, 0);
    lv_obj_set_style_text_font(search_field, &font_ui_20, 0);
    lv_obj_set_style_bg_color(search_field,lv_color_hex(0x292929),0);
    lv_obj_set_style_text_color(search_field,lv_color_black(),0);
    lv_obj_set_style_radius(search_field,0,0);
    lv_obj_set_style_border_width(search_field,0,0);
    lv_obj_set_style_border_width(search_field,0,LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(search_field,0,0);
    lv_obj_set_style_outline_width(search_field,0,LV_STATE_FOCUSED);
    lv_obj_set_scrollbar_mode(search_field,LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(search_field,LV_OPA_TRANSP,0);
    lv_obj_set_style_pad_left(search_field,0,0);lv_obj_set_style_pad_right(search_field,0,0);
    lv_obj_set_style_text_color(search_field,lv_color_hex(0x535353),LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_t *mag=lv_image_create(p);lv_image_set_src(mag,&icon_search);lv_obj_set_size(mag,26,26);
    lv_obj_set_style_image_recolor(mag,lv_color_black(),0);lv_obj_set_style_image_recolor_opa(mag,LV_OPA_COVER,0);
    lv_image_set_inner_align(mag,LV_IMAGE_ALIGN_CONTAIN);lv_obj_set_pos(mag,14,16);
    lv_obj_t *clear=button(p,"",44,52,search_clear_cb,NULL);lv_obj_set_pos(clear,cfg->screen_width-2*PAD-46,3);
    search_clear_btn=clear;
    lv_obj_set_style_bg_opa(clear,0,0);lv_obj_t *x=lv_image_create(clear);lv_image_set_src(x,&icon_close);
    lv_obj_set_style_image_recolor(x,lv_color_black(),0);lv_obj_set_style_image_recolor_opa(x,LV_OPA_COVER,0);
    lv_obj_set_size(x,24,24);lv_image_set_inner_align(x,LV_IMAGE_ALIGN_CONTAIN);lv_obj_center(x);
    keyboard_style_caret(search_field);
    lv_obj_add_event_cb(search_field, field_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(search_field, instant_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
    search_clear_sync();
    lv_obj_t *chips=box(p,cfg->screen_width-2*PAD,50,false);lv_obj_set_pos(chips,0,68);search_chips=chips;
    lv_obj_set_flex_flow(chips,LV_FLEX_FLOW_ROW);lv_obj_set_style_pad_column(chips,8,0);
    lv_obj_set_scrollable(chips,true);lv_obj_set_scroll_dir(chips,LV_DIR_HOR);lv_obj_set_scrollbar_mode(chips,LV_SCROLLBAR_MODE_OFF);
    const char *names[]={"All","Songs","Albums","Artists","Playlists","Lyrics"};
    const int widths[]={58,86,96,94,110,88};
    for(int i=0;i<6;i++) {
        filter_buttons[i]=button(chips,names[i],widths[i],46,filter_cb,(void *)(intptr_t)i);
        lv_obj_set_style_radius(filter_buttons[i],23,0);lv_obj_set_style_bg_color(filter_buttons[i],mint(),0);
        lv_obj_set_style_bg_opa(filter_buttons[i],i?LV_OPA_TRANSP:LV_OPA_COVER,0);
        lv_obj_set_style_border_width(filter_buttons[i],1,0);lv_obj_set_style_border_color(filter_buttons[i],lv_color_hex(0x555555),0);
        lv_obj_set_style_text_color(lv_obj_get_child(filter_buttons[i],0),i?ink():lv_color_black(),0);
    }
    search_status = label(p, "", &font_ui_16, cfg->screen_width - 2 * PAD, true);
    lv_obj_set_pos(search_status, 0, 122);
    search_results = column(p, cfg->screen_width, body_height() - 150);
    lv_obj_set_style_pad_hor(search_results, PAD, 0);
    lv_obj_set_pos(search_results, -PAD, 150);
    search_browse();
    search_keyboard_parent = box(main_menu_screen, cfg->screen_width, 310, false);
    lv_obj_set_pos(search_keyboard_parent, 0, cfg->screen_height - NAV_H - 310);
    search_keyboard = keyboard_create(search_keyboard_parent, cfg->screen_width, 310, search_field,
                                      &icon_search, NULL, submit_cb, NULL);
    search_keyboard_visible(false);
    search_chips_show(false);
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
    if(tab_save_pending && lv_tick_elaps(tab_changed)>1500 && lv_display_get_inactive_time(NULL)>1000) {config_save();tab_save_pending=false;}
    browse_poll();
    search_art_update();
    /* Search and Your Library made ahead, hidden, once the screen has been
     * left alone for a few seconds on the main pages: the first tap on either
     * then only shows it. One at a time, a few seconds apart. */
    static uint32_t prebuilt_at;
    if (lv_screen_active() == main_menu_screen && lv_display_get_inactive_time(NULL) > 3000 &&
        lv_tick_elaps(prebuilt_at) > 4000 && !bottomsheet_active() && !player_sheet_is_open()) {
        int want = !page[SEARCH] ? SEARCH : !page[LIBRARY] ? LIBRARY : -1;
        uint32_t started = lv_tick_get();
        if (want >= 0) {
            page_ensure(want);
            prebuilt_at = lv_tick_get();
            printf("home: %s built ahead in %u ms\n", want == SEARCH ? "Search" : "Your Library",
                   (unsigned)lv_tick_elaps(started));
        } else if (!library_scan_running() && selected != LIBRARY && collectionview_stale()) {
            /* A hidden page brought up to date in the same quiet moment, not
             * on the tap that shows it: after the startup check, refreshing
             * Your Library on the tap took 1.9 s (2026-11-05). */
            collectionview_refresh();
            prebuilt_at = lv_tick_get();
            printf("home: Your Library refreshed ahead in %u ms\n", (unsigned)lv_tick_elaps(started));
        } else if (!library_scan_running() && selected != HOME && home_built && home_revision != home_signature()) {
            home_build();
            prebuilt_at = lv_tick_get();
        }
    }
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
                snprintf(shown_text,sizeof(shown_text),"%s",local_text);shown_type=local_type;
                search_draw();
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
        if (home_revision!=home_signature() && !library_scan_running()
            && !bottomsheet_active() && lv_obj_get_scroll_y(page[HOME])==0
            && lv_display_get_inactive_time(NULL)>2000) home_build();
        home_art_update();
    }
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
    /* The tab already open, tapped again: back to the top of it, as Spotify
     * does -- the way out of a long Your Library. */
    if(lv_screen_active()==main_menu_screen && tab==selected && tab==LIBRARY) {collectionview_scroll_top();return;}
    if(lv_screen_active()!=main_menu_screen) {selected=tab;main_menu_mount_footer(main_menu_screen);switch_screen(main_menu_screen);}
    else show_page(tab);
}

/* A tab's page, built the first time it is wanted -- or earlier, hidden, while
 * nobody is touching the screen (prebuild_tick): building Search took most of
 * a second on the R1 the first time it was opened (2026-11-05). */
static void page_ensure(int tab) {
    if (page[tab]) return;
    /* Full width with the margin inside: card carousels run to the screen
     * edges (Spotify's peeking shelves) while everything else keeps its
     * 20 px gutter. */
    page[tab] = column(main_menu_screen, cfg->screen_width, body_height());
    lv_obj_set_style_pad_hor(page[tab], PAD, 0);
    lv_obj_set_pos(page[tab], 0, cfg->top_bar_height + HEADER_H);
    lv_obj_set_hidden(page[tab], tab != selected);
    if (tab == SEARCH) search_build();
    if (tab == LIBRARY) library_build();
}

static void show_page(int tab) {
    if (tab < HOME || tab > LIBRARY) return;
    selected = tab;
    if(tab<=LIBRARY) {config_set_int("ui","last_tab",tab);tab_save_pending=true;tab_changed=lv_tick_get();}
    /* Keep the bounded Home artwork slots warm across tab switches. */
    if (search_keyboard) search_keyboard_visible(false);
    page_ensure(tab);
    for (int i = 0; i < 3; i++) if (page[i]) lv_obj_set_hidden(page[i], i != tab);
    const char *titles[] = {"Home", "Search", "Your Library"};
    lv_label_set_text(heading, titles[tab]);
    lv_obj_set_hidden(header_add,tab!=LIBRARY);
    lv_obj_set_hidden(header_search,tab!=LIBRARY);lv_obj_set_hidden(header_tools,true);
    lv_obj_set_hidden(header_settings,true);
    for (int i = 0; i < 3; i++) {
        bool active = i == tab;
        lv_obj_set_style_image_recolor(nav_icon[i], active ? ink() : quiet(), 0);
        lv_obj_set_style_text_color(nav_label[i], active ? ink() : quiet(), 0);
    }
    if (tab == HOME) home_build_if_stale();
    /* Only when something it lists changed: rebuilt on every visit, it took
     * over a second on the R1 (2026-11-05). */
    if(tab==LIBRARY) collectionview_refresh_if_stale();
    if(tab==SEARCH && search_keyboard)search_keyboard_visible(false);
}

static void screen_cb(lv_event_t *e) {
    if (lv_event_get_code(e) == LV_EVENT_SCREEN_LOADED) {
        main_menu_mount_footer(main_menu_screen);
        show_page(selected);
        mini_update();
    } else {
        if (search_keyboard) search_keyboard_visible(false);
        mini_drop();
    }
}

void main_menu_init(gui_config_t *config) {
    cfg = config;
    /* All six legacy destinations remain reachable without remapping their
     * callbacks or the hardware transport keys. */
    tools_screen = lv_obj_create(NULL);
    const grid_entry_t entries[] = {
        {"music", &icon_menu_music, &music_screen},
        {"wireless", &icon_menu_wireless, &wireless_screen},
        {"audiobooks", &icon_menu_audiobooks, &audiobooks_screen},
        {"more", &icon_menu_more, &morepage_screen},
        {"settings", &icon_menu_settings, &settings_screen},
    };
    gridpage_build(tools_screen, cfg, entries, 5, 2, 3, true);
    switcher_attach_back_gesture(tools_screen);
    lv_obj_add_style(main_menu_screen, &theme_style_screen, 0);
    lv_obj_set_style_pad_all(main_menu_screen, 0, 0);
    lv_obj_set_scrollable(main_menu_screen, false);
    if(theme_is_dark()) {lv_obj_set_style_bg_color(main_menu_screen,lv_color_hex(0x121212),0);topbar_tint_on_screen(main_menu_screen,true,lv_color_hex(0x121212),lv_color_white());}
    header_profile = button(main_menu_screen, NULL, 44, 44, device_menu_cb, NULL);
    lv_obj_set_style_radius(header_profile, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_ext_click_area(header_profile, 4);
    lv_obj_set_pos(header_profile, PAD - 4, cfg->top_bar_height + 12);
    avatar(header_profile, 44, &font_ui_20_bold);
    heading = label(main_menu_screen, "Home", &font_ui_28, cfg->screen_width - 200, false);
    lv_obj_set_pos(heading, PAD + 52, cfg->top_bar_height + 17);
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
    player_sheet_attach_drag(mini, true); /* swipe it up, as on Spotify */
    player_sheet_set_handle(mini);
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
