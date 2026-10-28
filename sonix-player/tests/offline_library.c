#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/library/collections.h"
#include "src/system/core/config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static int hits[5];
static char excerpt[192];
static void collect(const library_instant_hit_t *hit,void *user) {
    (void)user;assert(hit->type>=0 && hit->type<5);hits[hit->type]++;
    if(hit->type==LIBRARY_SEARCH_LYRICS)snprintf(excerpt,sizeof(excerpt),"%s",hit->excerpt);
}
static void search(const char *query) { memset(hits,0,sizeof(hits));excerpt[0]=0;library_search_instant(query,6,collect,NULL); }
int main(int argc,char **argv) {
    assert(argc==2);char path[768],db[768],settings[768];
    snprintf(db,sizeof(db),"%s/.local/library.db",argv[1]);
    snprintf(settings,sizeof(settings),"%s/config.ini",argv[1]);
    config_init(settings);assert(library_open(db));playlists_init(argv[1]);collections_init(argv[1]);
    for(int i=0;i<5;i++) {
        snprintf(path,sizeof(path),"%s/Music/track%d.wav",argv[1],i);
        assert(library_import_download(path));
        for(int wait=0;library_scan_running() && wait<2000;wait++)usleep(10000);
        assert(!library_scan_running());
    }
    assert(library_track_count()==5);
    search("night");assert(hits[LIBRARY_SEARCH_TRACK] && hits[LIBRARY_SEARCH_ARTIST] && hits[LIBRARY_SEARCH_ALBUM] && hits[LIBRARY_SEARCH_LYRICS]);
    assert(strstr(excerpt,"night"));
    search("cafe");assert(hits[LIBRARY_SEARCH_TRACK]);
    search("CAFÉ");assert(hits[LIBRARY_SEARCH_TRACK]);
    search("nig");assert(hits[LIBRARY_SEARCH_TRACK]);
    char lyric[768];snprintf(lyric,sizeof(lyric),"%s/Music/track0.lrc",argv[1]);
    FILE *file=fopen(lyric,"w");assert(file);fputs("[00:00.00]A silvermoon lyric revision\n[00:04.00]We walk into the night\n",file);fclose(file);
    library_search_refresh_lyrics();search("silvermoon");assert(hits[LIBRARY_SEARCH_LYRICS]==1);
    assert(playlists_exists("Night Drive") || playlists_create("Night Drive"));
    snprintf(path,sizeof(path),"%s/Music/track0.wav",argv[1]);
    assert(playlists_add_track("Night Drive",path));assert(playlists_has_track("Night Drive",path));
    search("night");assert(hits[LIBRARY_SEARCH_PLAYLIST]);
    assert(playlists_remove_track("Night Drive",path));assert(!playlists_has_track("Night Drive",path));
    assert(playlists_add_track("Night Drive",path));
    assert(playlists_add_track("Night Drive",path));
    assert(library_playlist_remove_all_path("Night Drive",path));
    assert(!playlists_has_track("Night Drive",path));
    assert(playlists_add_track("Night Drive",path));
    assert(!library_fav_contains(path));
    char album[256];assert(library_track_album_value(path,album,sizeof(album)));
    assert(collections_put(COLLECTION_ALBUM,album,"Night Lights","Night Ensemble",true));
    assert(!library_fav_contains(path)); /* Saving album never likes tracks. */
    assert(collections_put(COLLECTION_ARTIST,"Night Ensemble","Night Ensemble","",true));
    assert(collections_put(COLLECTION_PLAYLIST,"Night Drive","Night Drive","You",true));
    assert(collections_put(COLLECTION_ALBUM,"Another","Another","Other",true));
    assert(collections_pin(COLLECTION_ALBUM,album,true));
    assert(collections_pin(COLLECTION_ARTIST,"Night Ensemble",true));
    assert(collections_pin(COLLECTION_PLAYLIST,"Night Drive",true));
    assert(collections_pin(COLLECTION_ALBUM,"Another",true));
    assert(collections_put(COLLECTION_ARTIST,"Fifth","Fifth","",true));
    assert(!collections_pin(COLLECTION_ARTIST,"Fifth",true));
    assert(collections_put(COLLECTION_ALBUM,"Another","Another","Other",false));
    assert(collections_put(COLLECTION_ARTIST,"Fifth","Fifth","",false));
    assert(access(path,R_OK)==0);
    collection_item_t row;assert(collections_get(COLLECTION_ALBUM,album,&row)&&row.saved&&row.pin);
    assert(playlists_rename("Night Drive","Night Ride"));
    assert(collections_get(COLLECTION_PLAYLIST,"Night Ride",&row) && row.pin);
    assert(!collections_get(COLLECTION_PLAYLIST,"Night Drive",&row));
    assert(playlists_rename("Night Ride","Night Drive"));
    assert(playlists_create("Temporary"));
    assert(collections_put(COLLECTION_PLAYLIST,"Temporary","Temporary","You",true));
    assert(playlists_delete("Temporary"));
    assert(!collections_get(COLLECTION_PLAYLIST,"Temporary",&row));
    library_close();assert(library_open(db));search("night");assert(hits[0]&&hits[4]);
    assert(collections_get(COLLECTION_ALBUM,album,&row)&&row.saved);
    assert(playlists_has_track("Night Drive",path));
    library_close();
    puts("PASS offline library: real audio scan, persistent lyric/metadata prefix index, accent folding, changed LRC reindex, playlist membership, independent saved albums/artists, four-pin limit, no audio deletion, reopen persistence");
    return 0;
}
