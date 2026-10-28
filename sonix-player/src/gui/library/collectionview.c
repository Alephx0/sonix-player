#include "src/gui/shell/bottomsheet.h"
#include "src/gui/nowplaying/player.h"
#include "collectionview.h"
#include "src/gui/shell/main_menu.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/library/medialist.h"
#include "src/gui/library/playlistpage.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/keyboard.h"
#include "src/gui/shell/popover.h"
#include "src/gui/shell/reference_icons.h"
#include "src/gui/shell/switcher.h"
#include "src/gui/shell/theme.h"
#include "src/gui/streaming/podcastpage.h"
#include "src/system/core/config.h"
#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/playback/playlist.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static gui_config_t *cfg;
static lv_obj_t *row_images[24], *row_boxes[24];
static struct {
	int row;
	cover_image_t image;
	bool loaded;
} art[8];
static void art_drop(void) {
	for (int i = 0; i < 8; i++) {
		coverloader_release(COVERLOADER_COLLECTION_BASE + i);
		if (art[i].loaded) {
			if (art[i].row >= 0 && art[i].row < 24 && row_images[art[i].row])
				lv_image_set_src(row_images[art[i].row], &icon_album);
			cover_free(&art[i].image);
		}
		art[i].loaded = false;
		art[i].row = -1;
	}
}
static bool sample_track(const char *name, const char *path, const char *artist, void *user) {
	(void)name;
	(void)artist;
	if (path && !access(path, R_OK)) {
		snprintf(user, 512, "%s", path);
		return false;
	}
	return true;
}
static void art_update(void);

static lv_obj_t *root, *list, *sheet, *sheet_body, *name_layer, *name_field;
static int filter, sort_mode, page;
static bool grid;
static collection_item_t rows[24], menu_item;
static int row_count;
static char track[512], track_title[256], track_artist[256];
static int member_page;
static lv_obj_t *member_list, *member_search;
static keyboard_t *member_keyboard;
static char member_query[201];
static bool name_returns;
static lv_obj_t *member_art[8];
static cover_image_t member_covers[8];
static int member_art_count;
static void member_art_drop(void) {
    for(int i=0;i<8;i++) {
        coverloader_release(COVERLOADER_MEMBERS_BASE+i);
        if(member_art[i]) lv_image_set_src(member_art[i],&icon_files_playlist);
        cover_free(&member_covers[i]);member_art[i]=NULL;
    }
    member_art_count=0;
}
static void member_art_tick(lv_timer_t *t) {
    (void)t;
    for(int i=0;i<member_art_count;i++) if(member_art[i] && !member_covers[i].pixels) {
        bool done=false;if(coverloader_take(COVERLOADER_MEMBERS_BASE+i,&member_covers[i],&done)) lv_image_set_src(member_art[i],&member_covers[i].dsc);
    }
}
static char member_names[24][201];
static int member_count, member_seen;
static unsigned seen_revision, seen_playlists;
static lv_timer_t *refresh_timer;
static lv_obj_t *text(lv_obj_t *p, const char *value, int width, const lv_font_t *font, bool dim) {
	lv_obj_t *o = lv_label_create(p);
	lv_obj_set_width(o, width);
	lv_label_set_text(o, value);
	lv_label_set_long_mode(o, LV_LABEL_LONG_DOT);
	lv_obj_add_style(o, dim ? &theme_style_text_dim : &theme_style_text, 0);
	lv_obj_set_style_text_font(o, font, 0);
	return o;
}
static lv_obj_t *panel(lv_obj_t *p, int w, int h) {
	lv_obj_t *o = lv_obj_create(p);
	lv_obj_remove_style_all(o);
	lv_obj_add_style(o, &theme_style_screen, 0);
	lv_obj_set_size(o, w, h);
	lv_obj_set_style_pad_all(o, 0, 0);
	lv_obj_set_scrollable(o, false);
	if (theme_is_dark())
		lv_obj_set_style_bg_color(o, lv_color_hex(0x121212), 0);
	return o;
}
static lv_obj_t *button(lv_obj_t *p, const char *s, int w, int h, lv_event_cb_t cb, void *u) {
	lv_obj_t *o = panel(p, w, h);
	lv_obj_add_style(o, &theme_style_card, 0);
	lv_obj_set_style_radius(o, 8, 0);
	lv_obj_set_clickable(o, true);
	if (s) {
		lv_obj_t *l = text(o, s, w - 16, &font_ui_18, false);
		lv_obj_center(l);
	}
	lv_obj_add_event_cb(o, cb, LV_EVENT_CLICKED, u);
	return o;
}
static void sheet_cleanup(void) {
    member_art_drop();
    sheet=sheet_body=name_layer=name_field=member_list=member_search=NULL;member_keyboard=NULL;
    player_refresh_membership();collectionview_refresh();
}
static void close_sheet(lv_event_t *e) { (void)e; bottomsheet_close(true); }
static void sheet_new(const char *title) {
    int w=lv_display_get_horizontal_resolution(NULL), h=lv_display_get_vertical_resolution(NULL);
    sheet=bottomsheet_open(h-60,sheet_cleanup);
    lv_obj_t *l=text(sheet,title,w-122,&font_ui_24_bold,false);lv_obj_set_pos(l,20,12);
    lv_obj_t *done=button(sheet,"Done",76,46,close_sheet,NULL);lv_obj_set_pos(done,w-92,0);
    sheet_body=panel(sheet,w-32,h-160);lv_obj_set_pos(sheet_body,16,60);
    lv_obj_set_style_bg_opa(sheet_body,0,0);lv_obj_set_scrollable(sheet_body,true);
    lv_obj_set_scroll_dir(sheet_body,LV_DIR_VER);lv_obj_set_flex_flow(sheet_body,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(sheet_body,10,0);
}
static void open_row(lv_event_t *e) {
	collection_item_t *r = lv_event_get_user_data(e);
	collections_touch(r->kind, r->key);
	if (r->kind == COLLECTION_PLAYLIST)
		medialist_open(r->title, LIBRARY_LIST_PLAYLIST, LIBRARY_FILTER_NONE, r->key);
	else if (r->kind == COLLECTION_ALBUM)
		medialist_open(r->title, LIBRARY_LIST_TRACKS, LIBRARY_FILTER_ALBUM, r->key);
	else if (r->kind == COLLECTION_ARTIST)
		medialist_open_artist(r->key);
	else
		podcastpage_open_feed(strtoll(r->key, NULL, 10), r->title);
}
static void save_action(lv_event_t *e) {
	(void)e;
	bool saved = !(collections_get(menu_item.kind, menu_item.key, &menu_item) && menu_item.saved);
	if (!collections_put(menu_item.kind, menu_item.key, menu_item.title, menu_item.creator, saved))
		gui_notify_popup("Could not save this collection.");
	close_sheet(NULL);
}
static void pin_action(lv_event_t *e) {
	(void)e;
	collection_item_t current = {0};
	bool pinned = collections_get(menu_item.kind, menu_item.key, &current) && current.pin;
	if (!pinned && !current.saved)
		collections_put(menu_item.kind, menu_item.key, menu_item.title, menu_item.creator, true);
	if (!collections_pin(menu_item.kind, menu_item.key, !pinned))
		gui_notify_popup("You can pin up to four collections.");
	close_sheet(NULL);
}
void collectionview_collection_options(collection_kind_t kind, const char *key, const char *title, const char *creator) {
	if (!key || !*key)
		return;
	collections_init(playlist_card_root());
	memset(&menu_item, 0, sizeof(menu_item));
	menu_item.kind = kind;
	snprintf(menu_item.key, sizeof(menu_item.key), "%s", key);
	snprintf(menu_item.title, sizeof(menu_item.title), "%s", title ? title : key);
	snprintf(menu_item.creator, sizeof(menu_item.creator), "%s", creator ? creator : "");
	collection_item_t saved = {0};
	collections_get(kind, key, &saved);
	sheet_new(title ? title : key);
	int w = lv_display_get_horizontal_resolution(NULL) - 32;
	button(sheet_body, kind == COLLECTION_ARTIST ? (saved.saved ? "Unfollow artist" : "Follow artist") : (saved.saved ? "Remove from Your Library" : "Save to Your Library"), w, 58, save_action, NULL);
	button(sheet_body, saved.pin ? "Unpin" : "Pin to top", w, 58, pin_action, NULL);
	text(sheet_body, "Your music files stay on the card.", w, &font_ui_16, true);
}
static void row_options(lv_event_t *e) {
	collection_item_t *r = lv_event_get_user_data(e);
	collectionview_collection_options(r->kind, r->key, r->title, r->creator);
}
static void route(lv_event_t *e) {
	switch ((intptr_t)lv_event_get_user_data(e)) {
    case 7: main_menu_open_downloads(); break;
	case 1:
		medialist_open("Liked Songs", LIBRARY_LIST_FAVOURITES, LIBRARY_FILTER_NONE, NULL);
		break;
	case 2:
		medialist_open("All albums", LIBRARY_LIST_ALBUMS, LIBRARY_FILTER_NONE, NULL);
		break;
	case 3:
		medialist_open("All artists", LIBRARY_LIST_ARTISTS, LIBRARY_FILTER_NONE, NULL);
		break;
	case 4:
		podcastpage_open_followed();
		break;
	case 5:
		playlistpage_open();
		break;
	case 6:
		medialist_open("Most played", LIBRARY_LIST_MOST_PLAYED, LIBRARY_FILTER_NONE, NULL);
		break;
	}
}
static bool register_playlist(const char *name, void *user) {
	(void)user;
	collection_item_t existing;
	if (!collections_get(COLLECTION_PLAYLIST, name, &existing))
		collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
	return true;
}
static void page_cb(lv_event_t *e) {
	page += (int)(intptr_t)lv_event_get_user_data(e);
	if (page < 0)
		page = 0;
	collectionview_refresh();
}
static void filters_cb(lv_event_t *e) {
	filter = (int)(intptr_t)lv_event_get_user_data(e);
	page = 0;
	config_set_int("collections", "filter", filter);
	config_save();
	collectionview_refresh();
}
static void sort_cb(lv_event_t *e) {
	(void)e;
	sort_mode = (sort_mode + 1) % 4;
	page = 0;
	config_set_int("collections", "sort", sort_mode);
	config_save();
	collectionview_refresh();
}
static void grid_cb(lv_event_t *e) {
	(void)e;
	grid = !grid;
	config_set_int("collections", "grid", grid);
	config_save();
	collectionview_refresh();
}
static void members_draw(void);
static void name_close(lv_event_t *e) {
	(void)e;
	if (name_layer) {
		lv_obj_delete(name_layer);
		name_layer = NULL;
	}
    if(!name_returns) bottomsheet_close(true);
}
static void name_accept(lv_event_t *e) {
	(void)e;
	const char *name = lv_textarea_get_text(name_field);
	if (!strcasecmp(name,"Liked Songs") || !playlists_create(name)) {
		gui_notify_popup("Choose a new, valid playlist name.");
		return;
	}
	bool ok = !track[0] || playlists_add_track(name, track);
	collections_put(COLLECTION_PLAYLIST, name, name, "You", true);
    if(name_returns && member_search) {
        snprintf(member_query,sizeof(member_query),"%s",name);member_page=0;
        lv_textarea_set_text(member_search,member_query);
    }
	name_close(NULL);
	if (!ok)
		gui_notify_popup("Playlist created, but the track could not be added.");
	if (sheet)
		members_draw();
	collectionview_refresh();
}
static void create_cb(lv_event_t *e) {
	bool membership = lv_event_get_user_data(e) != NULL;
    name_returns=membership;
	if (!membership)
		track[0] = 0;
	int w = lv_display_get_horizontal_resolution(NULL), h = lv_display_get_vertical_resolution(NULL);
	if(!sheet) sheet_new("New playlist");
    h=lv_obj_get_height(sheet);
    name_layer = panel(sheet, w, h);
    lv_obj_move_foreground(name_layer);
	lv_obj_set_clickable(name_layer, true);
	lv_obj_t *l = text(name_layer, "New playlist", w - 40, &font_ui_24_bold, false);
	lv_obj_set_pos(l, 20, 60);
	name_field = lv_textarea_create(name_layer);
	lv_obj_add_style(name_field, &theme_style_card, 0);
	lv_obj_add_style(name_field, &theme_style_text, 0);
	lv_obj_set_style_text_font(name_field, &font_ui_24, 0);
	lv_obj_set_size(name_field, w - 40, 64);
	lv_obj_set_pos(name_field, 20, 120);
	lv_textarea_set_one_line(name_field, true);
	lv_textarea_set_max_length(name_field, 200);
	lv_textarea_set_placeholder_text(name_field, "Playlist name");
	keyboard_style_caret(name_field);
	lv_obj_t *cancel = button(name_layer, "Cancel", 100, 48, name_close, NULL);
	lv_obj_set_pos(cancel, 20, 205);
	keyboard_create(name_layer, w, 300, name_field, NULL, "Create", name_accept, NULL);
}
void collectionview_create_playlist(lv_event_t *e) { create_cb(e); }
static bool member_collect(const char *name, void *user) {
	(void)user;
	char folded[512], query[512];
    library_search_fold(name,folded,sizeof(folded));library_search_fold(member_query,query,sizeof(query));
    if(query[0] && !strstr(folded,query)) return true;
    if (member_seen++ < member_page * 24)
		return true;
	if (member_count == 24)
		return false;
	snprintf(member_names[member_count++], 201, "%s", name);
	return true;
}
static void member_toggle(lv_event_t *e) {
	const char *name = lv_event_get_user_data(e);
	bool exists = playlists_has_track(name, track), ok = true;
	if (exists)
		ok = library_playlist_remove_all_path(name, track);
	else
		ok = playlists_add_track(name, track);
	if (!ok)
		gui_notify_popup("Could not update playlist membership.");
	members_draw();
}
static void liked_cb(lv_event_t *e) {
	(void)e;
	library_fav_toggle(track, track_title, track_artist);
	members_draw();
}
static void member_page_cb(lv_event_t *e) {
	member_page += (int)(intptr_t)lv_event_get_user_data(e);
	if (member_page < 0)
		member_page = 0;
	members_draw();
}
static lv_obj_t *member_row(const char *name, bool selected, lv_event_cb_t cb, void *u) {
    int w=lv_display_get_horizontal_resolution(NULL)-32;
    lv_obj_t *row=button(member_list,NULL,w,74,cb,u);
    lv_obj_set_style_bg_opa(row,selected?LV_OPA_20:LV_OPA_TRANSP,0);
    lv_obj_t *art=lv_image_create(row);lv_image_set_src(art,!strcmp(name,"Liked Songs")?&icon_heart:&icon_files_playlist);
    lv_obj_set_size(art,48,48);lv_image_set_inner_align(art,LV_IMAGE_ALIGN_CONTAIN);lv_obj_set_pos(art,4,12);
    if(!strcmp(name,"Liked Songs")) {
        lv_obj_set_style_bg_color(art,lv_color_hex(0x6432e8),0);lv_obj_set_style_bg_grad_color(art,lv_color_hex(0xa8dbca),0);
        lv_obj_set_style_bg_grad_dir(art,LV_GRAD_DIR_VER,0);lv_obj_set_style_bg_opa(art,255,0);lv_obj_set_style_radius(art,5,0);
    } else if(member_art_count<8) {
        library_playlist_row_t first;
        if(library_playlist_page(name,0,1,&first)==1) {
            int slot=member_art_count++;member_art[slot]=art;
            coverloader_request(COVERLOADER_MEMBERS_BASE+slot,first.path,48);
        }
    }
    lv_obj_t *l=text(row,name,w-122,&font_ui_22,false);lv_obj_set_pos(l,68,12);
    l=text(row,"Playlist",w-122,&font_ui_16,true);lv_obj_set_pos(l,68,42);
    lv_obj_t *check=lv_image_create(row);lv_image_set_src(check,selected?&icon_circle_check:&icon_plus);
    lv_obj_set_size(check,30,30);lv_image_set_inner_align(check,LV_IMAGE_ALIGN_CONTAIN);
    lv_obj_align(check,LV_ALIGN_RIGHT_MID,-4,0);lv_obj_set_style_image_recolor(check,lv_color_hex(selected?0x1ed760:0xb3b3b3),0);
    lv_obj_set_style_image_recolor_opa(check,255,0);return row;
}
static void members_draw(void) {
    if (!member_list) return;
    member_art_drop();
    lv_obj_clean(member_list);
    int w=lv_display_get_horizontal_resolution(NULL)-32;
    char query[512];library_search_fold(member_query,query,sizeof(query));
    bool liked_visible=!query[0] || strstr("liked songs",query), liked=library_fav_contains(track);
    member_count=member_seen=0;playlists_for_each(member_collect,NULL);
    for(int group=1;group>=0;group--) {
        bool heading=false;
        if(liked_visible && liked==group) {text(member_list,group?"Saved in":"Playlists",w,&font_ui_18,true);heading=true;member_row("Liked Songs",liked,liked_cb,NULL);}
        for(int i=0;i<member_count;i++) if(playlists_has_track(member_names[i],track)==group) {
            if(!heading) {text(member_list,group?"Saved in":"Playlists",w,&font_ui_18,true);heading=true;}
            member_row(member_names[i],group,member_toggle,member_names[i]);
        }
    }
    static lv_timer_t *art_timer;if(!art_timer) art_timer=lv_timer_create(member_art_tick,120,NULL);
    if(!member_count && !liked_visible && member_query[0]) text(member_list,"No matching playlists",w,&font_ui_18,true);
    if(member_page) button(member_list,"Previous playlists",w,48,member_page_cb,(void *)(intptr_t)-1);
    if(member_count==24) button(member_list,"More playlists",w,48,member_page_cb,(void *)(intptr_t)1);
    player_refresh_membership();
}
static void member_search_changed(lv_event_t *e) {
    (void)e;snprintf(member_query,sizeof(member_query),"%s",lv_textarea_get_text(member_search));member_page=0;members_draw();
}
static void member_search_done(lv_event_t *e) {
    (void)e;keyboard_set_visible(member_keyboard,false);lv_obj_set_height(member_list,lv_obj_get_height(sheet_body)-124);
}
static void member_search_focus(lv_event_t *e) {
    (void)e;
    if(!member_keyboard) member_keyboard=keyboard_create(sheet,lv_obj_get_width(sheet),270,member_search,NULL,"Done",member_search_done,NULL);
    keyboard_set_visible(member_keyboard,true);lv_obj_set_height(member_list,LV_MAX(80,lv_obj_get_height(sheet_body)-394));
}
typedef struct {const char *path;bool found;} membership_t;
static bool any_member(const char *name,void *u) {
    membership_t *m=u;
    m->found=playlists_has_track(name,m->path);return !m->found;
}
bool collectionview_track_saved(const char *path) {
    if(library_fav_contains(path)) return true;
    membership_t m={path,false};
    playlists_for_each(any_member,&m);return m.found;
}
void collectionview_manage_track(const char *path) {
	if (!path || !*path || !gui_card_available())
		return;
	snprintf(track, sizeof(track), "%s", path);
	track_title[0] = track_artist[0] = 0;
	playlists_track_names(path, track_title, sizeof(track_title), track_artist, sizeof(track_artist));
	member_page = 0;
	collections_init(playlist_card_root());
	sheet_new("Add to playlist");
    member_query[0]=0;
    int w=lv_display_get_horizontal_resolution(NULL)-32;
    button(sheet_body,"+ New playlist",w,52,create_cb,(void *)1);
    member_search=lv_textarea_create(sheet_body);lv_obj_set_size(member_search,w,54);
    lv_textarea_set_one_line(member_search,true);lv_textarea_set_placeholder_text(member_search,"Find a playlist");
    lv_obj_set_style_text_font(member_search,&font_ui_20,0);lv_textarea_set_max_length(member_search,200);
    lv_obj_add_style(member_search,&theme_style_card,0);lv_obj_add_style(member_search,&theme_style_text,0);
    lv_obj_add_event_cb(member_search,member_search_changed,LV_EVENT_VALUE_CHANGED,NULL);
    lv_obj_add_event_cb(member_search,member_search_focus,LV_EVENT_CLICKED,NULL);
    member_list=panel(sheet_body,w,lv_obj_get_height(sheet_body)-124);lv_obj_set_style_bg_opa(member_list,0,0);
    lv_obj_set_scrollable(member_list,true);lv_obj_set_scroll_dir(member_list,LV_DIR_VER);
    lv_obj_set_flex_flow(member_list,LV_FLEX_FLOW_COLUMN);lv_obj_set_style_pad_row(member_list,4,0);
    members_draw();
}
static lv_obj_t *collection_shortcut(const char *title, const char *subtitle, const lv_image_dsc_t *icon, uint32_t color, int action, int width) {
	lv_obj_t *row = button(list, NULL, width, 76, route, (void *)(intptr_t)action);
	lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
	lv_obj_t *art = panel(row, 64, 64);
	lv_obj_set_pos(art, 0, 6);
	lv_obj_set_style_bg_color(art, lv_color_hex(color), 0);
	lv_obj_set_style_bg_grad_color(art, lv_color_lighten(lv_color_hex(color), LV_OPA_50), 0);
	lv_obj_set_style_bg_grad_dir(art, LV_GRAD_DIR_VER, 0);
	lv_obj_t *im = lv_image_create(art);
	lv_image_set_src(im, icon);
	lv_obj_set_size(im, 30, 30);
	lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_center(im);
	lv_obj_set_style_image_recolor(im, lv_color_white(), 0);
	lv_obj_set_style_image_recolor_opa(im, LV_OPA_COVER, 0);
	lv_obj_t *l = text(row, title, width - 98, &font_ui_22, false);
	lv_obj_set_pos(l, 82, 12);
	l = text(row, subtitle, width - 98, &font_ui_16, true);
	lv_obj_set_pos(l, 82, 43);
	return row;
}
void collectionview_refresh(void) {
	if (!root || !cfg)
		return;
	collections_init(cfg->sd_root_path);
	playlists_for_each(register_playlist, NULL);
	art_drop();
	memset(row_images, 0, sizeof(row_images));
	memset(row_boxes, 0, sizeof(row_boxes));
	lv_obj_clean(root);
	int w = lv_obj_get_width(root), h = lv_obj_get_height(root);
	const char *names[] = {"All", "Playlists", "Albums", "Artists", "Podcasts"};
	lv_obj_t *filters = panel(root, w, 44);
	lv_obj_set_flex_flow(filters, LV_FLEX_FLOW_ROW);
	lv_obj_set_scrollable(filters, true);
	lv_obj_set_scroll_dir(filters, LV_DIR_HOR);
	lv_obj_set_style_pad_column(filters, 8, 0);
	for (int i = 0; i < 5; i++) {
		lv_obj_t *b = button(filters, names[i], i ? 104 : 52, 42, filters_cb, (void *)(intptr_t)i);
		lv_obj_set_style_radius(b, 24, 0);
		lv_obj_set_style_bg_opa(b, filter == i ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
		lv_obj_set_style_bg_color(b, lv_color_hex(0x242424), 0);
		lv_obj_set_style_border_width(b, 1, 0);
		lv_obj_set_style_border_color(b, lv_color_hex(filter == i ? 0xaaaaaa : 0x555555), 0);
	}
	static const char *sorts[] = {"Recently played", "Recently added", "Alphabetical", "Creator"};
	lv_obj_t *b = button(root, sorts[sort_mode], 210, 42, sort_cb, NULL);
	lv_obj_set_pos(b, 0, 54);
	lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);
	lv_obj_set_style_text_align(lv_obj_get_child(b, 0), LV_TEXT_ALIGN_LEFT, 0);
	b = button(root, NULL, 60, 42, grid_cb, NULL);
	lv_obj_t *layout_icon = lv_image_create(b);
	lv_image_set_src(layout_icon, grid ? &sonix_icon_library : &sonix_icon_grid);
	lv_obj_set_size(layout_icon, 22, 22);
	lv_image_set_inner_align(layout_icon, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_center(layout_icon);
	lv_obj_set_pos(b, w - 60, 54);
	lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);

	list = panel(root, w, h - 108);
	lv_obj_set_pos(list, 0, 108);
	lv_obj_set_scrollable(list, true);
	lv_obj_set_scroll_dir(list, LV_DIR_VER);
	lv_obj_set_flex_flow(list, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_pad_row(list, 4, 0);
	lv_obj_set_style_pad_column(list, 10, 0);

	if (page == 0 && (filter == 0 || filter == 1))
		collection_shortcut("Liked Songs", "Playlist / Your favorite tracks", &icon_heart, 0x5330aa, 1, w);
	if (page == 0 && filter == 4)
		collection_shortcut("Your episodes", "Podcasts & followed shows", &icon_podcast_list, 0x22694e, 4, w);
	row_count = collections_list(filter, (collection_sort_t)sort_mode, page * 24, rows, 24);
	for (int i = 0; i < row_count; i++) {
		collection_item_t *r = &rows[i];
		if (r->kind == COLLECTION_PLAYLIST && !playlists_exists(r->key))
			continue;
		int rw = grid ? (w - 10) / 2 : w, rh = grid ? 180 : 78;
		lv_obj_t *row = button(list, NULL, rw, rh, open_row, r);
		row_boxes[i] = row;
		lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
		row_images[i] = lv_image_create(row);
		lv_image_set_src(row_images[i], r->kind == COLLECTION_ARTIST ? &icon_artist : &icon_album);
		lv_obj_set_size(row_images[i], 64, 64);
		lv_image_set_inner_align(row_images[i], LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_pos(row_images[i], grid ? 12 : 0, 7);
		if (r->kind == COLLECTION_ARTIST) {
			lv_obj_set_style_bg_color(row_images[i], lv_color_hex(0x383838), 0);
			lv_obj_set_style_bg_opa(row_images[i], LV_OPA_COVER, 0);
			lv_obj_set_style_radius(row_images[i], LV_RADIUS_CIRCLE, 0);
		}
		char title[280];
		snprintf(title, sizeof(title), "%s", r->title);
		lv_obj_t *l = text(row, title, grid ? rw - 24 : rw - 132, &font_ui_22, false);
		lv_obj_set_pos(l, grid ? 12 : 82, grid ? 92 : 12);
		const char *kind = r->kind == COLLECTION_PLAYLIST ? "Playlist" : r->kind == COLLECTION_ALBUM ? "Album" : r->kind == COLLECTION_ARTIST ? "Artist" : "Podcast";
		char sub[300];
		snprintf(sub, sizeof(sub), "%s%s%s", kind, r->creator[0] ? " / " : "", r->creator);
		l = text(row, sub, grid ? rw - 24 : rw - 98, &font_ui_16, true);
		lv_obj_set_pos(l, grid ? 12 : 82, grid ? 130 : 43);
		if (r->pin) {
			lv_obj_set_x(l, grid ? 34 : 104);
			lv_obj_set_width(l, grid ? rw - 48 : rw - 120);
			lv_obj_t *pin = lv_image_create(row);
			lv_image_set_src(pin, &sonix_icon_pin);
			lv_obj_set_size(pin, 16, 16);
			lv_image_set_inner_align(pin, LV_IMAGE_ALIGN_CONTAIN);
			lv_obj_set_pos(pin, grid ? 12 : 82, grid ? 131 : 44);
			lv_obj_set_style_image_recolor(pin, lv_color_hex(0x1ed760), 0);
			lv_obj_set_style_image_recolor_opa(pin, LV_OPA_COVER, 0);
		}
		b = button(row, "...", 42, 40, row_options, r);
		lv_obj_set_pos(b, rw - 46, 5);
		lv_obj_set_style_bg_opa(b, LV_OPA_TRANSP, 0);
	}
	if (filter == 0) {
		collection_shortcut("On repeat", "Your most played songs", &icon_repeat_all, 0x8c3b69, 6, w);
        collection_shortcut("Downloads", "Transfers and local files", &icon_download, 0x335e78, 7, w);
	}
	if (!row_count)
		text(list, "Save albums or follow artists from song options. Your files remain available below.", w, &font_ui_18, true);
	if (page)
		button(list, "Previous", (w - 10) / 2, 48, page_cb, (void *)(intptr_t)-1);
	if (row_count == 24)
		button(list, "More", (w - 10) / 2, 48, page_cb, (void *)(intptr_t)1);
	if (filter == 0 || filter == 1)
		button(list, "Manage playlists", w, 54, route, (void *)5);
	if (filter == 0 || filter == 2)
		button(list, "All albums", w, 54, route, (void *)2);
	if (filter == 0 || filter == 3)
		button(list, "All artists", w, 54, route, (void *)3);
	if (filter == 0 || filter == 4)
		button(list, "Podcasts & shows", w, 54, route, (void *)4);
	seen_revision = collections_revision();
	seen_playlists = library_revision(LIBRARY_LIST_PLAYLIST);
}
static void art_update(void) {
	if (!root || !lv_obj_is_visible(root)) {
		art_drop();
		return;
	}
	lv_area_t viewport;
	lv_obj_get_coords(list, &viewport);
	for (int slot = 0; slot < 8; slot++) {
		int row = art[slot].row;
		if (row < 0 || !row_boxes[row])
			continue;
		lv_area_t box;
		lv_obj_get_coords(row_boxes[row], &box);
		if (box.y2 < viewport.y1 || box.y1 > viewport.y2) {
			coverloader_release(COVERLOADER_COLLECTION_BASE + slot);
			if (art[slot].loaded) {
				lv_image_set_src(row_images[row], &icon_album);
				cover_free(&art[slot].image);
			}
			art[slot].row = -1;
			art[slot].loaded = false;
		} else if (!art[slot].loaded) {
			bool found = false;
			if (coverloader_take(COVERLOADER_COLLECTION_BASE + slot, &art[slot].image, &found) && found) {
				art[slot].loaded = true;
				lv_image_set_src(row_images[row], &art[slot].image.dsc);
			}
		}
	}
	for (int row = 0; row < row_count; row++) {
		if (!row_boxes[row] || rows[row].kind == COLLECTION_ARTIST || rows[row].kind == COLLECTION_PODCAST)
			continue;
		lv_area_t box;
		lv_obj_get_coords(row_boxes[row], &box);
		if (box.y2 < viewport.y1 || box.y1 > viewport.y2)
			continue;
		int empty = -1;
		bool held = false;
		for (int i = 0; i < 8; i++) {
			if (art[i].row == row)
				held = true;
			else if (art[i].row < 0)
				empty = i;
		}
		if (held || empty < 0)
			continue;
		char path[512] = "";
		library_for_each(rows[row].kind == COLLECTION_ALBUM ? LIBRARY_LIST_TRACKS : LIBRARY_LIST_PLAYLIST, rows[row].kind == COLLECTION_ALBUM ? LIBRARY_FILTER_ALBUM : LIBRARY_FILTER_NONE, rows[row].key, sample_track, path);
		art[empty].row = row;
		if (path[0])
			coverloader_request(COVERLOADER_COLLECTION_BASE + empty, path, 64);
	}
}
static void refresh_cb(lv_timer_t *t) {
	(void)t;
	art_update();
	if (root && lv_obj_is_visible(root) && (seen_revision != collections_revision() || seen_playlists != library_revision(LIBRARY_LIST_PLAYLIST)) && !sheet)
		collectionview_refresh();
}
static void root_deleted(lv_event_t *e) {
	(void)e;
	art_drop();
	root = NULL;
	if (refresh_timer) {
		lv_timer_delete(refresh_timer);
		refresh_timer = NULL;
	}
}
void collectionview_build(lv_obj_t *parent, gui_config_t *config) {
	root = parent;
	cfg = config;
	filter = (int)config_get_int("collections", "filter", 0);
	if (filter < 0 || filter > 4)
		filter = 0;
	sort_mode = (int)config_get_int("collections", "sort", 0);
	if (sort_mode < 0 || sort_mode > 3)
		sort_mode = 0;
	grid = config_get_int("collections", "grid", 0) != 0;
	lv_obj_add_event_cb(root, root_deleted, LV_EVENT_DELETE, NULL);
	collectionview_refresh();
	if (!refresh_timer)
		refresh_timer = lv_timer_create(refresh_cb, 750, NULL);
}
