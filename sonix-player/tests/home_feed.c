#include "src/system/library/library.h"
#include "src/system/library/playlists.h"
#include "src/system/core/config.h"
#include "src/system/db/sqlite3.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
static int count(library_filter_t filter) {
    library_index_t *ix=library_index_open(LIBRARY_LIST_TRACKS,filter,"",LIBRARY_ORDER_DEFAULT,false);assert(ix);
    int n=library_index_count(ix);library_index_close(ix);return n;
}
int main(int argc,char **argv) {
    assert(argc==2);char dbpath[768],cfg[768],path[512],album[256];
    snprintf(dbpath,sizeof(dbpath),"%s/.local/library.db",argv[1]);snprintf(cfg,sizeof(cfg),"%s/config.ini",argv[1]);config_init(cfg);
    assert(library_open(dbpath));playlists_init(argv[1]);
    for(int i=0;i<5;i++) {snprintf(path,sizeof(path),"%s/Music/track%d.wav",argv[1],i);assert(library_import_download(path));for(int j=0;library_scan_running()&&j<2000;j++)usleep(10000);assert(!library_scan_running());}
    assert(count(LIBRARY_FILTER_HOME_REPEAT)==0);assert(count(LIBRARY_FILTER_HOME_REDISCOVER)==5);
    library_index_t *mix=library_index_open(LIBRARY_LIST_TRACKS,LIBRARY_FILTER_GENRE,"Ambient",LIBRARY_ORDER_DEFAULT,false);
    assert(mix && library_index_count(mix)==5);library_index_close(mix);
    snprintf(path,sizeof(path),"%s/Music/track0.wav",argv[1]);library_note_play(path);assert(count(LIBRARY_FILTER_HOME_REPEAT)==0);
    library_note_play(path);assert(count(LIBRARY_FILTER_HOME_REPEAT)==1);assert(count(LIBRARY_FILTER_HOME_REDISCOVER)==4);
    sqlite3 *db;assert(sqlite3_open(dbpath,&db)==SQLITE_OK);
    assert(sqlite3_exec(db,"UPDATE HOME_PLAYS SET day=CAST(strftime('%s','now') AS INT)/86400-30",NULL,NULL,NULL)==SQLITE_OK);
    assert(count(LIBRARY_FILTER_HOME_REPEAT)==0);sqlite3_close(db);
    library_note_play(path);library_note_play(path);assert(count(LIBRARY_FILTER_HOME_REPEAT)==1);
    assert(library_track_album_value(path,album,sizeof(album)));
    library_home_touch(LIBRARY_LIST_TRACKS,LIBRARY_FILTER_ALBUM,album,"Night Lights");
    library_index_spec_t spec={.valid=true,.kind=LIBRARY_LIST_TRACKS,.filter=LIBRARY_FILTER_ALBUM};snprintf(spec.value,sizeof(spec.value),"%s",album);
    library_home_resume_save(&spec,path,12.5);
    library_home_context_t rows[12];assert(library_home_contexts(rows,12,false)==1);assert(rows[0].position==12.5 && !strcmp(rows[0].path,path));
    assert(playlists_create("Home Mix"));assert(playlists_add_track("Home Mix",path));
    library_home_touch(LIBRARY_LIST_PLAYLIST,LIBRARY_FILTER_NONE,"Home Mix","Home Mix");
    library_home_touch(LIBRARY_LIST_PLAYLIST,LIBRARY_FILTER_NONE,"Home Mix","Home Mix");
    assert(library_home_contexts(rows,12,true)==2 && !strcmp(rows[0].title,"Home Mix") && rows[0].visits==2);
    assert(library_home_added(rows,12)==1 && rows[0].added>time(NULL)-60);
    assert(!strcmp(rows[0].title,"Night Lights"));
    assert(sqlite3_open(dbpath,&db)==SQLITE_OK);
    assert(sqlite3_exec(db,"UPDATE MEDIA_TABLE SET ctime=789000000",NULL,NULL,NULL)==SQLITE_OK);
    sqlite3_close(db);
    assert(library_home_added(rows,12)==1 && rows[0].added>time(NULL)-60);
    library_close();assert(library_open(dbpath));assert(library_home_contexts(rows,12,false)==1 && rows[0].position==12.5);library_close();
    puts("PASS Home: cold-start discovery, rolling 30-day qualified counts, stale-play exclusion, collection access ranking, album resume position, import date and restart persistence");
}
