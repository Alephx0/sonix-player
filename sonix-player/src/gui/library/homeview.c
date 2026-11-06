#include "homeview.h"
#include "audiobooks.h"
#include "src/gui/shell/switcher.h"
#include "playlistview.h"
#include "collectionview.h"
#include "medialist.h"
#include "src/gui/fonts/fonts.h"
#include "src/gui/nowplaying/coverloader.h"
#include "src/gui/nowplaying/player.h"
#include "src/gui/shell/bottomsheet.h"
#include "src/gui/shell/icons.h"
#include "src/gui/shell/reference_icons.h"
#include "src/gui/shell/theme.h"
#include "src/system/core/config.h"
#include "src/system/library/collections.h"
#include "src/system/library/library.h"
#include "src/system/library/audiobookdb.h"
#include "src/system/library/playlists.h"
#include "src/system/playback/device_state.h"
#include "src/system/playback/playlist.h"
#include <pthread.h>
#include "src/system/core/utils.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#define HOME_MAX 60
#define HOME_ART 14 // coverloader.h: the last 14 slots
#define HOME_SLOT 182
typedef struct {
	library_home_context_t context;
	collection_kind_t collection;
	char creator[256];
	lv_obj_t *art, *card;
	bool resume, audiobook;
    char progress[64], book_key[512];
	int art_size;
} item_t;
static item_t items[HOME_MAX], menu;
static int item_count, scope;
static int book_page;
static bool book_scan_pending;
static void rebuild(void);
static void book_details(item_t *i);
static bool menu_open;
static lv_obj_t *root;
static gui_config_t *config;
static struct {
	int item;
	bool requested, settled;
	cover_image_t cover;
} art[HOME_ART];
static const char *sections[] = {"Quick access", "Jump back in", "Recently added", "On repeat", "Made for you", "Rediscover"};
static int order[6];
static lv_obj_t *text(lv_obj_t *p, const char *s, int width, const lv_font_t *font, bool dim) {
	lv_obj_t *l = lv_label_create(p);
	lv_label_set_text(l, s);
	lv_obj_set_width(l, width);
	lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
	lv_obj_set_style_text_font(l, font, 0);
	lv_obj_set_style_text_color(l, lv_color_hex(dim ? 0xb3b3b3 : 0xffffff), 0);
	return l;
}
static lv_obj_t *box(lv_obj_t *p, int w, int h) {
	lv_obj_t *o = lv_obj_create(p);
	lv_obj_remove_style_all(o);
	lv_obj_set_size(o, w, h);
	lv_obj_set_scrollable(o, false);
	return o;
}
static lv_obj_t *button(lv_obj_t *p, const char *s, int w, int h, lv_event_cb_t cb, void *u) {
	lv_obj_t *o = box(p, w, h);
	lv_obj_set_clickable(o, true);
	lv_obj_add_event_cb(o, cb, LV_EVENT_CLICKED, u);
	if (s) {
		lv_obj_t *l = text(o, s, w - 12, &font_ui_18, false);
		lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_center(l);
	}
	return o;
}
static void icon(lv_obj_t *b, const lv_image_dsc_t *glyph, int size, uint32_t color) {
	lv_obj_t *im = lv_image_create(b);
	lv_image_set_src(im, glyph);
	lv_obj_set_size(im, size, size);
	lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_center(im);
	lv_obj_set_style_image_recolor(im, lv_color_hex(color), 0);
	lv_obj_set_style_image_recolor_opa(im, 255, 0);
}
static bool sample(const char *name, const char *path, const char *artist, void *u) {
	(void)name;
	item_t *item = u;
	snprintf(item->context.path, sizeof(item->context.path), "%s", path ? path : "");
	snprintf(item->creator, sizeof(item->creator), "%s", artist ? artist : "");
	return false;
}
static bool resolve_sync(item_t *item) {
	if (item->collection == COLLECTION_ARTIST || item->collection == COLLECTION_PODCAST)
		return true;
	library_index_spec_t *s = &item->context.spec;
	/* A shelf needs one cover, not an index/count of every track in each
	 * album. Validate a saved album position without materializing that list. */
	if(s->kind==LIBRARY_LIST_TRACKS && (s->filter==LIBRARY_FILTER_ALBUM || s->filter==LIBRARY_FILTER_GENRE)) {
		char path[512], title[256], value[512];
		bool resumed=item->resume && item->context.path[0] && s->filter==LIBRARY_FILTER_ALBUM &&
			library_track_album_value(item->context.path,value,sizeof(value)) && !strcmp(value,s->value);
		if(resumed) snprintf(path,sizeof(path),"%s",item->context.path);
		else if(!library_sample_path(s->kind,s->filter,s->value,0,path,sizeof(path))) return false;
		snprintf(item->context.path,sizeof(item->context.path),"%s",path);
		library_track_names(path,title,sizeof(title),item->creator,sizeof(item->creator));
		if(item->resume && !resumed) item->context.position=0;
		return true;
	}
	library_index_t *ix = library_index_open(s->kind, s->filter, s->value, s->order, s->desc);
	bool valid = ix && library_index_count(ix) > 0;
	if (valid) {
		char saved[512];
		snprintf(saved, sizeof(saved), "%s", item->context.path);
		library_index_window(ix, 0, 1, sample, item);
		if (item->resume && saved[0] && library_index_find_path(ix, saved) >= 0)
			snprintf(item->context.path, sizeof(item->context.path), "%s", saved);
		else if (item->resume)
			item->context.position = 0;
	}
	if (ix)
		library_index_close(ix);
	return valid;
}
typedef struct {
    collection_item_t saved[24];
    library_home_context_t recent[12], frequent[12], added[12];
    library_release_t releases[10];
    library_stat_row_t artist;
    item_t resolved[100];
    bool valid[100];
    int ns, nr, nf, na, nrel, nresolved, ngenre;
    char genres[4][256];
    unsigned epoch;
} home_data_t;
static home_data_t *feed, *completed;
static pthread_mutex_t feed_lock = PTHREAD_MUTEX_INITIALIZER;
static bool feed_running, feed_requested;
static unsigned feed_epoch;
static lv_timer_t *feed_timer;
static void home_render(void);
static item_t from_context(library_home_context_t context);
static item_t from_saved(collection_item_t r);
static bool same_context(const item_t *a, const item_t *b) {
    return a->context.spec.kind==b->context.spec.kind && a->context.spec.filter==b->context.spec.filter &&
        a->resume==b->resume && !strcmp(a->context.spec.value,b->context.spec.value);
}
static void prepare_item(home_data_t *d, item_t item) {
    for(int n=0;n<d->nresolved;n++) if(same_context(&item,&d->resolved[n])) return;
    if(d->nresolved==100) return;
    int n=d->nresolved++;
    d->valid[n]=resolve_sync(&item);
    d->resolved[n]=item;
}
static bool resolve(item_t *item) {
    if(item->collection==COLLECTION_ARTIST || item->collection==COLLECTION_PODCAST) return true;
    if(!feed) return false;
    for(int n=0;n<feed->nresolved;n++) if(same_context(item,&feed->resolved[n])) {
        snprintf(item->context.path,sizeof(item->context.path),"%s",feed->resolved[n].context.path);
        snprintf(item->creator,sizeof(item->creator),"%s",feed->resolved[n].creator);
        item->context.position=feed->resolved[n].context.position;
        return feed->valid[n];
    }
    return false;
}
static bool feed_genre(const char *name,const char *path,const char *artist,void *u) {
    (void)path;(void)artist;home_data_t *d=u;
    if(d->ngenre==4) return false;
    snprintf(d->genres[d->ngenre++],256,"%s",name);return d->ngenre<4;
}
static void *feed_worker(void *arg) {
    home_data_t *d=arg;
    thread_be_low_priority("home metadata");
    d->ns=collections_list(0,COLLECTION_RECENT,0,d->saved,24);
    d->nr=library_home_contexts(d->recent,12,false);
    d->nf=library_home_contexts(d->frequent,12,true);
    d->na=library_home_added(d->added,12);
    for(int n=0;n<d->ns;n++) prepare_item(d,from_saved(d->saved[n]));
    for(int n=0;n<d->nr;n++) {
        item_t i=from_context(d->recent[n]);prepare_item(d,i);i.resume=true;prepare_item(d,i);
    }
    for(int n=0;n<d->nf;n++) prepare_item(d,from_context(d->frequent[n]));
    for(int n=0;n<d->na;n++) prepare_item(d,from_context(d->added[n]));
    library_for_each(LIBRARY_LIST_GENRES,LIBRARY_FILTER_NONE,NULL,feed_genre,d);
    for(int n=0;n<d->ngenre;n++) {
        item_t i={0};i.context.spec.kind=LIBRARY_LIST_TRACKS;i.context.spec.filter=LIBRARY_FILTER_GENRE;
        snprintf(i.context.spec.value,sizeof(i.context.spec.value),"%s",d->genres[n]);prepare_item(d,i);
    }
    library_filter_t filters[]={LIBRARY_FILTER_HOME_REPEAT,LIBRARY_FILTER_HOME_REDISCOVER,LIBRARY_FILTER_HOME_DAYLIST};
    for(int n=0;n<3;n++) {item_t i={0};i.context.spec.kind=LIBRARY_LIST_TRACKS;i.context.spec.filter=filters[n];prepare_item(d,i);}
    if(library_stats_top_artists(&d->artist,1)>0 && d->artist.name[0]) {
        d->nrel=library_discography(d->artist.name,0,0,10,d->releases);
        for(int n=0;n<d->nrel;n++) {
            item_t i={0};i.context.spec.kind=LIBRARY_LIST_TRACKS;i.context.spec.filter=LIBRARY_FILTER_ALBUM;
            snprintf(i.context.spec.value,sizeof(i.context.spec.value),"%.255s",d->releases[n].value);prepare_item(d,i);
        }
    }
    pthread_mutex_lock(&feed_lock);completed=d;pthread_mutex_unlock(&feed_lock);
    return NULL;
}
static void feed_poll(lv_timer_t *timer) {
    (void)timer;
    pthread_mutex_lock(&feed_lock);home_data_t *ready=completed;completed=NULL;pthread_mutex_unlock(&feed_lock);
    if(ready) {
        feed_running=false;
        if(ready->epoch==feed_epoch) {free(feed);feed=ready;} else free(ready);
    }
    if(!feed_running && feed_requested) {
        home_data_t *d=calloc(1,sizeof(*d));pthread_t worker;
        if(d) {
            d->epoch=feed_epoch;
            pthread_attr_t attr;pthread_attr_init(&attr);pthread_attr_setstacksize(&attr,256*1024);
            int rc=pthread_create(&worker,&attr,feed_worker,d);pthread_attr_destroy(&attr);
            if(!rc) {pthread_detach(worker);feed_running=true;feed_requested=false;}
            else free(d);
        }
    }
    /* Keep a feed stable while scrolling or using an overlay. No database
     * work occurs here, including on the first Home visit. */
    if(feed && feed->epoch==feed_epoch && root && lv_obj_is_visible(root) && scope!=2 &&
       !menu_open && !bottomsheet_active() && !player_sheet_is_open() &&
       lv_obj_get_scroll_y(root)==0 && lv_display_get_inactive_time(NULL)>150) {
        home_render();
        feed->epoch=0;
    }
}
static void open_item(lv_event_t *e) {
	item_t *i = lv_event_get_user_data(e);
	if (player_sheet_drag_active() || menu_open)
		return;
	if (i->audiobook) { book_details(i); return; }
	if (i->collection == COLLECTION_ARTIST) {
		medialist_open_artist(i->context.spec.value);
		return;
	}
	if (i->collection == COLLECTION_PODCAST) {
		return;
	}
	if(i->context.spec.kind==LIBRARY_LIST_PLAYLIST){playlistview_open(i->context.spec.value,false);return;}
	medialist_open(i->context.title, i->context.spec.kind, i->context.spec.filter, i->context.spec.value);
}
static void play(item_t *i) {
    if(i->audiobook) {
        char file[512];double position=0;
        if(!audiobookdb_get_position(i->book_key,file,sizeof(file),&position))
            snprintf(file,sizeof(file),"%s",i->context.path);
        audiobooks_play_at(i->book_key,file,position);
        return;
    }
    if(i->collection)collections_touch(i->collection,i->context.spec.value);
	library_index_spec_t *s = &i->context.spec;
	library_index_t *ix = library_index_open(s->kind, s->filter, s->value, s->order, s->desc);
	if (!ix)
		return;
	int start = i->resume ? library_index_find_path(ix, i->context.path) : 0;
	if (start < 0)
		start = 0;
	// Preview mode: Home stays visible and the mini-player follows; only a tap
	// on the mini-player expands Now Playing.
	if (device_state_play_index_at(ix, start, i->resume ? i->context.position : 0))
		player_refresh_now_playing();
}
static void play_cb(lv_event_t *e) { play(lv_event_get_user_data(e)); }
static bool queue_track(const char *name, const char *path, const char *artist, void *u) {
	(void)name;
	(void)artist;
	snprintf(u, 512, "%s", path ? path : "");
	return false;
}
static void menu_action(lv_event_t *e) {
	int action = (int)(intptr_t)lv_event_get_user_data(e);
	bottomsheet_close(true);
	if (action == 0)
		play(&menu);
	if (action == 1) {
		collection_item_t saved = {0};
		collections_get(menu.collection, menu.context.spec.value, &saved);
		if (!saved.saved)
			collections_put(menu.collection, menu.context.spec.value, menu.context.title, menu.creator, true);
		if (!collections_pin(menu.collection, menu.context.spec.value, !saved.pin))
			gui_notify_popup("You can pin up to four collections.");
	}
	if (action == 2) {
		library_index_spec_t *s = &menu.context.spec;
		int n = 0;
		library_index_t *ix = library_index_open(s->kind, s->filter, s->value, s->order, s->desc);
		if (ix) {
			int total = library_index_count(ix);
			for (int j = 0; j < total && j < 64; j++) {
				char path[512] = "";
				library_index_window(ix, j, 1, queue_track, path);
				if (!path[0] || !playlist_priority_add(path))
					break;
				n++;
			}
			library_index_close(ix);
		}
		device_state_queue_changed();
		char msg[80];
		snprintf(msg, sizeof(msg), "Added %d tracks to queue", n);
		gui_notify_popup(msg);
	}
	if (action == 3 && menu.creator[0])
		medialist_open_artist(menu.creator);
}
static void menu_cleanup(void) { menu_open = false; }
/* One option of the shared sheets: a grey icon and the label, a full-width
 * row that greys while pressed. */
static void menu_row(lv_obj_t *body, const lv_image_dsc_t *glyph, const char *label, int action) {
	int w = config->screen_width - 32;
	lv_obj_t *row = box(body, w, 56);
	lv_obj_set_clickable(row, true);
	lv_obj_set_style_radius(row, 8, 0);
	lv_obj_set_style_bg_color(row, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
	lv_obj_add_event_cb(row, menu_action, LV_EVENT_CLICKED, (void *)(intptr_t)action);
	lv_obj_t *im = lv_image_create(row);
	lv_image_set_src(im, glyph);
	lv_obj_set_size(im, 26, 26);
	lv_image_set_inner_align(im, LV_IMAGE_ALIGN_CONTAIN);
	lv_obj_set_style_image_recolor(im, lv_color_hex(0xb3b3b3), 0);
	lv_obj_set_style_image_recolor_opa(im, LV_OPA_COVER, 0);
	lv_obj_align(im, LV_ALIGN_LEFT_MID, 8, 0);
	lv_obj_t *l = text(row, label, w - 60, &font_ui_20, false);
	lv_obj_align(l, LV_ALIGN_LEFT_MID, 52, 0);
}
static void options(lv_event_t *e) {
	menu = *(item_t *)lv_event_get_user_data(e);
    if(menu.audiobook) { book_details(&menu); return; }
	bool playable = menu.collection != COLLECTION_ARTIST && menu.collection != COLLECTION_PODCAST;
	int rows = (playable ? 2 : 0) + (menu.collection ? 1 : 0) + (menu.creator[0] ? 1 : 0);
	lv_obj_t *body = bottomsheet_open(36 + 56 + rows * 56 + 28, menu_cleanup);
	menu_open = true;
	lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(body, 16, 0);
	int w = config->screen_width - 32;
	lv_obj_t *title = text(body, menu.context.title, w - 60, &font_ui_24_bold, false);
	lv_obj_set_style_pad_left(title, 4, 0);
	lv_obj_set_style_margin_bottom(title, 12, 0);
	if (playable) {
		menu_row(body, &icon_play, menu.resume ? "Resume listening" : "Play collection", 0);
		menu_row(body, &icon_list_queue, "Add to queue", 2);
	}
	if (menu.collection) {
		collection_item_t saved = {0};
		collections_get(menu.collection, menu.context.spec.value, &saved);
		menu_row(body, &sonix_icon_pin, saved.pin ? "Unpin from Home" : "Pin to Home", 1);
	}
	if (menu.creator[0])
		menu_row(body, &icon_artist, "Go to artist", 3);
}
static bool visible(item_t *i) {
	if (!i->art || !lv_obj_is_visible(i->art))
		return false;
	lv_area_t r, a, p;
	lv_obj_get_coords(root, &r);
	lv_obj_get_coords(i->art, &a);
	lv_obj_get_coords(lv_obj_get_parent(i->card), &p);
	return a.y2 >= r.y1 && a.y1 <= r.y2 && a.x2 >= p.x1 && a.x1 <= p.x2 && a.x2 >= r.x1 && a.x1 <= r.x2;
}
static void art_release(int slot) {
	int n = art[slot].item;
	coverloader_release(HOME_SLOT + slot);
	if (n >= 0 && n < item_count && items[n].art) {
		// Back to the grey square and its glyph (see card()).
		lv_image_set_src(items[n].art, NULL);
		lv_obj_t *glyph = lv_obj_get_child(items[n].art, 0);
		if (glyph)
			lv_obj_set_hidden(glyph, false);
	}
	cover_free(&art[slot].cover);
	art[slot].item = -1;
	art[slot].requested = art[slot].settled = false;
}
void homeview_art_drop(void) {
	for (int i = 0; i < HOME_ART; i++)
		art_release(i);
}
void homeview_art_update(void) {
    if(root && scope==2 && book_scan_pending && !audiobookdb_scan_running() && lv_obj_is_visible(root) && !menu_open && !player_sheet_is_open()) {
        book_scan_pending=false;rebuild();
    }
	if (!root || !lv_obj_is_visible(root) || player_sheet_is_open())
		return;
	for (int i = 0; i < HOME_ART; i++)
		if (art[i].item >= 0 && !visible(&items[art[i].item]))
			art_release(i);
	for (int j = 0; j < item_count; j++) {
		if (!items[j].context.path[0] || !visible(&items[j]))
			continue;
		bool found = false;
		int free_slot = -1;
		for (int i = 0; i < HOME_ART; i++) {
			if (art[i].item == j)
				found = true;
			if (art[i].item < 0)
				free_slot = i;
		}
		if (!found && free_slot >= 0) {
			art[free_slot].item = j;
			coverloader_request(HOME_SLOT + free_slot, items[j].context.path, items[j].art_size);
			art[free_slot].requested = true;
		}
	}
	for (int i = 0; i < HOME_ART; i++)
		if (art[i].item >= 0 && !art[i].settled) {
			bool done = false;
			if (coverloader_take(HOME_SLOT + i, &art[i].cover, &done)) {
				lv_image_set_src(items[art[i].item].art, &art[i].cover.dsc);
				lv_obj_t *glyph = lv_obj_get_child(items[art[i].item].art, 0);
				if (glyph)
					lv_obj_set_hidden(glyph, true);
			}
			art[i].settled = done;
		}
}
/* Whether this card is what is playing now: its title turns green, as
 * Spotify marks the playing context on its cards. */
static bool now_playing(const item_t *i) {
	const library_index_spec_t *spec = &i->context.spec;
	if (i->audiobook || !spec->value[0])
		return false;
	if (spec->kind == LIBRARY_LIST_TRACKS && spec->filter == LIBRARY_FILTER_ALBUM)
		return playlist_album()[0] && !strcmp(playlist_album(), spec->value);
	if (spec->kind == LIBRARY_LIST_PLAYLIST)
		return !playlist_album()[0] && !strcmp(playlist_context_name(), spec->value);
	return false;
}
#define TILE_H 56
#define SHELF_CARD_W 150
/* Shelf cards are 150 px; the audiobook grid sets two columns while it builds. */
static int CARD_W = SHELF_CARD_W;
static void card(lv_obj_t *parent, const item_t *source, bool compact) {
	if (item_count == HOME_MAX)
		return;
	item_t *i = &items[item_count++];
	*i = *source;
	bool round = i->collection == COLLECTION_ARTIST;
	bool playing = now_playing(i);
	int w = compact ? (config->screen_width - 48) / 2 : CARD_W;
	int h = compact ? TILE_H : (source->audiobook ? CARD_W + 82 : CARD_W + 78);
	i->card = button(parent, NULL, w, h, open_item, i);
	if (compact) {
		/* Spotify's shortcut: the sleeve flush at the left of a dark tile,
		 * the name beside it in up to two lines. */
		/* No corner clipping: it draws each tile into a layer of its own and
		 * masks it, on every frame -- on the R1 a large share of a Home repaint
		 * for a 4-pixel rounding (2026-11-05). */
		lv_obj_set_style_radius(i->card, 4, 0);
		lv_obj_set_style_bg_color(i->card, lv_color_hex(0x2a2a2a), 0);
		lv_obj_set_style_bg_opa(i->card, 255, 0);
	} else {
		lv_obj_set_style_bg_opa(i->card, LV_OPA_TRANSP, 0);
	}
	lv_obj_set_style_bg_color(i->card, lv_color_hex(compact ? 0x3a3a3a : 0x1f1f1f), LV_STATE_PRESSED);
	lv_obj_set_style_bg_opa(i->card, 255, LV_STATE_PRESSED);
	i->art_size = compact ? TILE_H : CARD_W;
	i->art = lv_image_create(i->card);
	lv_obj_set_size(i->art, i->art_size, i->art_size);
	lv_image_set_inner_align(i->art, LV_IMAGE_ALIGN_CONTAIN);
	/* Until (or unless) a cover arrives: Spotify's grey square with a small
	 * glyph in the middle. The cover is drawn over the square and the glyph is
	 * hidden while it shows. */
	lv_obj_set_style_bg_color(i->art, lv_color_hex(0x333333), 0);
	lv_obj_set_style_bg_opa(i->art, 255, 0);
	if (i->context.spec.kind != LIBRARY_LIST_FAVOURITES) {
		lv_obj_t *glyph = lv_image_create(i->art);
		lv_image_set_src(glyph, i->audiobook ? &icon_book_headphones : round ? &icon_artist : i->collection == COLLECTION_PODCAST ? &icon_podcast_list : &icon_music_note);
		lv_obj_set_size(glyph, i->art_size * 2 / 5, i->art_size * 2 / 5);
		lv_image_set_inner_align(glyph, LV_IMAGE_ALIGN_CONTAIN);
		lv_obj_set_style_image_recolor(glyph, lv_color_hex(0xb3b3b3), 0);
		lv_obj_set_style_image_recolor_opa(glyph, LV_OPA_COVER, 0);
		lv_obj_center(glyph);
	}
	lv_obj_set_pos(i->art, 0, 0);
	lv_obj_set_style_radius(i->art, round ? LV_RADIUS_CIRCLE : compact ? 0 : 4, 0);
	lv_obj_set_style_clip_corner(i->art, round, 0); /* artists stay round, as Spotify draws them */
	if (!i->context.path[0]) {
		if (i->context.spec.kind == LIBRARY_LIST_FAVOURITES) {
			/* Liked Songs' own cover: the heart on the violet gradient. */
			lv_obj_set_style_bg_color(i->art, lv_color_hex(0x450af5), 0);
			lv_obj_set_style_bg_grad_color(i->art, lv_color_hex(0xc4efd9), 0);
			lv_obj_set_style_bg_grad_dir(i->art, LV_GRAD_DIR_VER, 0);
			lv_obj_set_style_image_recolor(i->art, lv_color_white(), 0);
			lv_obj_set_style_image_recolor_opa(i->art, LV_OPA_COVER, 0);
			/* The heart as a child sized to the middle of the sleeve (a
			 * scaled source is not honoured by a sized image widget). */
			lv_obj_t *heart = lv_image_create(i->art);
			lv_image_set_src(heart, &icon_heart);
			lv_obj_set_size(heart, i->art_size * 9 / 20, i->art_size * 9 / 20);
			lv_image_set_inner_align(heart, LV_IMAGE_ALIGN_CONTAIN);
			lv_obj_set_style_image_recolor(heart, lv_color_white(), 0);
			lv_obj_set_style_image_recolor_opa(heart, LV_OPA_COVER, 0);
			lv_obj_center(heart);
		}
	}
	if (compact) {
		lv_obj_t *t = text(i->card, i->context.title, w - TILE_H - 20, &font_ui_18, false);
		lv_label_set_long_mode(t, LV_LABEL_LONG_DOT);
		lv_obj_set_height(t, 2 * font_ui_18.line_height);
		lv_obj_set_style_text_line_space(t, -2, 0);
		lv_obj_align(t, LV_ALIGN_LEFT_MID, TILE_H + 10, 0);
		if (playing)
			lv_obj_set_style_text_color(t, lv_color_hex(0x1ed760), 0);
		/* Spotify reveals two lines only when the name needs them: a short
		 * name sits on the tile's centre line. */
		lv_obj_update_layout(t);
		if (lv_label_get_text(t) && lv_obj_get_self_height(t) <= font_ui_18.line_height + 2)
			lv_obj_set_height(t, font_ui_18.line_height + 4);
		lv_obj_align(t, LV_ALIGN_LEFT_MID, TILE_H + 10, 0);
	} else {
		/* Shelf card: the sleeve, then the name and what it is, in grey. */
		lv_obj_t *t = text(i->card, i->context.title, w, &font_ui_18, false);
		lv_obj_set_pos(t, 0, CARD_W + 8);
		lv_obj_set_height(t, font_ui_18.line_height + 4);
		if (round)
			lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, 0);
		if (playing)
			lv_obj_set_style_text_color(t, lv_color_hex(0x1ed760), 0);
		const char *kind = i->collection == COLLECTION_PLAYLIST ? "Playlist" : round ? "Artist" : "Album";
		char sub[300];
		if (i->audiobook)
			snprintf(sub, sizeof(sub), "%s", i->creator);
		else if (i->context.spec.filter == LIBRARY_FILTER_GENRE || i->context.spec.filter == LIBRARY_FILTER_HOME_REPEAT ||
				 i->context.spec.filter == LIBRARY_FILTER_HOME_REDISCOVER || i->context.spec.filter == LIBRARY_FILTER_HOME_DAYLIST)
			snprintf(sub, sizeof(sub), "Mix from your library");
		else if (round)
			snprintf(sub, sizeof(sub), "Artist");
		else {
			/* Spotify's album and playlist cards (iOS captures 2026-11-04): what it
			 * is in grey, the name in bold, who made it in grey. */
			lv_obj_t *k = text(i->card, kind, w, &font_ui_16, true);
			lv_obj_set_pos(k, 0, CARD_W + 6);
			lv_obj_set_height(k, font_ui_16.line_height + 2);
			lv_obj_set_pos(t, 0, CARD_W + 26);
			lv_obj_set_style_text_font(t, &font_ui_20_bold, 0);
			lv_obj_set_height(t, font_ui_20_bold.line_height + 2);
			snprintf(sub, sizeof(sub), "%s", i->creator);
			t = text(i->card, sub, w, &font_ui_16, true);
			lv_obj_set_pos(t, 0, CARD_W + 52);
			lv_obj_set_height(t, font_ui_16.line_height + 2);
			goto card_text_done;
		}
		t = text(i->card, sub, w, &font_ui_16, true);
		lv_obj_set_pos(t, 0, CARD_W + 34);
		lv_obj_set_height(t, font_ui_16.line_height + 4);
		if (round)
			lv_obj_set_style_text_align(t, LV_TEXT_ALIGN_CENTER, 0);
		if (i->audiobook) {
			t = text(i->card, i->progress, w, &font_ui_16, true);
			lv_obj_set_pos(t, 0, CARD_W + 56);
			lv_obj_set_height(t, font_ui_16.line_height + 4);
		}
	card_text_done:;
	}
	/* No overlay buttons: as on Spotify, a tap opens and a long press offers
	 * Play, the queue and pinning. */
	lv_obj_add_event_cb(i->card, options, LV_EVENT_LONG_PRESSED, i);
}
/* A Spotify shelf: the heading, then a carousel running from the page's
 * gutter to the screen's edge, so the next card peeks in and a sideways drag
 * flicks through them; each card comes to rest at the gutter. */
static lv_obj_t *shelf(const char *title) {
	if (title) { /* NULL: the caller has put its own header up */
		lv_obj_t *heading = text(root, title, config->screen_width - 40, &font_ui_24_bold, false);
		lv_obj_set_style_margin_top(heading, 10, 0);
	}
	int gutter = lv_obj_get_style_pad_left(root, 0);
	lv_obj_t *row = box(root, config->screen_width, scope == 2 ? CARD_W + 90 : CARD_W + 84);
	lv_obj_set_style_translate_x(row, -gutter, 0);
	lv_obj_set_style_pad_hor(row, gutter, 0);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 16, 0);
	lv_obj_set_scrollable(row, true);
	lv_obj_set_scroll_dir(row, LV_DIR_HOR);
	lv_obj_set_scroll_snap_x(row, LV_SCROLL_SNAP_START);
	lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
	lv_obj_add_flag(row, LV_OBJ_FLAG_SCROLL_MOMENTUM);
	return row;
}
/* Spotify's artist shelves ("Discover more from Garoad", "More like AZALI",
 * iOS captures 2026-11-04): a round picture, a grey line over the artist's
 * name, then a carousel. Offline there are no similar artists, so it is the
 * releases of the artist played most these four weeks. */
static void artist_shelf(void) {
	library_stat_row_t top=feed->artist;
	if (!top.name[0])
		return;
	library_release_t *rel=feed->releases;
	int n = feed->nrel;
	if (n < 1)
		return;
	lv_obj_t *head = box(root, config->screen_width - 40, 64);
	lv_obj_set_style_margin_top(head, 14, 0);
	lv_obj_t *disc = lv_obj_create(head);
	lv_obj_remove_style_all(disc);
	lv_obj_set_size(disc, 56, 56);
	lv_obj_set_style_radius(disc, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_color(disc, lv_color_hex(0x7a1f1f), 0);
	lv_obj_set_style_bg_opa(disc, LV_OPA_COVER, 0);
	lv_obj_align(disc, LV_ALIGN_LEFT_MID, 0, 0);
	char initial[8];
	snprintf(initial, sizeof(initial), "%.1s", top.name);
	lv_obj_t *il = text(disc, initial, 40, &font_ui_24_bold, false);
	lv_obj_set_style_text_align(il, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_center(il);
	lv_obj_t *over = text(head, "Discover more from", config->screen_width - 120, &font_ui_16, true);
	lv_obj_set_pos(over, 70, 6);
	lv_obj_t *name = text(head, top.name, config->screen_width - 120, &font_ui_24_bold, false);
	lv_obj_set_pos(name, 70, 28);
	lv_obj_t *row = NULL;
	for (int j = 0; j < n; j++) {
		item_t i = {0};
		i.collection = COLLECTION_ALBUM;
		i.context.spec.valid = true;
		i.context.spec.kind = LIBRARY_LIST_TRACKS;
		i.context.spec.filter = LIBRARY_FILTER_ALBUM;
		snprintf(i.context.spec.value, sizeof(i.context.spec.value), "%.255s", rel[j].value);
		snprintf(i.context.title, sizeof(i.context.title), "%s", rel[j].title);
		snprintf(i.creator, sizeof(i.creator), "%s", rel[j].artist);
		if (!resolve(&i))
			continue;
		if (!row)
			row = shelf(NULL);
		card(row, &i, false);
	}
	if (!row)
		lv_obj_delete(head);
}

static item_t from_context(library_home_context_t context) {
	item_t i = {0};
	i.context = context;
	i.collection = context.spec.kind == LIBRARY_LIST_PLAYLIST ? COLLECTION_PLAYLIST : COLLECTION_ALBUM;
	return i;
}
static item_t from_saved(collection_item_t r) {
	item_t i = {0};
	i.collection = r.kind;
	i.context.spec.valid = true;
	i.context.spec.kind = r.kind == COLLECTION_PLAYLIST ? LIBRARY_LIST_PLAYLIST : LIBRARY_LIST_TRACKS;
	i.context.spec.filter = r.kind == COLLECTION_ALBUM ? LIBRARY_FILTER_ALBUM : LIBRARY_FILTER_NONE;
	snprintf(i.context.spec.value, sizeof(i.context.spec.value), "%.255s", r.key);
	snprintf(i.context.title, sizeof(i.context.title), "%s", r.title);
	snprintf(i.creator, sizeof(i.creator), "%s", r.creator);
	i.context.added = r.added;
	return i;
}
static item_t quick[8];
static int quick_count;
static void quick_add(item_t i) {
	if (quick_count == 8)
		return;
	for (int j = 0; j < quick_count; j++)
		if (quick[j].context.spec.kind == i.context.spec.kind && quick[j].context.spec.filter == i.context.spec.filter && !strcmp(quick[j].context.spec.value, i.context.spec.value))
			return;
	if (i.context.spec.kind==LIBRARY_LIST_FAVOURITES || resolve(&i))
		quick[quick_count++] = i;
}
static item_t mixes[4];
static int mix_count;
static bool genre(const char *name, const char *path, const char *artist, void *u) {
	(void)path;
	(void)artist;
	(void)u;
	item_t i = {0};
	i.context.spec.kind = LIBRARY_LIST_TRACKS;
	i.context.spec.filter = LIBRARY_FILTER_GENRE;
	snprintf(i.context.spec.value, sizeof(i.context.spec.value), "%s", name);
	snprintf(i.context.title, sizeof(i.context.title), "%.240s Mix", name);
	if (mix_count < 4)
		mixes[mix_count++] = i;
	return mix_count < 4;
}
static void rebuild(void) { homeview_build(root, config); }
static void scope_cb(lv_event_t *e) {
    if (switcher_back_drag_active() || player_sheet_drag_active()) return;
	scope = (int)(intptr_t)lv_event_get_user_data(e);
	config_set_int("home", "scope", scope);
	config_save();
	book_page=0;
	rebuild();
    lv_obj_scroll_to_y(root,0,LV_ANIM_OFF);
}
static void customize(lv_event_t *e);
static void customize_change(lv_event_t *e) {
	intptr_t v = (intptr_t)lv_event_get_user_data(e);
	int pos = v / 2;
	char key[32];
	if (v % 2) {
		if (pos > 0) {
			int n = order[pos];
			order[pos] = order[pos - 1];
			order[pos - 1] = n;
		}
	} else {
		snprintf(key, sizeof(key), "hide_%d", order[pos]);
		config_set_bool("home", key, !config_get_bool("home", key, false));
	}
	for (int j = 0; j < 6; j++) {
		snprintf(key, sizeof(key), "order_%d", j);
		config_set_int("home", key, order[j]);
	}
	config_save();
	rebuild();
	customize(NULL);
}
static void customize_toggle(lv_event_t *e) {
	/* Value-changed of the switch: the same "hide" bit as before. */
	intptr_t v = (intptr_t)lv_event_get_user_data(e);
	char key[32];
	snprintf(key, sizeof(key), "hide_%d", order[v]);
	config_set_bool("home", key, !lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
	config_save();
	rebuild();
}
/* Customize Home: each shelf with a switch for whether it shows and an
 * arrow to move it up, in the shared sheet's row style. */
static void customize(lv_event_t *e) {
	(void)e;
	lv_obj_t *b = bottomsheet_open(36 + 56 + 6 * 64 + 24, NULL);
	lv_obj_set_flex_flow(b, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_hor(b, 20, 0);
	int w = config->screen_width - 40;
	lv_obj_t *title = text(b, "Customize Home", w - 60, &font_ui_24_bold, false);
	lv_obj_set_style_margin_bottom(title, 12, 0);
	for (int j = 0; j < 6; j++) {
		char key[32];
		snprintf(key, sizeof(key), "hide_%d", order[j]);
		lv_obj_t *r = box(b, w, 64);
		lv_obj_t *name = text(r, sections[order[j]], w - 150, &font_ui_20, false);
		lv_obj_align(name, LV_ALIGN_LEFT_MID, 0, 0);
		if (j) {
			lv_obj_t *up = button(r, NULL, 48, 48, customize_change, (void *)(intptr_t)(j * 2 + 1));
			lv_obj_set_style_radius(up, LV_RADIUS_CIRCLE, 0);
			lv_obj_set_style_bg_color(up, lv_color_hex(0x2a2a2a), LV_STATE_PRESSED);
			lv_obj_set_style_bg_opa(up, LV_OPA_COVER, LV_STATE_PRESSED);
			icon(up, &icon_chevron_up, 22, 0xb3b3b3);
			lv_obj_align(up, LV_ALIGN_RIGHT_MID, -66, 0);
		}
		lv_obj_t *sw = lv_switch_create(r);
		lv_obj_set_size(sw, 52, 32);
		lv_obj_set_style_bg_color(sw, lv_color_hex(0x535353), LV_PART_MAIN);
		lv_obj_set_style_bg_color(sw, lv_color_hex(0x1ed760), LV_PART_INDICATOR | LV_STATE_CHECKED);
		lv_obj_set_style_bg_color(sw, lv_color_white(), LV_PART_KNOB);
		if (!config_get_bool("home", key, false))
			lv_obj_add_state(sw, LV_STATE_CHECKED);
		lv_obj_add_event_cb(sw, customize_toggle, LV_EVENT_VALUE_CHANGED, (void *)(intptr_t)j);
		lv_obj_align(sw, LV_ALIGN_RIGHT_MID, 0, 0);
	}
}

static bool book_first_part(const char *path,const char *title,void *user) {
    (void)title;snprintf(user,512,"%s",path);return false;
}
static void book_resume_cb(lv_event_t *e) {
    (void)e;bottomsheet_close(true);play(&menu);
}
static void book_details(item_t *i) {
    menu=*i;
    lv_obj_t *body=bottomsheet_open(430,menu_cleanup);menu_open=true;
    lv_obj_set_flex_flow(body,LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(body,20,0);
    lv_obj_set_style_pad_row(body,14,0);
    int w=config->screen_width-40;
    text(body,menu.context.title,w,&font_ui_24_bold,false);
    text(body,menu.creator,w,&font_ui_18,true);
    text(body,menu.progress,w,&font_ui_16,true);
    char *summary=NULL;
    audiobookdb_book_info(menu.book_key,NULL,0,NULL,0,&summary);
    char excerpt[768];snprintf(excerpt,sizeof(excerpt),"%s",summary ? summary : "");
    lv_obj_t *description=text(body,*excerpt ? excerpt : "Listen offline, with your place saved automatically.",w,&font_ui_18,true);
    lv_label_set_long_mode(description,LV_LABEL_LONG_WRAP);lv_obj_set_height(description,110);
    free(summary);
    lv_obj_t *resume=button(body,menu.resume?"Resume listening":"Start listening",w,52,book_resume_cb,NULL);
    lv_obj_set_style_bg_color(resume,lv_color_hex(0x1ed760),0);lv_obj_set_style_bg_opa(resume,255,0);
    lv_obj_set_style_radius(resume,26,0);lv_obj_set_style_text_color(lv_obj_get_child(resume,0),lv_color_black(),0);
}
static bool book_card(const char *name,const char *path,void *user) {
    item_t i={0};i.audiobook=true;
    snprintf(i.context.title,sizeof(i.context.title),"%s",name);
    snprintf(i.book_key,sizeof(i.book_key),"%s",path);
    snprintf(i.context.path,sizeof(i.context.path),"%s",path);
    if(audiobookdb_is_folder_book(path))audiobookdb_parts_for_each(path,book_first_part,i.context.path);
    audiobookdb_book_info(path,NULL,0,i.creator,sizeof(i.creator),NULL);
    if(!i.creator[0])snprintf(i.creator,sizeof(i.creator),"Unknown author");
    char file[512];double seconds=0;
    i.resume=audiobookdb_get_position(path,file,sizeof(file),&seconds) && (seconds>0 || strcmp(file,i.context.path));
    if(audiobookdb_is_finished(path))snprintf(i.progress,sizeof(i.progress),"Finished");
    else if(i.resume)snprintf(i.progress,sizeof(i.progress),"Resume at %d:%02d",(int)seconds/60,(int)seconds%60);
    else snprintf(i.progress,sizeof(i.progress),"Not started");
    card(user,&i,false);return item_count<HOME_MAX;
}
/* Database callbacks run under the index mutex: collect a small window first,
 * then resolve metadata/artwork and create widgets after releasing it. */
static struct { char name[256],path[512]; } book_window[12];
static int book_window_count;
static bool book_collect(const char *name,const char *path,void *u) {
    (void)u;if(book_window_count==12)return false;
    snprintf(book_window[book_window_count].name,256,"%s",name);
    snprintf(book_window[book_window_count++].path,512,"%s",path);return true;
}
static void book_cards(audiobookdb_index_t *ix,int offset,int count,lv_obj_t *parent) {
    book_window_count=0;audiobookdb_index_window(ix,offset,count,book_collect,NULL);
    for(int i=0;i<book_window_count;i++)
        if(book_window[i].path[0])book_card(book_window[i].name,book_window[i].path,parent);
}
static void book_shelf(const char *title,audiobook_list_t kind,audiobook_order_t order) {
    audiobookdb_index_t *ix=audiobookdb_index_open(kind,NULL,order,true);
    if(ix && audiobookdb_index_count(ix)>0) {
        lv_obj_t *row=shelf(title);book_cards(ix,0,6,row);
    }
    if(ix)audiobookdb_index_close(ix);
}
static void book_page_cb(lv_event_t *e) {
    book_page+=(int)(intptr_t)lv_event_get_user_data(e);if(book_page<0)book_page=0;
    rebuild();lv_obj_scroll_to_y(root,0,LV_ANIM_OFF);
}
static void book_scan_cb(lv_event_t *e) {
    (void)e;
    if(!gui_card_available()){gui_notify_no_card();return;}
    char folder[512];snprintf(folder,sizeof(folder),"%s/Audiobooks",config->sd_root_path);
    if(audiobookdb_scan_start(folder)) {book_scan_pending=true;rebuild();}
    else gui_notify_popup("Could not start the audiobook scan.");
}
static void books_build(void) {
    int w=config->screen_width-40;
    bool scanning=audiobookdb_scan_running();book_scan_pending=scanning;
    lv_obj_t *toolbar=box(root,w,44);lv_obj_set_flex_flow(toolbar,LV_FLEX_FLOW_ROW);
    text(toolbar,"Your audiobooks",w-120,&font_ui_24_bold,false);
    lv_obj_t *scan=button(toolbar,scanning?"Scanning...":"Scan books",116,44,book_scan_cb,NULL);
    lv_obj_set_style_bg_color(scan,lv_color_hex(0x282828),0);lv_obj_set_style_bg_opa(scan,255,0);lv_obj_set_style_radius(scan,22,0);
    if(scanning)lv_obj_add_state(scan,LV_STATE_DISABLED);
    if(!gui_card_available() || !audiobookdb_count()) {
        lv_obj_t *empty=box(root,w,280);
        lv_obj_t *im=lv_image_create(empty);lv_image_set_src(im,&icon_book_headphones);
        lv_obj_set_size(im,96,96);lv_image_set_inner_align(im,LV_IMAGE_ALIGN_CONTAIN);lv_obj_align(im,LV_ALIGN_TOP_MID,0,30);
        lv_obj_t *hint=text(empty,scanning?"Looking for your books...":"Your next chapter starts here",w,&font_ui_24_bold,false);
        lv_obj_set_style_text_align(hint,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_y(hint,154);
        hint=text(empty,"Add books to the Audiobooks folder on your card, then tap Scan books.",w-24,&font_ui_18,true);
        lv_label_set_long_mode(hint,LV_LABEL_LONG_WRAP);lv_obj_set_style_text_align(hint,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_pos(hint,12,202);
        return;
    }
    if(book_page==0) {
        book_shelf("Continue listening",AUDIOBOOK_LIST_CONTINUE,AUDIOBOOK_ORDER_PLAYED);
        book_shelf("Recently added",AUDIOBOOK_LIST_ALL,AUDIOBOOK_ORDER_ADDED);
    }
    audiobookdb_index_t *ix=audiobookdb_index_open(AUDIOBOOK_LIST_ALL,NULL,AUDIOBOOK_ORDER_NAME,false);
    int total=ix?audiobookdb_index_count(ix):0;
    if(book_page*12>=total)book_page=0;
    text(root,"All audiobooks",w,&font_ui_24_bold,false);
    lv_obj_t *grid=box(root,w,LV_SIZE_CONTENT);lv_obj_set_flex_flow(grid,LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_column(grid,16,0);lv_obj_set_style_pad_row(grid,16,0);
    CARD_W=(w-16)/2; /* two columns of large covers */
    if(ix){book_cards(ix,book_page*12,12,grid);audiobookdb_index_close(ix);}
    CARD_W=SHELF_CARD_W;
    if(total>12) {
        lv_obj_t *nav=box(root,w,48);lv_obj_set_flex_flow(nav,LV_FLEX_FLOW_ROW);
        if(book_page)button(nav,"Previous",110,44,book_page_cb,(void *)(intptr_t)-1);
        char page[48];snprintf(page,sizeof(page),"Page %d of %d",book_page+1,(total+11)/12);text(nav,page,170,&font_ui_16,true);
        if((book_page+1)*12<total)button(nav,"Next",100,44,book_page_cb,(void *)(intptr_t)1);
    }
}

static void home_render(void) {
    lv_obj_t *parent=root;gui_config_t *cfg=config;
	uint32_t build_started = lv_tick_get();
	homeview_art_drop();
	root = parent;
	config = cfg;
	item_count = 0;
	lv_obj_clean(root);
	scope = config_get_int("home", "scope", 0);
	if (scope < 0 || scope > 2)
		scope = 0;
	collections_init(cfg->sd_root_path);

	int w = cfg->screen_width - 40;
	lv_obj_t *filters = box(root, w, 44);
	lv_obj_set_flex_flow(filters, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(filters, 8, 0);
	const char *names[] = {"All", "Music", "Audiobooks"};
	int widths[] = {54, 78, 132};
	for (int j = 0; j < 3; j++) {
		lv_obj_t *b = button(filters, names[j], widths[j], 42, scope_cb, (void *)(intptr_t)j);
		lv_obj_set_style_radius(b, 22, 0);
		lv_obj_set_style_bg_opa(b, 255, 0);
		lv_obj_set_style_bg_color(b, lv_color_hex(scope == j ? 0x1ed760 : 0x282828), 0);
		lv_obj_set_style_text_color(lv_obj_get_child(b, 0), lv_color_hex(scope == j ? 0x111111 : 0xffffff), 0);
	}
	if(scope!=2) {
        lv_obj_t *edit = button(filters, NULL, 40, 42, customize, NULL);
        icon(edit, &icon_music_settings, 24, 0xb3b3b3);
    }
    if(scope==2) { books_build();lv_obj_update_layout(root);return; }
    if(!feed) {text(root,"Loading your music...",w,&font_ui_18,true);return;}
    collection_item_t *saved=feed->saved;int ns=feed->ns;
    library_home_context_t *recent=feed->recent,*frequent=feed->frequent,*added=feed->added;
    int nr=feed->nr,nf=feed->nf,na=feed->na;
	bool used[6] = {0};
	for (int j = 0; j < 6; j++) {
		char key[32];
		snprintf(key, sizeof(key), "order_%d", j);
		int n = config_get_int("home", key, j);
		if (n < 0 || n > 5 || used[n]) {
			for (n = 0; n < 6 && used[n]; n++)
				;
		}
		order[j] = n;
		used[n] = true;
	}
	/* Where the time goes, section by section, in the log: on the R1 a whole
	 * build took 900 ms (2026-11-05). */
	char timing[160] = "";
	uint32_t mark = lv_tick_get();
	int timing_len = snprintf(timing, sizeof(timing), "start %u", (unsigned)lv_tick_elaps(build_started));
	if (scope != 2)
		for (int z = 0; z < 6; z++) {
			if (z > 0 && timing_len > 0 && (size_t)timing_len < sizeof(timing)) {
				timing_len += snprintf(timing + timing_len, sizeof(timing) - (size_t)timing_len, " s%d %u", order[z - 1],
									   (unsigned)lv_tick_elaps(mark));
				mark = lv_tick_get();
			}
			int section = order[z];
			char key[32];
			snprintf(key, sizeof(key), "hide_%d", section);
			if (config_get_bool("home", key, false))
				continue;
			if (section == 0) {
				quick_count = 0;
				for (int j = 0; j < ns; j++)
					if (saved[j].kind!=COLLECTION_FOLDER && saved[j].pin && saved[j].kind != COLLECTION_PODCAST)
						quick_add(from_saved(saved[j]));
				for (int j = 0; j < nr && j < 3; j++)
					quick_add(from_context(recent[j]));
				for (int j = 0; j < nf && j < 3; j++)
					quick_add(from_context(frequent[j]));
				item_t liked = {0};
				liked.context.spec.kind = LIBRARY_LIST_FAVOURITES;
				snprintf(liked.context.title, sizeof(liked.context.title), "Liked Songs");
				quick_add(liked);
				for (int j = 0; j < na; j++)
					quick_add(from_context(added[j]));
				for (int j = 0; j < ns; j++)
					if (saved[j].kind == COLLECTION_PLAYLIST)
						quick_add(from_saved(saved[j]));
				if (quick_count) {
					lv_obj_t *grid = box(root, w, ((quick_count + 1) / 2) * (TILE_H + 8));
					lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
					lv_obj_set_style_pad_column(grid, 8, 0);
					lv_obj_set_style_pad_row(grid, 8, 0);
					for (int j = 0; j < quick_count; j++)
						card(grid, &quick[j], true);
				}
			} else if (section == 1 && nr) {
				lv_obj_t *row = NULL;
				int shown = 0;
				for (int j = 0; j < nr && shown < 10; j++) {
					item_t i = from_context(recent[j]);
					i.resume = true;
					if (!resolve(&i))
						continue;
					if (!row)
						row = shelf(sections[1]);
					card(row, &i, false);
					shown++;
				}
			} else if (section == 2) {
				item_t candidates[24];
				int n = 0;
				for (int j = 0; j < na; j++)
					candidates[n++] = from_context(added[j]);
				for (int j = 0; j < ns && n < 24; j++)
					if (saved[j].kind == COLLECTION_PLAYLIST)
						candidates[n++] = from_saved(saved[j]);
				for (int j = 1; j < n; j++) {
					item_t v = candidates[j];
					int k = j;
					while (k > 0 && candidates[k - 1].context.added < v.context.added) {
						candidates[k] = candidates[k - 1];
						k--;
					}
					candidates[k] = v;
				}
				lv_obj_t *row = NULL;
				int shown = 0;
				for (int j = 0; j < n && shown < 10; j++)
					if (resolve(&candidates[j])) {
						if (!row)
							row = shelf(sections[2]);
						card(row, &candidates[j], false);
						shown++;
					}
			} else if (section == 3 || section == 5) {
				item_t i = {0};
				i.context.spec.kind = LIBRARY_LIST_TRACKS;
				i.context.spec.filter = section == 3 ? LIBRARY_FILTER_HOME_REPEAT : LIBRARY_FILTER_HOME_REDISCOVER;
				snprintf(i.context.title, sizeof(i.context.title), "%s", section == 3 ? "Your repeat mix" : "Rediscover your music");
				if (resolve(&i))
					card(shelf(sections[section]), &i, false);
			} else if (section == 4) {
                mix_count=0;
                for(int g=0;g<feed->ngenre;g++) genre(feed->genres[g],NULL,NULL,NULL);
				lv_obj_t *row = NULL;
				/* Spotify's Daylist, offline: what you play around this time
				 * of day, named for the moment ("Wednesday evening"). */
				{
					static const char *const DAYS[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
					time_t now_t = time(NULL);
					struct tm now_tm;
					localtime_r(&now_t, &now_tm);
					const char *part = now_tm.tm_hour < 5 ? "night" : now_tm.tm_hour < 12 ? "morning" : now_tm.tm_hour < 17 ? "afternoon" : now_tm.tm_hour < 22 ? "evening" : "night";
					item_t d = {0};
					d.context.spec.valid = true;
					d.context.spec.kind = LIBRARY_LIST_TRACKS;
					d.context.spec.filter = LIBRARY_FILTER_HOME_DAYLIST;
					snprintf(d.context.title, sizeof(d.context.title), "%s %s", DAYS[now_tm.tm_wday], part);
					if (resolve(&d)) {
						row = shelf(sections[4]);
						card(row, &d, false);
					}
				}
				for (int j = 0; j < mix_count; j++)
					if (resolve(&mixes[j])) {
						if (!row)
							row = shelf(sections[4]);
						card(row, &mixes[j], false);
					}
			}
		}

	if (timing_len > 0 && (size_t)timing_len < sizeof(timing)) {
		timing_len += snprintf(timing + timing_len, sizeof(timing) - (size_t)timing_len, " s%d %u", order[5],
							   (unsigned)lv_tick_elaps(mark));
	}
	mark = lv_tick_get();
	if (scope != 2)
		artist_shelf();
	if (!item_count && scope != 2)
		text(root, "Add music to your card to start your Home feed.", w, &font_ui_20, true);
	uint32_t artists_ms = lv_tick_elaps(mark);
	mark = lv_tick_get();
	lv_obj_update_layout(root);
	printf("home: sections (ms) %s artists %u layout %u\n", timing, (unsigned)artists_ms, (unsigned)lv_tick_elaps(mark));
}

void homeview_build(lv_obj_t *parent,gui_config_t *cfg) {
    int next_scope=config_get_int("home","scope",0);
    bool retain=parent==root && item_count>0 && scope==next_scope && next_scope!=2;
    root=parent;config=cfg;scope=next_scope;
    collections_init(cfg->sd_root_path);
    ++feed_epoch;feed_requested=true;
    if(!feed_timer) feed_timer=lv_timer_create(feed_poll,50,NULL);
    /* First visit and explicit customization draw a lightweight shell or
     * the previous snapshot immediately. Refresh never blocks the tab tap. */
    if(!retain) home_render();
}
